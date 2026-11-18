
![alt text](https://embeddedproto.com/wp-content/uploads/2022/04/Embedded_Proto.png "Embedded Proto Logo")


Embedded Proto is a product of Embedded AMS B.V. For more information about Embedded Proto please visit [EmbeddedProto.com](https://EmbeddedProto.com).

Copyrights 2020-2026 Embedded AMS B.V. Amsterdam, [www.EmbeddedAMS.nl](https://www.EmbeddedAMS.nl), [info@EmbeddedAMS.nl](mailto:info@EmbeddedAMS.nl)

![alt text](https://embeddedproto.com/wp-content/uploads/2022/08/ArduinoPro_Example_Featured_Image.jpg "Arduino to Server")

# Introduction

This repository hosts example code for Embedded Proto, the embedded implementation of Google Protocol Buffers. It is a simple example showing how an Arduino sends data through a server to a front end.

Following this tutorial, you will learn how an IoT system works from the device through the server to the front end. 

![alt text](https://embeddedproto.com/wp-content/uploads/2022/08/ActualFrontEnd.jpg "Front end")

You can read the full tutorial [here](https://embeddedproto.com/a-simple-arduino-iot-example-with-protobuf/).

# Requirements
An Arduino with WiFi. An Arduino Pro Portenta H7 was used in this example.
* Arduino IDE, we used version 1.8.19.
* NPM, we used version 7.11.2.
* NodeJS, we used version 10.19.0.
* Embedded Proto version 4.0.0 (included as a submodule). Its only requirement is Python 3.11 or newer, see the [installation instructions](https://embeddedproto.com/documentation/installation/).

# Installation
This is the short installation instruction. For instructions and explanations, please read the full [tutorial](https://embeddedproto.com/a-simple-arduino-iot-example-with-protobuf/).
```shell
git clone --recursive https://github.com/Embedded-AMS/EmbeddedProto_Example_Arduino_Pro_WiFi_to_NodeJS.git 
cd embeddedproto_example_arduino_pro_wifi_to_nodejs
python setup.py
mkdir ~/Arduino/libraries/EmbeddedProto_Library
cp -r EmbeddedProto/src/* ~/Arduino/libraries/EmbeddedProto_Library/
cd NodeServer
npm install
```

The setup script runs the install script of Embedded Proto, which creates a virtual environment in the submodule folder, and generates the source code with the `embeddedproto` command. Add `--installer uv` when you have [uv](https://docs.astral.sh/uv/) installed, it is a lot faster. The library is header only and requires C++17, which the Arduino cores for the Portenta already use.

# Running the code
Upload the sketch using the Arduino IDE. When connected over USB, you can view the debug information over the com port. 

To run the server:
```shell
npm run server
```
Debug information will be shown in the terminal. The URL on which you can find the front end is shown at the start of the output.

Have fun!

---

Three simple things you can do to help improve Embedded Proto: 
* Give private [feedback](https://EmbeddedProto.com/feedback).
* Report an issue in public on [Github](https://github.com/Embedded-AMS/EmbeddedProto/issues).
* Stay up to date on Embedded Proto via our [User mailing list](https://EmbeddedProto.com/signup).

