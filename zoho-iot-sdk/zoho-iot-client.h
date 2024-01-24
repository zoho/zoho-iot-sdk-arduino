#ifndef ZOHO_IOT_CLIENT_H_
#define ZOHO_IOT_CLIENT_H_

#include <stdlib.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <map>
#include <cstring>
#include <vector>

#define topic_prefix "/devices/"
#define telemetry "/telemetry"
#define event "/events"
#define command "/commands"
#define commandAck "/commands/ack"

#define sdk_name (char *)"zoho-iot-sdk-arduino"
#define sdk_version (char *)"0.0.1"
#define sdk_url (char *)""

#define COMMAND_RECIEVED_ACK_CODE 1000
#define MAX_RETRY_INTERVAL 1800
#define MIN_RETRY_INTERVAL 2
#define MAX_PAYLOAD_SIZE (int)100000
#define DEFAULT_PAYLOAD_SIZE (int)32000

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
    NOT_INITIALIZED,
    INITIALIZED,
    CONNECTED,
    RETRYING,
    CONNECTION_LOST,
    DISCONNECTED
  } clientState;

  unsigned long current_retry_interaval = (unsigned long)MIN_RETRY_INTERVAL;
  PubSubClient *_mqtt_client;
  const char *_mqtt_user_name;
  const char *_mqtt_password;
  char *_client_id;
  char *_mqtt_server;
  uint16_t _port;
  char *_publish_topic, *_command_topic, *_event_topic, *_command_ack_topic;
  const uint16_t _retry_limit = 5;
  clientState currentState;
  uint16_t retryCount = 0;
  JsonDocument root;
  JsonDocument eventDataObject;


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
        root[assetName].to<JsonObject>();
      }
      JsonObject obj = root[assetName];
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
  void formMqttPublishTopic(const char *clientID);
  bool extractMqttServerAndDeviceDetails(const string &mqttUserName);
  char *formConnectionString(const char *username);
  void onMessageReceived(char *topic, uint8_t *payload, unsigned int length);
  void addConnectionParameter(char *connectionParamKey, char *connectionParamValue);
  MQTT_CALLBACK_SIGNATURE;
  inline bool checkStringIsValid(const char *value)
  {
    return (value == NULL || strcmp(value, "") == 0) ? false : true;
  }

public:
  typedef enum
  {
    SUCCESS = 0,
    FAILURE = -1,
    CONNECTION_ERROR = -3,
    CLIENT_ERROR = -2
  } transactionStatus;

  typedef enum
  {
    SUCCESFULLY_EXECUTED = 1001,
    EXECUTION_FAILURE = 4000,
    METHOD_NOT_FOUND = 4001,
    EXECUTING_PREVIOUS_COMMAND = 4002,
    INSUFFICIENT_INPUTS = 4003,
    DEVICE_CONNECTIVITY_ISSUES = 4004,
    PARTIAL_EXECUTION = 4005,
    ALREADY_ON_SAME_STATE = 4006
  } commandAckResponseCodes;

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
  int8_t init(const char *mqttUserName, const char *mqttPassword);
  int8_t connect();
  int8_t setMaxPayloadSize(int size);
  int8_t publish(const char *message);
  int8_t dispatch();
  int8_t dispatchEventFromJSONString(const char *eventType, const char *eventDescription, char *eventDataJSONString, const char *assetName);
  int8_t dispatchEventFromEventDataObject(const char *eventType, const char *eventDescription, const char *assetName);
  int8_t publishCommandAck(const char *correlation_id, commandAckResponseCodes status_code, const char *responseMessage);
  int8_t subscribe(MQTT_CALLBACK_SIGNATURE);
  int8_t get_command_topic(std::string& command_topic_string);
  int8_t reconnect();
  int8_t disconnect();
  inline bool isConnected()
  {
    bool status = _mqtt_client->connected();
    return status;
  }
  inline void zyield()
  {
    _mqtt_client->loop();
  }

  inline bool addEventDataPointNumber(const char *key, double value)
  {
    return addEventDataPoint(key, value);
  }
  inline bool addEventDataPointString(const char *key, const char *value)
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