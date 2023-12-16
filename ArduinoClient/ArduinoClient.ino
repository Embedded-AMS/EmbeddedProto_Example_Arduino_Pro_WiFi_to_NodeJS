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

#include <WiFi.h>
#include <time.h>

#include "WeatherSettings.h"
#include "WeatherData.h"


// Please enter your configurations in the settings.h file:
//  * WiFi name (SSID)
//  * WiFi password
//  * Server IP
//  * Server Port number
#include "settings.h" 

// Setup global variables.
WiFiClient client;
WeatherSettings weather_settings(client);
WeatherData weather_data(client);

String string;
int iteration_counter = 0;
unsigned long update_time = 0;
unsigned long now_time = 0;

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

  // Request the settings from the server.
  weather_settings.request_from_server();
}



// The main loop of the program.
void loop() 
{
  now_time = millis();
  if(WL_CONNECTED == WiFi.status())
  {    
    signal_oke();
    
    if(now_time >= update_time) 
    {      
      weather_data.update();
      weather_data.serialize_and_send();
      
      // Reset the timer for the next update.
      update_time = now_time + (1000 * weather_settings.get().get_update_period_sec());
    }
  }
  else 
  {
    connect_to_wifi();
  }
}
