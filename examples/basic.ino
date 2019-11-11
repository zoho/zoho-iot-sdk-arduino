#include <ESP8266WiFi.h>
#include <zoho-iot-client.h>

#define ssid "Zoho-Guest"
#define password ""

#define MQTT_USERNAME (char *)"/domain_name/v1/devices/client_id/connect"
#define MQTT_PASSWORD (char *)"device_token"

WiFiClient espClient;
ZohoIOTClient zc;
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

void setup()
{
    Serial.begin(115200);
    Serial.println("Booting Up!");
    delay(5000);
    setup_wifi();
    delay(5000);
    zc.init(MQTT_USERNAME, MQTT_PASSWORD);
    zc.connect();
    Serial.println("Ready!");
}

void loop()
{
    setup_wifi();
    Serial.printf("Connection status ");
    Serial.println(zc.connect());
    zc.addDataPointNumber("voltage", rand()/100);
    zc.addDataPointNumber("current", rand()/300);
    zc.dispatch();
    delay(2000);
}