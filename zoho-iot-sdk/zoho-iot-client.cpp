#include "zoho-iot-client.h"

unsigned int retryCount = 0;
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
    return SUCCESS;
}

int ZohoIOTClient::publish(char *message)
{
    if (_mqtt_client->connected() != true || message == NULL)
    {
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
    // Serial.print("Payload message : ");
    // Serial.println(payloadMsg);

    return publish(payloadMsg);
}

int ZohoIOTClient::subscribe(char *topic, MQTT_CALLBACK_SIGNATURE)
{
    //Subscribe to topic and set method to be called message on that topic.
    //TODO: Empty validation
    if (topic == NULL)
    {
        return FAILURE;
    }
    _mqtt_client->setCallback(callback);
    if (_mqtt_client->subscribe(topic) == true)
    {
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}

int ZohoIOTClient::connect()
{
    //TODO: Empty validation
    //TODO: resubscribe old subscriptions if reconnecting.

    if (_mqtt_client->connected())
    {
        //Already having an active connecting with HUB... No job to do here..
        return SUCCESS;
    }

    // Serial.println("Initiating connection with HUB...!");

    while (!_mqtt_client->connected())
    {
        // Serial.println("Connecting..");
        if (retryCount > _retry_limit)
        {
            // Serial.println("retry limit exceeded");
            return CONNECTION_ERROR;
        }

        if (_mqtt_client->connect(_device_id, _device_id, _device_token))
        {
            // Serial.println("Successfully Connected!");
            retryCount = 0;
            if (_mqtt_client->publish("hello", "Connected!") == true)
                return SUCCESS;
            else
                return FAILURE;
        }
        else
        {
            // Serial.print("RetryCount: ");
            // Serial.println(retryCount);
            // Serial.print("Failed. rc:");
            // Serial.print(_mqtt_client.state());
            // Serial.println(" Retry in 5 seconds");
            delay(5000);
        }
        retryCount++;
    }
    return SUCCESS;
}