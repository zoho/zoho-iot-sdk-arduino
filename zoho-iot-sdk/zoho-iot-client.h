#ifndef ZOHO_IOT_CLIENT_H_
#define ZOHO_IOT_CLIENT_H_

#include <stdlib.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <map>
#include <cstring>
#include <vector>

#define topic_prefix "/devices/"
#define telemetry "/telemetry"

#define sdk_name (char *)"zoho-iot-sdk-c"
#define sdk_version (char *)"0.0.1"
#define sdk_url (char *)""

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

  typedef enum
  {
    NOT_INITIALIZED,
    INITIALIZED,
    CONNECTED,
    RETRYING,
    CONNECTION_LOST,
    DISCONNECTED
  } clientState;

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

  PubSubClient *_mqtt_client;
  char *_mqtt_user_name;
  char *_mqtt_password;
  char *_client_id;
  char *_mqtt_server;
  int _port;
  char *_publish_topic, *_command_topic;
  const unsigned int _retry_limit = 5;
  clientState currentState;
  unsigned int retryCount = 0;

  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  template <typename T>
  inline bool addDataPoint(const char *key, value_types type, T val, const char *assetName)
  {
    if (key == NULL)
    {
      return false;
    }
    if (type != TYPE_INT && type != TYPE_CHAR && type != TYPE_DOUBLE)
    {
      // Serial.println("Error: Unsupported data type.");
      // TRACE("Error: Unsupported data type.");
      return false;
    }
    if (assetName != NULL && assetName != "")
    {
      if (!root.containsKey(assetName))
      {
        root.createNestedObject(assetName);
      }
      JsonObject &obj = root[assetName];
      obj[key] = val;
    }
    else
    {
      root[key] = val;
    }
    return true;
  }
  inline void setPort(bool isTLSEnabled)
  {
    _port = isTLSEnabled ? 8883 : 1883;
  }
  void formMqttPublishTopic(char *clientID);
  bool extractMqttServerAndDeviceDetails(const string &mqttUserName);
  char *formConnectionString(char *username);

public:
  inline ZohoIOTClient(Client *client, bool isTLSEnabled)
  {
    _mqtt_client = new PubSubClient(*client);
    setPort(isTLSEnabled);
    currentState = NOT_INITIALIZED;
  }
  inline ZohoIOTClient(PubSubClient *pubSubClient, bool isTLSEnabled)
  {
    _mqtt_client = pubSubClient;
    setPort(isTLSEnabled);
    currentState = NOT_INITIALIZED;
  }
  inline ~ZohoIOTClient() {}
  int init(char *mqttUserName, char *mqttPassword);
  void addConnectionParameter(char *connectionParamKey, char *connectionParamValue);
  int connect();
  int publish(char *message);
  int dispatch();
  int subscribe(char *topic, MQTT_CALLBACK_SIGNATURE);
  int disconnect();
  inline void zyield()
  {
    _mqtt_client->loop();
  }

  // Adding Integer
  inline bool addDataPointNumber(const char *key, int value, const char *assetName)
  {
    return addDataPoint(key, TYPE_INT, value, assetName);
  }
  inline bool addDataPointNumber(const char *key, int value)
  {
    return addDataPoint(key, TYPE_INT, value, NULL);
  }

  // Adding Double
  inline bool addDataPointNumber(const char *key, double value)
  {
    return addDataPoint(key, TYPE_DOUBLE, value, NULL);
  }
  inline bool addDataPointNumber(const char *key, double value, const char *assetName)
  {
    return addDataPoint(key, TYPE_DOUBLE, value, assetName);
  }

  // Adding Char*
  inline bool addDataPointString(const char *key, const char *value)
  {
    if (value == NULL)
    {
      return false;
    }
    return addDataPoint(key, TYPE_CHAR, value, NULL);
  }
  inline bool addDataPointString(const char *key, const char *value, const char *assetName)
  {
    if (value == NULL)
    {
      return false;
    }
    return addDataPoint(key, TYPE_CHAR, value, assetName);
  }

  // Adding String
  inline bool addDataPointString(const char *key, string value)
  {
    if (value.empty())
    {
      return false;
    }
    return addDataPoint(key, TYPE_CHAR, value.c_str(), NULL);
  }
  inline bool addDataPointString(const char *key, string value, const char *assetName)
  {
    if (value.empty())
    {
      return false;
    }
    return addDataPoint(key, TYPE_CHAR, value.c_str(), assetName);
  }

  // Adding Error
  inline bool markDataPointAsError(const char *key)
  {
    return addDataPoint(key, TYPE_CHAR, "<ERROR>", NULL);
  }
  inline bool markDataPointAsError(const char *key, const char *assetName)
  {
    return addDataPoint(key, TYPE_CHAR, "<ERROR>", assetName);
  }
};
#endif