const http = require('http');
const fs = require('fs').promises;
var protobuf = require("protobufjs");


const hostname = '127.0.0.1';
const port = 3000;

var weather_data_objects = [];

protobuf.load("../Proto/weather.proto", function(err, root) {
  if (err)
      throw err;
  else 
    console.log('Protobuf file loaded, setting up the server.')

  // Obtain the message types
  var DataMessage = root.lookupType("weather.Data");
  var SettingsMessage = root.lookupType("weather.Settings");
  var weather_settings;

  // Set the default settings
  const default_settings = {updatePeriodSec: 2}
  const errMsg = SettingsMessage.verify(default_settings);
  if(errMsg) {
      throw Error(errMsg);
  }
  else {
    weather_settings = SettingsMessage.create(default_settings);
    weather_settings.update_period_sec = 10;
    console.log(`weather_settings = ${JSON.stringify(weather_settings)}`)
  }

  // Create a server object which listens to various end points.
  const server = http.createServer((req, res) => {
    if(req.url.startsWith("/api")) {
      switch(req.url) {
        case "/api/settings":
          if('POST' == req.method) {
            console.log("/api/settings POST")
            // TODO post
          }
          else {
            console.log("/api/settings GET")
            // Get request, return the current settings
            res.setHeader("Content-Type", "application/x-protobuf");
            res.writeHead(200);
            let settings_buffer = SettingsMessage.encode(weather_settings).finish();
            console.log(`settings_buffer = ${Array.prototype.toString.call(settings_buffer)}`);
            res.end(settings_buffer);
          }
          break
        case "/api/data":
          if('POST' == req.method) {
            console.log("/api/data POST");
            // An empty buffer to store the data received.
            var receive_buffer = Buffer.alloc(0);

            req.on('data', function(chunk) {
              let temp = receive_buffer;
              receive_buffer = new Buffer.concat([temp, chunk]);
            })
            req.on('end', function() {
              console.log('Body: ')
              let data_str = ''
              for(let pair of receive_buffer.entries()) {
                data_str += pair[1].toString() + ", ";
              }
              console.log(data_str);

              let message = DataMessage.decode(receive_buffer);
              console.log('Message: ');
              console.log(message);

              // This is normally where you would add the data in database.
              // In this demo we use a simple array.
              weather_data_objects.push(message);

              res.writeHead(200);
              res.end();
            })
          }
          else {
            console.log("/api/data GET");
            // TODO   
          }
          break;
        default:
          res.writeHead(404);
          res.end();
          break;
      }
    }
    else {
      // Serve the home page, the home page is stored in a html file.
      fs.readFile(__dirname + "/home.html")
          .then(contents => {
              res.setHeader("Content-Type", "text/html");
              res.writeHead(200);
              res.end(contents);
          })
          .catch(err => {
              res.writeHead(500);
              res.end(err);
          });
    }
  });

  server.listen(port, hostname, () => {
    console.log(`Server running at http://${hostname}:${port}/`);
  });
  
}); // end of protobuf.load()
