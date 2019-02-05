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

  typedef enum
  {
    SUCCESS = 0,
    FAILURE = -1,
    CONNECTION_ERROR = -2
  } transactionStatus;

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
  // const char *_mqtt_server = "172.22.138.253"; //Shahul IP
  const char *_mqtt_server = "172.22.142.33"; //kishan IP
  const unsigned int _port = 1883;
  const char *_publish_topic = "test_topic9876";
  const unsigned int _retry_limit = 5;

  std::map<std::string, data> dataPointsMap;

  template <typename T>
  inline bool addDataPoint(const char key[], value_types type, T val)
  {
    if (type == TYPE_INT || type == TYPE_CHAR || type == TYPE_DOUBLE)
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
  int connect();
  int dispatch();
  int publish(char *message);
  int subscribe(char *topic, MQTT_CALLBACK_SIGNATURE);
  inline void yield()
  {
    _mqtt_client.loop();
  }

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