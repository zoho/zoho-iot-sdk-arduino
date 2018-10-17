#ifndef ZOHO_IOT_CLIENT_H_
#define ZOHO_IOT_CLIENT_H_
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

class ZohoIOTClient
{
private:
  PubSubClient _mqtt_client;
  char *_device_id;
  char *_device_token;
  const char *_mqtt_server = "172.22.138.253"; //Shahul IP
  const unsigned int _port = 1883;

public:
  inline ZohoIOTClient(Client &client) : _mqtt_client(client) {}
  inline ~ZohoIOTClient() {}
  void init(char *device_id, char *device_token);
  bool connect();
};

#endif