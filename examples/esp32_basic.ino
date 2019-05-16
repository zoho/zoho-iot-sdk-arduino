#include <WiFi.h>
#include <zoho-iot-client.h>

#define ssid "Zoho-Guest"
#define password ""

#define DEVICE_ID (char *)"dev13mar_2"
#define DEVICE_TOKEN (char *)"secret"

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
    zc.init(DEVICE_ID, DEVICE_TOKEN);
    Serial.println("Ready!");
}

void loop()
{
    setup_wifi();
    zc.connect();
    zc.addDataPointNumber("temp", rand());
    zc.dispatch();
    delay(2000);
}