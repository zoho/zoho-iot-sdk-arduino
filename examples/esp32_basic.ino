#include <WiFi.h>
#include <zoho-iot-client.h>

#define ssid "Zoho-Guest"
#define password ""

#define MQTT_USERNAME (char *)"/domain_name/v1/devices/client_id/connect"
#define MQTT_PASSWORD (char *)"mqtt_pwd"

WiFiClientSecure espClient;
ZohoIOTClient zc(&espClient,false);

void setup_wifi()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }

    delay(10);
    //Connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    WiFi.persistent(false);
    WiFi.begin(ssid, password);

    delay(10);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void on_message(char *topic, byte *payload, unsigned int length)
{
    String msg = "";
    for (unsigned int itr = 0; itr < length; itr++)
    {
        msg += (char)payload[itr];
    }
    Serial.print("[ ");
    Serial.print(topic);
    Serial.print(" ] : ");
    Serial.print(msg);
    Serial.println();
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Booting Up!");
    setup_wifi();
    zc.init(MQTT_USERNAME, MQTT_PASSWORD);
    zc.connect();
    char *sub_topic = strcat(strcat("/devices/", "client_id"), "/command");
    zc.subscribe(sub_topic,on_message);
    Serial.println("Ready!");
}

void loop()
{
    //Watchdog for Wifi & MQTT connection status.
    //Automatically reconnect in case of connection failure.
    setup_wifi();
    //Serial.printf("Connection status ");
    //Serial.println(zc.connect());
    zc.addDataPointNumber("temp", rand());
    //zc.addDataPointNumber("current", rand() / 300);
    //Serial.print("dispatch:");
    //Serial.println(zc.dispatch());
    zc.zyield();
    delay(1000);
}