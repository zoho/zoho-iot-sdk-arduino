# Zoho IOT Arduino SDK

## Overview

The Zoho IOT SDK for Arduino supports popular IoT devices such as the ESP8266, ESP32, and Arduino series. It simplifies the process of connecting to Zoho IOT application. The SDK uses the lightweight MQTT messaging protocol to facilitate data exchange between the device and the cloud.

## Features:

### Connectivity

This SDK helps your device establish a reliable MQTT connection with the server, ensuring minimal overhead while delivering telemetry data and receiving commands from the cloud.

### Data Collection

The SDK has built-in support for JSON data formatting, allowing you to effortlessly generate telemetry payloads based on sensor data. For advanced requirements, it offers the flexibility to create custom JSON structures.

### Device Compatibility

The SDK functions as a library within the Arduino Development Environment. It targets the following devices:

- ESP 8266
- ESP 32
- Arduino devices (with Ethernet Shield / ESP01 module)

> As a library to Arduino development platform, it can support all the devices (with networking options) that are supported by Arduino IDE.

## Setup Instructions

### Dependencies

Make sure you are already installed the below libraries on your Arduino IDE:

- `PubSubClient`
- `ArduinoJson`
- `Wifi / Ethernet drivers based on your device type`

If these libraries are not installed, download them using the "Download ZIP" option from the respective repositories, unzip them into the libraries folder of your Arduino IDE, or import them using the process described below.

### Adding the Library

Follow the below steps to import this SDK as Library in Arduino:

- Download the latest version of SDK using this link https://github.com/zoho/zoho-iot-sdk-arduino/archive/refs/tags/0.1.0.zip
- In Arduino IDE, choose `Sketch` -> `Include Library` -> `Add .ZIP Library..`
- From the downloaded location, choose the `zoho-iot-sdk-arduino-0.1.0.zip`.

### Tying out the Examples
The SDK comes with default example sketches. You can find them under `File` -> `Examples` -> `ZOHO-IOT-SDK` within the Arduino IDE.