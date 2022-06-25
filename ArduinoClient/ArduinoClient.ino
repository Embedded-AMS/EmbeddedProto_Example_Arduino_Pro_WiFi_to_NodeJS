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

#include <WiFi.h>
#include <time.h>

#include "weather.h"
#include "src/ReadBufferFixedSize.h"
#include "src/WriteBufferFixedSize.h"
#include "src/Errors.h"

// Please enter your configurations in the settings.h file:
//  * WiFi name (SSID)
//  * WiFi password
//  * Server IP
//  * Server Port number
#include "settings.h" 

// Setup global variables.
WiFiClient client;
weather::Data weather_data;
weather::Settings weather_settings;

constexpr int BUFFER_SIZE = 256;
EmbeddedProto::WriteBufferFixedSize<BUFFER_SIZE> write_buffer;
EmbeddedProto::ReadBufferFixedSize<BUFFER_SIZE> read_buffer;
String string;

unsigned long update_time = 0;

// Use the Portenta RGB led to signal an error.
void signal_error()
{
  digitalWrite(LEDR, LOW);
  delay(100);
  digitalWrite(LEDR, HIGH);
  delay(100);
  digitalWrite(LEDR, LOW);
  delay(100);
  digitalWrite(LEDR, HIGH);
  delay(700);
}

// Use the RGB led to signal we are not connected to wifi.
void signal_wifi_disconnect()
{
  digitalWrite(LEDB, LOW);
  delay(100);
  digitalWrite(LEDB, HIGH);
  delay(100);
  digitalWrite(LEDB, LOW);
  delay(100);
  digitalWrite(LEDB, HIGH);
  delay(700);
}

// Use the RGB led to signal all good.
void signal_oke()
{
  digitalWrite(LEDG, LOW);
  delay(500);
  digitalWrite(LEDG, HIGH);
  delay(500);
}

// Connect to WiFi.
void connect_to_wifi()
{
  Serial.print("Connecting to SSID: ");
  Serial.println(SECRET_SSID);
  
  auto status = WiFi.begin(SECRET_SSID, SECRET_PASS);
  
  while(WL_CONNECTED != status)
  {
    signal_wifi_disconnect();
        
    // wait 1 seconds for connection:
    delay(1000);
    
    status = WiFi.begin(SECRET_SSID, SECRET_PASS);
  }
  
  Serial.println("You're connected to the network");
}

void setup()
{
  // Initialize digital pin LED_BUILTIN as an output.
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  // Switch off the LEDs.
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDB, HIGH);
  
  // Initialize the serial port to output debug information.
  Serial.begin(9600);
  while(!Serial)
  {
    // Signal an error
    signal_error();
  }
  Serial.println();
  Serial.println();

  connect_to_wifi();
  
  // Check for the WiFi module.
  if(WL_NO_SHIELD == WiFi.status())
  {
    Serial.println("Communication with WiFi module failed!");
    while (true) 
    {
      signal_error();
    }
  }

  // Set the default value in the settings object.
  constexpr int DEFAULT_PERIOD_SEC = 5;
  weather_settings.set_update_period_sec(DEFAULT_PERIOD_SEC);

  // Request the settings from the server.
  if(client.connect(SERVER_IP, SERVER_PORT)) {
    Serial.println("Request settings from the server.");
    client.println("GET /api/settings HTTP/1.1");
    client.println("Host: " + String(SERVER_IP) + "/api/settings");
    client.println("Connection: close");
    client.println();
  }
  else {
    Serial.println("Unable to connect to server");
  }
}

void receive_settings()
{
  string = client.readStringUntil('\n'); // HTTP/1.1 200 OK      
  if("HTTP/1.1 200 OK\r" == string) {
    string = client.readStringUntil('\n'); // Content-Type: application/x-protobuf
    string = client.readStringUntil('\n'); // Date
    string = client.readStringUntil('\n'); // Connection: close
    string = client.readStringUntil('\n'); // Transfer-Encoding: chunked     
    string = client.readStringUntil('\n'); // Empty string
    
    string = client.readStringUntil('\n'); // Number of data bytes as a string.
    const int n_bytes_data = string.toInt();
    const int n_bytes_received = client.readBytes(read_buffer.get_data_array(), min(n_bytes_data, BUFFER_SIZE));
    read_buffer.set_bytes_written(n_bytes_received);
    
    if(n_bytes_received >= n_bytes_data)
    {
      const auto result = weather_settings.deserialize(read_buffer);
      if(EmbeddedProto::Error::NO_ERRORS == result) 
      {
        Serial.println("Settings received: update_period_sec=" + String(weather_settings.get_update_period_sec()));
        
      }
      else 
      {
        Serial.println("Failed to deserialize settings.");
      }
    }

    read_buffer.clear();
  }
}

void loop() 
{
  if(WL_CONNECTED == WiFi.status())
  {
    signal_oke();
    
    if(client.available()) {
      receive_settings();
    }

    if(millis() >= update_time) 
    {      
      // Fake reading data
      weather_data.set_temperature(21.0F);

      const auto result = weather_data.serialize(write_buffer);
      if(EmbeddedProto::Error::NO_ERRORS == result)
      {
        if(client.connect(SERVER_IP, SERVER_PORT)) {
          Serial.println("Sending N bytes to the server: w" + String(write_buffer.get_size()));
          for(int i = 0; i < write_buffer.get_size(); ++i)
          {
            const byte b = *(write_buffer.get_data() + i);
            if(16 > b) { Serial.print("0"); }
            Serial.print(b, HEX);
            Serial.print(" ");
          }
          Serial.println();
          
          client.println("POST /api/data HTTP/1.1");
          client.println("Host: " + String(SERVER_IP) + "/api/data");
          client.println("Content-Type: application/octet-stream");
          client.println("Content-Length: " + String(write_buffer.get_size()));
          client.write(write_buffer.get_data(), write_buffer.get_size());
          client.println();
          client.println('0');
          client.println();
          
          delay(500);

          while(-1 != client.read()) {
            
          }
        }
        else
        {
          Serial.println("Failed to connect to server.");
        }
      }
      write_buffer.clear();
      
      // Reset the timer for the next update.
      update_time = millis() + (1000 * weather_settings.get_update_period_sec());
    }
  }
  else 
  {
    connect_to_wifi();
  }
}
