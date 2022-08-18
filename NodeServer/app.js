/*
 *  Copyright (C) 2020-2022 Embedded AMS B.V. - All Rights Reserved
 *
 *  This file is part of Embedded Proto.
 *
 *  Embedded Proto is open source software: you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License as published 
 *  by the Free Software Foundation, version 3 of the license.
 *
 *  Embedded Proto  is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Embedded Proto. If not, see <https://www.gnu.org/licenses/>.
 *
 *  For commercial and closed source application please visit:
 *  <https://EmbeddedProto.com/license/>.
 *
 *  Embedded AMS B.V.
 *  Info:
 *    info at EmbeddedProto dot com
 *
 *  Postal address:
 *    Johan Huizingalaan 763a
 *    1066 VH, Amsterdam
 *    the Netherlands
 */


const http = require('http');
const fs = require('fs').promises;
const protobuf = require("protobufjs");

let DataMessage;
let DataHistoryMessage;
let SettingsMessage;

const hostname = '127.0.0.1';
let weather_data;
let data_history
let weather_settings;


const port = 3000;

protobuf.load("../Proto/weather.proto", function(err, root) {
  if (err)
      throw err;
  else 
    console.log('Protobuf file loaded, setting up the server.')

  // Obtain the message types
  DataMessage = root.lookupType("weather.Data");
  DataHistoryMessage = root.lookupType("weather.DataHistory");
  SettingsMessage = root.lookupType("weather.Settings");
  data_history = DataHistoryMessage.create();

  // Set the default settings, a 5 second update period.
  const default_settings = {updatePeriodSec: 5}
  let errMsg = SettingsMessage.verify(default_settings);
  if(errMsg) {
    throw Error(errMsg);
  }
  else {
    weather_settings = SettingsMessage.create(default_settings);
    console.log(`weather_settings = ${JSON.stringify(weather_settings)}`)
  }

  // Initialize the weather data object at zero.
  const dummy_data = {temperature: 0.0, humidity: 0.0, airPressure: 0, windSpeed: 0.0, windDirection: 0.0}
  errMsg = DataMessage.verify(dummy_data);
  if(errMsg) {
      throw Error(errMsg);
  }
  else {
    weather_data = DataMessage.create(dummy_data);
    console.log(`weather_data = ${JSON.stringify(weather_data)}`)
  }

  // Create a server object which listens to various end points.
  const server = http.createServer((req, res) => {
    if(req.url.startsWith("/api")) {
      // Process requests directed to the API.
      server_api(req, res)
    }
    else {
      // In any other case serve the homepage.
      server_home(req, res)
    }
  });

  // Start the server. When it started report the IP and port it is running on.
  server.listen(port, hostname, () => {
    console.log(`Server running at http://${hostname}:${port}/`);
  });
  
}); // end of protobuf.load()


// Process requests directed to the API.
function server_api(req, res) {
  switch(req.url) {
    case "/api/settings":
      // Process POST and GET requests to the weather settings API.
      server_api_settings(req, res)
      break 
    case "/api/data":
      // Process POST and GET requests to the weather data API.
      server_api_data(req, res)
      break;
    default:
      // Return an error in any other case.
      res.writeHead(404);
      res.end();
      break;
  }
}

// Process POST and GET requests to the weather settings API.
function server_api_settings(req, res) {
  if('GET' == req.method) {
    console.log("/api/settings GET")
    // Get request, return the current settings
    res.setHeader("Content-Type", "application/x-protobuf");
    res.writeHead(200);
    let settings_buffer = SettingsMessage.encode(weather_settings).finish();
    console.log(`settings_buffer = ${Array.prototype.toString.call(settings_buffer)}`);
    res.end(settings_buffer);
  }
  else {
    // Return an error in any other case.
    res.writeHead(405);
    res.end();
  }
}

// Process POST and GET requests to the weather data API.
function server_api_data(req, res) {
  if('POST' == req.method) {
    console.log("/api/data POST");
    // An empty buffer to store the data received.
    let receive_buffer = Buffer.alloc(0);

    req.on('data', function(chunk) {
      const temp = receive_buffer.slice();
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
      //weather_data_objects.push(message);
      data_history.data.push(message);
      
      res.writeHead(200);
      res.end();
    })
  }
  else if('GET' == req.method) {
    console.log("/api/data GET");
    res.setHeader("Content-Type", "application/x-protobuf");
    res.writeHead(200);

    let data_buffer = DataHistoryMessage.encode(data_history).finish();
    console.log(`data_buffer = ${Array.prototype.toString.call(data_buffer)}`);
    res.end(data_buffer);

    // Clear the data.
    data_history.data = [];
  }
  else {
    // Return an error in any other case.
    res.writeHead(405);
    res.end();
  }
}

// Serve files related to the homepage.
function server_home(req, res) {
  if('GET' == req.method) {
    // Depending on the request serve different files related to the homepage.
    let filename;
    switch(req.url) {
      case "/":
        filename = __dirname + "/static/home.html";
        res.setHeader("Content-Type", "text/html");
        break;
      case "/favicon.ico":
        filename = __dirname + "/static/favicon.svg";
        res.setHeader("Content-Type", "image/svg+xml");
        break;
      case "/weather_bundle.js":
        filename = __dirname + "/static/weather_bundle.js";
        res.setHeader("Content-Type", "text/javascript");
        break;
      default:
        res.writeHead(404);
        res.end();
        return
    }

    // Serve the requested file.
    fs.readFile(filename)
        .then(contents => {
            res.writeHead(200);
            res.end(contents);
        })
        .catch(fs_err => {
            res.writeHead(500);
            res.end(fs_err);
        });
  }
  else {
    // Return an error in any other case.
    res.writeHead(405);
    res.end();
  }
}
