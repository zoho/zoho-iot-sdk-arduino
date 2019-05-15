# Zoho IoT Arduino SDK

## Overview

This Zoho IoT SDK for Arduino will provide support for the famous IOT devices like ESP8266, ESP32 & Arduino Series of devices. It will simplify the process of establishing connection to the Zoho cloud platform. The lightweight MQTT messaging protocol is used to exchange the data between the device & cloud.

## Features:

### Connectivity

This SDK will help your device to establish a reliable MQTT connection with the server as minimal as much as possible in order to deliver telemetry data from the device and to receive commands from the cloud.

### Data Collection

This SDK has inbuilt support for JSON data format. It will allow you to effortlessly generate Telemetry payload based on the polled data from sensors. For the advanced usage, it will provide you the flexibility to form your own JSON structure.

### Device Compatibility

This SDK will act as a library for Arduino Development Environment. The list of targeted devices are:

- ESP 8266
- ESP 32
- Arduino devices (with Ethernet Shield / ESP01 module)

> As a library to Arduino development platform, it can support all the devices (with networking options) that are supported by Arduino IDE.

## Setup Instructions

### Dependencies

Make sure you are already installed the below libraries on your Arduino IDE:

- [`Async-mqtt-client`](https://github.com/marvinroger/async-mqtt-client)
- [`ESPAsyncTCP (ESP8266)`](https://github.com/me-no-dev/ESPAsyncTCP)
- [`AsyncTCP (ESP32)`](https://github.com/me-no-dev/AsyncTCP)
- `ArduinoJson`
- `Wifi / Ethernet drivers based on your device type`

If not installed , download them using "Download ZIP" option from the respective repositories and unzip them in libraries folder of your Arduino IDE or import them in process similar to importing our SDK as mentioned below.
### Adding the Library

Follow the below steps to import this SDK as Library in Arduino:

- Download this SDK by choosing "Download ZIP" option.
- Unzip the downloaded files.
- In Arduino IDE, choose `Sketch` -> `Import Library` -> `Add Library`
- From the downloaded location, choose the folder `zoho-iot-sdk`.

### Tying out the Examples

This SDK has some default examples to try some basic operations. Please find it examples folder and follows its appropriate readme instructions.
