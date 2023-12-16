/*
 *  Copyright (C) 2020-2024 Embedded AMS B.V. - All Rights Reserved
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
 *    Atoomweg 2
 *    1627 LE, Hoorn
 *    the Netherlands
 */
  
// Please enter your configurations in the settings.h file:
//  * WiFi name (SSID)
//  * WiFi password
//  * Server IP
//  * Server Port number
#include "settings.h"

#include "WeatherData.h"

#include <time.h>

WeatherData::WeatherData(Client& client) : client_(client)
{

}

void WeatherData::update()
{
  const float time_sec = millis() / 1000.0F;

  ++counter_;
  
  // Acttualy fake reading data
  const float temperature = 21.0F + 4.5 * cos(6.28F * time_sec / 120.0F); 
  weather_data_.set_temperature(temperature);

  const float humidity = 40.0F + 40.0F * sin(6.28F * time_sec / 160.0F);
  weather_data_.set_humidity(humidity);

  const float air_pressure = 1024 + int(10*(counter_ % 40));
  weather_data_.set_air_pressure(air_pressure);

  const float wind_speed = (16.0F + (3.0 * cos(6.28F * time_sec / 220.0F))) * random(9000, 11000)/10000;
  weather_data_.set_wind_speed(wind_speed);

  const float wind_direction = (360.0F * (0.5 + 0.5*cos(6.28F * time_sec / 1000.0F))) * random(9000, 11000)/10000;
  weather_data_.set_wind_direction(wind_direction);
}

bool WeatherData::serialize_and_send()
{  
  // Serialize the weather data.
  const auto serialize_result = weather_data_.serialize(write_buffer_);

  // If all went well start connecting to the server.
  bool result = (EmbeddedProto::Error::NO_ERRORS == serialize_result) && post();


  // Clear the write buffer after using it.
  write_buffer_.clear();
  
  return result;
  
}

bool WeatherData::post()
{
  bool result = false;
  if(client_.connect(SERVER_IP, SERVER_PORT)) 
  {
    Serial.println("Sending N bytes to the server: " + String(write_buffer_.get_size()));
    for(int i = 0; i < write_buffer_.get_size(); ++i)
    {
      const byte b = *(write_buffer_.get_data() + i);
      Serial.print(b);
      Serial.print(" ");
    }
    Serial.println();

    Serial.println("POST /api/data HTTP/1.1");
    Serial.println("Host: " + String(SERVER_IP) + "/api/data");
    Serial.println("Content-Type: application/x-protobuf");
    Serial.println("Connection: close");
    Serial.println("Transfer-Encoding: chunked");
    Serial.println("");
    Serial.println(write_buffer_.get_size(), HEX);
    Serial.write(write_buffer_.get_data(), write_buffer_.get_size());
    Serial.println();
    Serial.println('0');
    Serial.println();
    
    client_.println("POST /api/data HTTP/1.1");
    client_.println("Host: " + String(SERVER_IP) + "/api/data");
    client_.println("Content-Type: application/x-protobuf");
    client_.println("Connection: close");
    client_.println("Transfer-Encoding: chunked");
    client_.println("");
    client_.println(write_buffer_.get_size(), HEX);
    client_.write(write_buffer_.get_data(), write_buffer_.get_size());
    client_.println();
    client_.println('0');
    client_.println();
    
    // Next wait for the response.
    Serial.println();

    delay(500);
    
    while(client_.available()) 
    {
        String string = client_.readStringUntil('\n');
        Serial.println(string);
        if(string.startsWith(""))
        {
          result = true;
        }
    }
  }
  else
  {
    Serial.println("Failed to connect to server.");
  }
  
  client_.stop();
  return result;
}
