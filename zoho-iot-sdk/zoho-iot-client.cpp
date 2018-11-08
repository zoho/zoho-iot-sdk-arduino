#include "zoho-iot-client.h"

void ZohoIOTClient::init(char *device_id, char *device_token)
{
    //TODO: Empty validation
    //TODO: unsubscribe old subscriptions.
    _device_id = device_id;
    _device_token = device_token;
    _mqtt_client.setServer(_mqtt_server, _port);
}

bool ZohoIOTClient::publish(char *message)
{
    //TODO: Empty validation
    return _mqtt_client.publish(_publish_topic, message);
}

bool ZohoIOTClient::dispatch()
{
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
            return false;
        }
        }
        it++;
    }
    dataPointsMap.clear();
    int size = root.measureLength() + 1;
    char payloadMsg[size];
    root.printTo(payloadMsg, size);

    Serial.print("Payload message : ");
    Serial.println(payloadMsg);

    return publish(payloadMsg);
}

bool ZohoIOTClient::connect()
{
    //TODO: Empty validation
    //TODO: resubscribe old subscriptions if reconnecting.

    if (_mqtt_client.connected())
    {
        //Already having an active connecting with HUB... No job to do here..
        return true;
    }

    Serial.println("Initiating connection with HUB...!");

    while (!_mqtt_client.connected())
    {
        Serial.println("Connecting..");

        if (_mqtt_client.connect(_device_id, _device_id, _device_token))
        {
            Serial.println("Successfully Connected!");

            return _mqtt_client.publish("hello", "Connected!");
        }
        else
        {
            Serial.print("Failed. rc:");
            Serial.print(_mqtt_client.state());
            Serial.println(" Retry in 5 seconds");
            delay(5000);
        }
    }
    return true;
}