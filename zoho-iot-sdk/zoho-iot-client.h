#ifndef ZOHO_IOT_CLIENT_H_
#define ZOHO_IOT_CLIENT_H_
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

class ZohoIOTClient
{
private:
  // Client *nw_client;
  WiFiClient _wifi_client;
  PubSubClient *_mqtt_client;
  char *_wifi_ssid;
  char *_wifi_password;
  char *_device_id;
  char *_device_token;
  const char *_mqtt_server = "172.22.138.253"; //Shahul IP
  const unsigned int _port = 1883;

public:
  ZohoIOTClient();
  void setWifiParams(char *ssid, char *password);
  void setHUBConnectionParams(char *device_id, char *device_token);
  void connect_WIFI();
  void connect_IOT_HUB();
};

#endif