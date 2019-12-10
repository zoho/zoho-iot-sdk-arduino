#include "zoho-iot-client.h"

void ZohoIOTClient::formMqttPublishTopic(char *client_id)
{
    //TODO: To find alternative for new operator for string concatenation.
    int publish_topic_size = strlen(topic_prefix) + strlen(client_id) + strlen(telemetry) + 1;
    _publish_topic = new char[publish_topic_size];
    snprintf(_publish_topic, publish_topic_size, "%s%s%s", topic_prefix, client_id, telemetry);
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
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["device_id"] = _client_id;
    JsonObject &dataObj = root.createNestedObject("data");

    std::map<string, data>::iterator it = dataPointsMap.begin();
    while (it != dataPointsMap.end())
    {
        data value = it->second;
        switch (value.type)
        {
        case TYPE_INT:
        {
            dataObj[it->first.c_str()] = value.val.i_val;
            break;
        }
        case TYPE_DOUBLE:
        {
            dataObj[it->first.c_str()] = value.val.d_val;
            break;
        }
        case TYPE_CHAR:
        {
            dataObj[it->first.c_str()] = value.val.s_val;
            break;
        }
        default:
        {
            return FAILURE;
        }
        }
        it++;
    }
    dataPointsMap.clear();
    int size = root.measureLength() + 1;
    char payloadMsg[size];
    root.printTo(payloadMsg, size);
    //TODO: remove below debug message(payload message).
    // Serial.print("Payload message : ");
    // Serial.println(payloadMsg);

    return publish(payloadMsg);
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
        _mqtt_client->connect(_client_id, _mqtt_user_name, _mqtt_password);
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