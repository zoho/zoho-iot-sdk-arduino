#include "zoho-iot-client.h"
char connectionStringBuff[256] = "";
void ZohoIOTClient::formMqttPublishTopic(char *client_id)
{
    //TODO: To find alternative for new operator for string concatenation.
    int publish_topic_size = strlen(topic_prefix) + strlen(client_id) + strlen(telemetry) + 1;
    int event_topic_size = strlen(topic_prefix) + strlen(client_id) + strlen(event) + 1;
    _publish_topic = new char[publish_topic_size];
    _event_topic = new char[event_topic_size];
    snprintf(_publish_topic, publish_topic_size, "%s%s%s", topic_prefix, client_id, telemetry);
    snprintf(_event_topic, event_topic_size, "%s%s%s", topic_prefix, client_id, event);
}

bool ZohoIOTClient::extractMqttServerAndDeviceDetails(const string &mqttUserName)
{
    vector<string> mqttUserNameVector;
    string str = mqttUserName;
    char *token = strtok(const_cast<char *>(str.c_str()), "/");
    while (token != nullptr)
    {
        mqttUserNameVector.push_back(std::string(token));
        token = strtok(nullptr, "/");
    }

    if (mqttUserNameVector.size() != 5 || mqttUserNameVector[1].empty() || mqttUserNameVector[4].empty())
    {
        return false;
    }
    _mqtt_server = strdup(mqttUserNameVector[0].c_str());
    _client_id = strdup(mqttUserNameVector[3].c_str());
    return true;
}

int ZohoIOTClient::init(char *mqttUserName, char *mqttPassword)
{
    if (mqttUserName == NULL || mqttPassword == NULL)
    {
        return FAILURE;
    }
    //TODO: Empty validation
    //TODO: unsubscribe old subscriptions.
    _mqtt_user_name = mqttUserName;
    _mqtt_password = mqttPassword;

    if (!extractMqttServerAndDeviceDetails(mqttUserName))
    {
        return FAILURE;
    }
    _mqtt_client->setServer(_mqtt_server, _port);
    formMqttPublishTopic(_client_id);
    currentState = INITIALIZED;
    return SUCCESS;
}

int ZohoIOTClient::dispatchEventFromEventDataObject(char *eventType, char *eventDescription, char *assetName)
{
    int size = eventDataObject.measureLength() + 1;
    char eventDataJSONString[size];
    eventDataObject.printTo(eventDataJSONString, size);
    return dispatchEventFromJSONString(eventType, eventDescription, eventDataJSONString, assetName);
}

int ZohoIOTClient::dispatchEventFromJSONString(char *eventType, char *eventDescription, char *eventDataJSONString, char *assetName)
{
    if (currentState != CONNECTED)
    {
        return CONNECTION_ERROR;
    }

    if (!_mqtt_client->connected())
    {
        currentState = CONNECTION_LOST;
        return FAILURE;
    }

    if (!checkStringIsValid(eventType)|| eventDescription == NULL || !checkStringIsValid(eventDataJSONString))
    {
        // Serial.println("Event Type/Description or dataJsonString cant be Null or empty");
        return FAILURE;
    }

    DynamicJsonBuffer event_dispatch_buffer, event_data_buffer;
    JsonObject &eventObject = event_dispatch_buffer.createObject();
    eventObject["event_type"] = eventType;
    eventObject["event_descr"] = eventDescription;
    JsonObject &parsed_event_Data = event_data_buffer.parseObject(eventDataJSONString);
    if (!parsed_event_Data.success())
    {
        return FAILURE;
    }
    eventObject["event_data"] = parsed_event_Data;
    int size;
    bool pub_status = false;
    if (!checkStringIsValid(assetName))
    {
        size = eventObject.measureLength() + 1;
        char payloadMsg[size];
        eventObject.printTo(payloadMsg, size);
        pub_status = _mqtt_client->publish(_event_topic, payloadMsg);
        eventObject.remove("event_type");
        eventObject.remove("event_data");
        eventObject.remove("event_descr");
        event_data_buffer.clear();
    }
    else
    {
        JsonObject &eventDisptachObject = event_dispatch_buffer.createObject();
        eventDisptachObject[assetName] = eventObject;
        size = eventDisptachObject.measureLength() + 1;
        char payloadMsg[size];
        eventDisptachObject.printTo(payloadMsg, size);
        pub_status = _mqtt_client->publish(_event_topic, payloadMsg);
        eventDisptachObject.remove(assetName);
        event_dispatch_buffer.clear();
    }
    if (pub_status == true)
    {
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}

int ZohoIOTClient::publish(char *message)
{
    if (currentState != CONNECTED)
    {
        return CONNECTION_ERROR;
    }
    if (message == NULL)
    {
        return FAILURE;
    }
    if (!_mqtt_client->connected())
    {
        currentState = CONNECTION_LOST;
        return FAILURE;
    }
    //TODO: Empty validation
    if (_mqtt_client->publish(_publish_topic, message) == true)
    {
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}

int ZohoIOTClient::dispatch()
{
    //Form json payload and publish to HUB...
    if (currentState != CONNECTED)
    {
        return CONNECTION_ERROR;
    }
    int size = root.measureLength() + 1;
    char payloadMsg[size];
    root.printTo(payloadMsg, size);
    //TODO: remove below debug message(payload message).
    // Serial.print("Payload message : ");
    // Serial.println(payloadMsg);

    return publish(payloadMsg);
}
void ZohoIOTClient::addConnectionParameter(char *connectionParamKey, char *connectionParamValue)
{
    sprintf(connectionStringBuff, "%s%s%s%s%s", connectionStringBuff, connectionParamKey, "=", connectionParamValue, "&");
}

char *ZohoIOTClient::formConnectionString(char *username)
{
    sprintf(connectionStringBuff, "%s%s", username, "?");
    addConnectionParameter((char *)"sdk_name", sdk_name);
    addConnectionParameter((char *)"sdk_version", sdk_version);
    // addConnectionParameter("sdk_url", sdk_url);
    connectionStringBuff[strlen(connectionStringBuff) - 1] = '\0';
    return connectionStringBuff;
}

int ZohoIOTClient::connect()
{
    //TODO: Empty validation
    //TODO: resubscribe old subscriptions if reconnecting.
    if (currentState < INITIALIZED)
    {
        return CONNECTION_ERROR;
    }
    // bool connectionState = _mqtt_client->connected();
    if (_mqtt_client->connected())
    {
        //Already having an active connecting with HUB... No job to do here..
        currentState = CONNECTED;
        return SUCCESS;
    }
    // Serial.println("Connecting..");
    while (!_mqtt_client->connected())
    {
        _mqtt_client->connect(_client_id, formConnectionString(_mqtt_user_name), _mqtt_password);
        if (retryCount > _retry_limit)
        {
            currentState = RETRYING;
            break;
        }
        if (_mqtt_client->connected())
        {
            retryCount = 0;
            currentState = CONNECTED;
            return SUCCESS;
        }
        else
        {
            //Serial.print("RetryCount: ");
            // Serial.println(retryCount);
            // Serial.print("Failed. rc:");
            // Serial.print(_mqtt_client.state());
            // Serial.println(" Retry in 5 seconds");
            delay(5000);
            currentState = RETRYING;
            retryCount++;
        }
    }
    return CONNECTION_ERROR;
}

int ZohoIOTClient::subscribe(char *topic, MQTT_CALLBACK_SIGNATURE)
{
    //Subscribe to topic and set method to be called message on that topic.
    //TODO: Empty validation
    if (currentState != CONNECTED)
    {
        return CONNECTION_ERROR;
    }

    if (topic == NULL)
    {
        return FAILURE;
    }
    if (!_mqtt_client->connected())
    {
        currentState = CONNECTION_LOST;
        return FAILURE;
    }
    _mqtt_client->setCallback(callback);
    if (!_mqtt_client->subscribe(topic))
    {
        return FAILURE;
    }
    return SUCCESS;
}

int ZohoIOTClient::disconnect()
{
    if (_mqtt_client->connected())
    {
        _mqtt_client->disconnect();
        if (_mqtt_client->connected())
        {
            return FAILURE;
        }
    }
    return SUCCESS;
}