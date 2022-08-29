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
 

#ifndef WEATHER_DATA_H
#define WEATHER_DATA_H

// From arduino include the Client base class.
#include <WiFi.h>

// Load the header file with the generated *.proto code.
#include "weather.h"

// Load some files required from the Embedded Proto library.
#include <WriteBufferFixedSize.h>
#include <Errors.h>

class WeatherData
{
  static constexpr int BUFFER_SIZE = 256;

  public:
    WeatherData(Client& client);
    ~WeatherData() = default;

    // Update the data in the weather protobuf object.
    void update();

    // Send the weather data to the server.
    bool serialize_and_send();

  private:
    bool post();
  
    Client& client_;

    // Embedded Proto data message objects.
    weather::Data weather_data_;

    // Embedded Proto buffers used for serializing and deserializing messages.
    EmbeddedProto::WriteBufferFixedSize<BUFFER_SIZE> write_buffer_;

    int counter_ = 0;
};


#endif // WEATHER_DATA_H
