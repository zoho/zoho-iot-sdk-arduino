#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <zoho-iot-client.h>

#define ssid "Zoho-Guest"
#define password ""

#define DEVICE_ID "dev13mar_2"
#define DEVICE_TOKEN "secret"

WiFiClient espClient;
ZohoIOTClient zc(espClient);

int count = 0;

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
    setup_wifi();
    zc.init(DEVICE_ID, DEVICE_TOKEN);
    zc.connect();

    Serial.println("Ready!");
}

void loop()
{
    //Watchdog for Wifi & MQTT connection status.
    //Automatically reconnect in case of connection failure.
    setup_wifi();
    zc.connect();

    count = count + 1;
    Serial.println(count);

    digitalWrite(2, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(1000);           // wait for a second
    digitalWrite(2, LOW);  // turn the LED off by making the voltage LOW
    delay(1000);
}