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

#include "GetConfig.h"

// Load the header file with the generated *.proto code.
#include "weather.h"

// Load some files required from the Embedded Proto library.
#include <WriteBufferFixedSize.h>
#include <Errors.h>

// Please enter your configurations in the settings.h file:
//  * WiFi name (SSID)
//  * WiFi password
//  * Server IP
//  * Server Port number
#include "settings.h" 

// Setup global variables.
WiFiClient client;
String string;
int iteration_counter = 0;
unsigned long update_time = 0;

// Embedded Proto data message objects.
weather::Data weather_data;
weather::Settings weather_settings;

// Embedded Proto buffers used for serializing and deserializing messages.
constexpr int BUFFER_SIZE = 256;
EmbeddedProto::WriteBufferFixedSize<BUFFER_SIZE> write_buffer;



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
  
  if(WL_CONNECTED != status)
  {
    signal_wifi_disconnect();
    Serial.println("Unable to connect to the network.");
  }
  else 
  {
    Serial.println("You're connected to the network.");
  }

  // Check for the WiFi module.
  if(WL_NO_SHIELD == WiFi.status())
  {
    Serial.println("Communication with WiFi module failed!");
    while (true) 
    {
      signal_error();
    }
  }
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

  connect_to_wifi();

  // Set the default value in the settings object.
  constexpr int DEFAULT_PERIOD_SEC = 5;
  weather_settings.set_update_period_sec(DEFAULT_PERIOD_SEC);

  // Request the settings from the server.
  get_config::request_settings_from_server(client, weather_settings);
}


// Read weather data from the sensors. In this example we generate semi random data.
void get_sensor_data()
{
  const float time_sec = millis() / 1000.0F;
  static int counter = 0;

  ++counter;
  
  // Acttualy fake reading data
  const float temperature = 21.0F + 4.5 * cos(6.28F * time_sec / 120.0F); 
  weather_data.set_temperature(temperature);

  const float humidity = 40.0F + 40.0F * sin(6.28F * time_sec / 160.0F);
  weather_data.set_humidity(humidity);

  const float air_pressure = 1024 + int(10*(counter % 40));
  weather_data.set_air_pressure(air_pressure);

  const float wind_speed = (16.0F + (3.0 * cos(6.28F * time_sec / 220.0F))) * random(9000, 11000)/10000;
  weather_data.set_wind_speed(wind_speed);

  const float wind_direction = (360.0F * (0.5 + 0.5*cos(6.28F * time_sec / 1000.0F))) * random(9000, 11000)/10000;
  weather_data.set_wind_direction(wind_direction);
}

// Send weather data to the server.
bool send_weather_data()
{
  bool result = false;
  
  // Serialize the weather data.
  const auto serialize_result = weather_data.serialize(write_buffer);

  // If all went well start connecting to the server.
  if(EmbeddedProto::Error::NO_ERRORS == serialize_result)
  {
    if(client.connect(SERVER_IP, SERVER_PORT)) 
    {
      Serial.println("Sending N bytes to the server: " + String(write_buffer.get_size()));
      for(int i = 0; i < write_buffer.get_size(); ++i)
      {
        const byte b = *(write_buffer.get_data() + i);
        Serial.print(b);
        Serial.print(" ");
      }
      Serial.println();
      
      client.println("POST /api/data HTTP/1.1");
      client.println("Host: " + String(SERVER_IP) + "/api/data");
      client.println("Content-Type: application/x-protobuf");
      client.println("Connection: close");
      client.println("Transfer-Encoding: chunked");
      client.println("");
      client.println(write_buffer.get_size(), HEX);
      client.write(write_buffer.get_data(), write_buffer.get_size());
      client.println();
      client.println('0');
      client.println();
      
      delay(500);

      Serial.println();
      while(client.available()) 
      {
          string = client.readStringUntil('\n');
          Serial.println(string);
          if(string.startsWith(""))
          {
            result = true;
          }
      }

      client.stop();
    }
    else
    {
      Serial.println("Failed to connect to server.");
      client.stop();
    }
  }

  // Clear the write buffer after using it.
  write_buffer.clear();

  return result;
}


// The main loop of the program.
void loop() 
{
  if(WL_CONNECTED == WiFi.status())
  {
    signal_oke();
    
    if(millis() >= update_time) 
    {      
      get_sensor_data();

      send_weather_data();
      
      // Reset the timer for the next update.
      update_time = millis() + (1000 * weather_settings.get_update_period_sec());
    }
  }
  else 
  {
    connect_to_wifi();
  }
}
