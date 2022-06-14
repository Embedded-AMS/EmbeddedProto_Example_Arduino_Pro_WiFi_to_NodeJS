const http = require('http');
const fs = require('fs').promises;
var protobuf = require("protobufjs");


const hostname = '127.0.0.1';
const port = 3000;



protobuf.load("../Proto/weather.proto", function(err, root) {
  if (err)
      throw err;
  else 
    console.log('Protobuf file loaded, setting up the server.')

  // Obtain the message types
  var DataMessage = root.lookupType("weather.Data");
  var SettingsMessage = root.lookupType("weather.Settings");

  // Set the default settings
  const default_settings = {updatePeriodSec: 5}
  const errMsg = SettingsMessage.verify(default_settings);
  if(errMsg) {
      throw Error(errMsg);
  }
  else {
    var weather_settings = SettingsMessage.create(default_settings);
    weather_settings.update_period_sec = 5;
    console.log(`weather_settings = ${JSON.stringify(weather_settings)}`)
  }

  // Create a server object which listens to various end points.
  const server = http.createServer((req, res) => {
    if(req.url.startsWith("/api")) {
      switch(req.url) {
        case "/api/settings":
          if('POST' == req.method) {
            // TODO post
          }
          else {
            console.log("/api/settings GET")
            // Get request, return the current settings
            res.setHeader("Content-Type", "application/x-protobuf");
            res.writeHead(200);
            let buffer = SettingsMessage.encode(weather_settings).finish()
            console.log(`buffer = ${Array.prototype.toString.call(buffer)}`)
            res.end(buffer)
          }
          break
        case "/api/data":
          // TODO both get and post
          break
        default:
          res.writeHead(404);
          res.end();
          break
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
              return;
          });
    }
  });

  server.listen(port, hostname, () => {
    console.log(`Server running at http://${hostname}:${port}/`);
  });
  
}); // end of protobuf.load()
