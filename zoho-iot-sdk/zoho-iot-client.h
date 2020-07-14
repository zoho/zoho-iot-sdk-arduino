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
#define event "/events"

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

  typedef union
  {
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

  PubSubClient *_mqtt_client;
  char *_mqtt_user_name;
  char *_mqtt_password;
  char *_client_id;
  char *_mqtt_server;
  int _port;
  char *_publish_topic, *_command_topic, *_event_topic;
  const unsigned int _retry_limit = 5;
  clientState currentState;
  unsigned int retryCount = 0;

  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  JsonObject &eventDataObject = jsonBuffer.createObject();
  template <typename T>
  inline bool addDataPoint(const char *key, T val, const char *assetName)
  {
    if (key == NULL)
    {
      return false;
    }
    if (checkStringIsValid(assetName))
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

  template <typename T>
  inline bool addEventDataPoint(const char *key, T val)
  {
    if (key == NULL || strcmp(key, "") == 0)
    {
      // Serial.println("Cant add Null/empty keys");
      return false;
    }
    eventDataObject[key] = val;
    return true;
  }

  inline void setPort(bool isTLSEnabled)
  {
    _port = isTLSEnabled ? 8883 : 1883;
  }
  void formMqttPublishTopic(char *clientID);
  bool extractMqttServerAndDeviceDetails(const string &mqttUserName);
  char *formConnectionString(char *username);
  inline bool checkStringIsValid(const char *value)
  {
    return (value == NULL || strcmp(value, "") == 0) ? false : true;
  }

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
  int dispatchEventFromJSONString(char *eventType, char *eventDescription, char *eventDataJSONString, char *assetName);
  int dispatchEventFromEventDataObject(char *eventType, char *eventDescription, char *assetName);
  int subscribe(char *topic, MQTT_CALLBACK_SIGNATURE);
  int disconnect();
  inline void zyield()
  {
    _mqtt_client->loop();
  }

  inline bool addEventDataPointNumber(const char *key, double value)
  {
    return addEventDataPoint(key, value);
  }
  inline bool addEventDataPointString(const char *key, char *value)
  {
    return checkStringIsValid(value) ? addEventDataPoint(key, value) : false;
  }
  inline bool addEventDataPointString(const char *key, string value)
  {
    return (value.empty() == true) ? false : addEventDataPoint(key, value.c_str());
  }

  // Adding Double
  inline bool addDataPointNumber(const char *key, double value)
  {
    return addDataPoint(key, value, NULL);
  }
  inline bool addDataPointNumber(const char *key, double value, const char *assetName)
  {
    return addDataPoint(key, value, assetName);
  }

  // Adding Char*
  inline bool addDataPointString(const char *key, const char *value)
  {
    return (value == NULL) ? false : addDataPoint(key, value, NULL);
  }
  inline bool addDataPointString(const char *key, const char *value, const char *assetName)
  {
    return (value == NULL) ? false : addDataPoint(key, value, assetName);
  }

  // Adding String
  inline bool addDataPointString(const char *key, string value)
  {
    return (value.empty() == true) ? false : addDataPoint(key, value.c_str(), NULL);
  }
  inline bool addDataPointString(const char *key, string value, const char *assetName)
  {
    return (value.empty() == true) ? false : addDataPoint(key, value.c_str(), assetName);
  }

  // Adding Error
  inline bool markDataPointAsError(const char *key)
  {
    return addDataPoint(key, "<ERROR>", NULL);
  }
  inline bool markDataPointAsError(const char *key, const char *assetName)
  {
    return addDataPoint(key, "<ERROR>", assetName);
  }
};
#endif