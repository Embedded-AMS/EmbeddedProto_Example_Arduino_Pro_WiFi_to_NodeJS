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

#ifndef GET_CONFIG_H
#define GET_CONFIG_H

// Please enter your configurations in the settings.h file:
//  * WiFi name (SSID)
//  * WiFi password
//  * Server IP
//  * Server Port number
#include "settings.h"

// Load the header file with the generated *.proto code.
#include "weather.h"

// Load some files required from the Embedded Proto library.
#include <ReadBufferFixedSize.h>
#include <Errors.h>

namespace get_config 
{
  constexpr int BUFFER_SIZE = 256;
  EmbeddedProto::ReadBufferFixedSize<BUFFER_SIZE> read_buffer;
  
  // Request the settings from the server.
  bool request_settings_from_server(Client& client, weather::Settings& weather_settings)
  {
    bool result = false;
    Serial.println("Request settings from the server.");
    if(client.connect(SERVER_IP, SERVER_PORT)) 
    {
      client.println("GET /api/settings HTTP/1.1");
      client.println("Host: " + String(SERVER_IP) + "/api/settings");
      client.println("Connection: close");
      client.println();
  
      // Wait for the response
      //delay(1500);
        
      String string = client.readStringUntil('\n'); 
      Serial.println(string); // HTTP/1.1 200 OK      
      if(string.startsWith("HTTP/1.1 200 OK"))
      {
        // Read untill an empty line
        do 
        {
          string = client.readStringUntil('\n'); 
          Serial.println(string);
        } while(!string.startsWith("\r"));
  
        // After the empty line the data lenght follows.
        string = client.readStringUntil('\n'); 
        const int n_bytes_data = string.toInt();
        Serial.println("n_bytes_data: " + String(n_bytes_data));
  
        // Read the data bytes
        const int n_bytes_received = client.readBytes(read_buffer.get_data_array(), min(n_bytes_data, BUFFER_SIZE));
  
        // Print out any more data from the server.
        while(client.available())
        {
          string = client.readStringUntil('\n'); 
          Serial.println(string);
        }
  
        // Print out the data human redable.
        Serial.println("n_bytes_received: " + String(n_bytes_received));
        for(int i = 0; i < n_bytes_received; ++i)
        {
          Serial.print(*(read_buffer.get_data_array() + i), HEX);
          Serial.print(" "); 
        }
        Serial.println(""); 
  
        // Set the number of bytes received.
        read_buffer.set_bytes_written(n_bytes_received);
  
        // Deserialize the settings from the read buffer.
        const auto desrialize_result = weather_settings.deserialize(read_buffer);
        if(EmbeddedProto::Error::NO_ERRORS == desrialize_result) 
        {
          result = true;
          Serial.println("Settings received: update_period_sec=" + String(weather_settings.get_update_period_sec()));
        }
        else 
        {
          Serial.println("Failed to deserialize settings.");
        }
  
        // Clear the read buffer.
        read_buffer.clear(); 
      }
      else 
      {
        Serial.println("Requesting settings failed.");
      }
  
      client.stop();
    }
    else 
    {
      Serial.println("Unable to connect to server");
      client.stop();
    }
    return result;
  }
  
} // End of namespace get_config

#endif // GET_CONFIG_H
