#ifndef ZOHO_IOT_CLIENT_H_
#define ZOHO_IOT_CLIENT_H_
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <map>
using namespace std;

class ZohoIOTClient
{
private:
  typedef enum
  {
    TYPE_DOUBLE,
    TYPE_INT,
    TYPE_CHAR
  } value_types;

  typedef union {
    int i_val;
    double d_val;
    const char *s_val;
  } value;

  typedef struct data
  {
    value_types type;
    value val;
    data(value_types datatype, int value)
    {
      type = datatype;
      val.i_val = value;
    }
    data(value_types datatype, double value)
    {
      type = datatype;
      val.d_val = value;
    }
    data(value_types datatype, const char *value)
    {
      type = datatype;
      val.s_val = value;
    }
  } data;

  PubSubClient _mqtt_client;
  char *_device_id;
  char *_device_token;
  const char *_mqtt_server = "172.22.138.253"; //Shahul IP
  const unsigned int _port = 1883;

  std::map<const char *, data> dataPointsMap;
  template <typename T>
  inline bool addDataPoint(const char *key, value_types type, T val)
  {
    if (type == TYPE_INT)
    {
      data dp(type, val);
      dataPointsMap.insert(make_pair(key, dp));
    }
    else if (type == TYPE_DOUBLE)
    {
      data dp(type, val);
      dataPointsMap.insert(make_pair(key, dp));
    }
    else if (type == TYPE_CHAR)
    {
      data dp(type, val);
      dataPointsMap.insert(make_pair(key, dp));
    }
    else
    {
      Serial.println("Error: Unsupported data type.");
      return false;
    }
    return true;
  }

public:
  inline ZohoIOTClient(Client &client) : _mqtt_client(client) {}
  inline ~ZohoIOTClient() {}
  void init(char *device_id, char *device_token);
  bool connect();

  inline bool addDataPointNumber(const char *key, int value)
  {
    return addDataPoint(key, TYPE_INT, value);
  }
  inline bool addDataPointNumber(const char *key, double value)
  {
    return addDataPoint(key, TYPE_DOUBLE, value);
  }
  inline bool addDataPointString(const char *key, const char *value)
  {
    return addDataPoint(key, TYPE_CHAR, value);
  }
  inline bool addDataPointString(const char *key, string value)
  {
    return addDataPoint(key, TYPE_CHAR, value.c_str());
  }
};

#endif