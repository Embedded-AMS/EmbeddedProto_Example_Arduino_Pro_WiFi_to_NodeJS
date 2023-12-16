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

#ifndef WEATHER_CONFIG_H
#define WEATHER_CONFIG_H

// From arduino include the Client base class.
#include <WiFi.h>

// Load the header file with the generated *.proto code.
#include "weather.h"

// Load some files required from the Embedded Proto library.
#include <ReadBufferFixedSize.h>
#include <Errors.h>

class WeatherSettings 
{
  static constexpr int BUFFER_SIZE = 256;

  // Set the default value in the settings object.
  static constexpr int DEFAULT_PERIOD_SEC = 5;

  public:
    WeatherSettings(Client& client);
    ~WeatherSettings() = default;
    
    bool request_from_server();

    const weather::Settings& get() const;

  private:
    bool get_data_buffer_from_server();
  
    Client& client_;

    EmbeddedProto::ReadBufferFixedSize<BUFFER_SIZE> read_buffer_;

    ::weather::Settings weather_settings_;
  
  
}; // End of class get_config

#endif // WEATHER_CONFIG_H
