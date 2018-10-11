#include "zoho-iot-client.h"
// #include "IPAddress.h"
// #include "Client.h"
// #include "Stream.h"

ZohoIOTClient::ZohoIOTClient()
{

    _mqtt_client = new PubSubClient(_wifi_client);
}

void ZohoIOTClient::setWifiParams(char *ssid, char *password)
{
    //TODO: Empty validation
    _wifi_ssid = ssid;
    _wifi_password = password;
}

void ZohoIOTClient::setHUBConnectionParams(char *device_id, char *device_token)
{
    //TODO: Empty validation
    _device_id = device_id;
    _device_token = device_token;
}

void ZohoIOTClient::connect_WIFI()
{
    delay(10);
    Serial.print("\n\n\n Connecting to ");
    Serial.println(_wifi_ssid);
    // WiFi.mode(WIFI_STA);
    WiFi.begin(_wifi_ssid, _wifi_password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }

    Serial.print("\nSuccessfully connected to ");
    Serial.println(_wifi_ssid);
    Serial.print("\nwith IP: ");
    Serial.println(WiFi.localIP());
}

void ZohoIOTClient::connect_IOT_HUB()
{
    //TODO: move this to constructor.
    (this->_mqtt_client)->setServer(_mqtt_server, _port);

    while (!(this->_mqtt_client)->connected())
    {
        Serial.println("Connecting to IOT HUB..");

        if ((this->_mqtt_client)->connect(_device_id, _device_id, _device_token))
        {
            Serial.println("Successfully Connected!");

            (this->_mqtt_client)->publish("hello", "Connected!");
        }
        else
        {
            Serial.print("Failed. rc:");
            Serial.print((this->_mqtt_client)->state());
            Serial.println(" Retry in 5 seconds");
            delay(5000);
        }
    }
}