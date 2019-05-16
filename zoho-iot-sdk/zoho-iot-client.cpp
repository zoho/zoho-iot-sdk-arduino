#include "zoho-iot-client.h"
unsigned int retryCount = 0;

void ZohoIOTClient::formTopics(char *device_id)
{
    //TODO: To find alternative for new operator for string concatenation.
    int topic_size = strlen(topic_prefix) + strlen(device_id) + strlen(telemetry);
    _publish_topic = new char[topic_size+1];
    snprintf(_publish_topic, topic_size,"%s%s%s", topic_prefix, device_id, telemetry);
}

int ZohoIOTClient::init(char *device_id, char *device_token)
{
    if (device_id == NULL || device_token == NULL)
    {
        return FAILURE;
    }
    //TODO: Empty validation
    //TODO: unsubscribe old subscriptions.
    _device_id = device_id;
    _device_token = device_token;
    _mqtt_client->setServer(_mqtt_server, _port);
    _mqtt_client->setClientId(_device_id);
    _mqtt_client->setCredentials(_device_id, _device_token);
    formTopics(device_id);
    return SUCCESS;
}

int ZohoIOTClient::publish(char *message)
{
    if (_mqtt_client->connected() != true || message == NULL)
    {
        return FAILURE;
    }
    string msg = message;
    if (_mqtt_client->publish(_publish_topic, 1, false, msg.c_str(), msg.length(), false, 1) != 0)
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
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["device_id"] = _device_id;
    JsonObject &dataObj = root.createNestedObject("data");

    std::map<std::string, data>::iterator it = dataPointsMap.begin();
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
    //Serial.print("Payload message : ");
    //Serial.println(payloadMsg);

    return publish(payloadMsg);
}

int ZohoIOTClient::connect()
{
    //TODO: Empty validation
    //TODO: resubscribe old subscriptions if reconnecting.
    retryCount = 0;
    if (_mqtt_client->connected())
    {
        //Already having an active connecting with HUB... No job to do here..
        return SUCCESS;
    }
    // Serial.println("Connecting..");
    while (!_mqtt_client->connected())
    {
        _mqtt_client->connect();
        if (retryCount > _retry_limit)
        {
            break;
        }
        if (_mqtt_client->connected())
        {
            retryCount = 0;
            return SUCCESS;
        }
        else
        {
            delay(5000);
        }
        retryCount++;
    }
    return CONNECTION_ERROR;
}

int ZohoIOTClient::subscribe(char *topic)
{
    if (topic == NULL || !_mqtt_client->connected())
    {
        return FAILURE;
    }
    uint8_t qos = 0;
    if (_mqtt_client->subscribe(topic, qos) == 0)
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