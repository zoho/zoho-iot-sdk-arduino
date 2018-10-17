#include "zoho-iot-client.h"
// #include "IPAddress.h"
// #include "Client.h"
// #include "Stream.h"

void ZohoIOTClient::init(char *device_id, char *device_token)
{
    //TODO: Empty validation
    //TODO: unsubscribe old subscriptions.
    _device_id = device_id;
    _device_token = device_token;
    _mqtt_client.setServer(_mqtt_server, _port);
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