#include <ESP8266WiFi.h>
#include <zoho-iot-client.h>

#define ssid "Wifi_ssid"
#define password "Wifi_password"

WiFiClientSecure espClient;
ZohoIOTClient zc(&espClient, true);
const char fingerPrint[] = "AA:BB:CC:DD:EE:FF:00:11:22:33:44:55:66:77:88:99:AA:BB:CC:DD";
#define MQTT_USERNAME (char *)"/mqtt_domain_name/v1/devices/client_id/connect"
#define MQTT_PASSWORD (char *)"mqtt_password"
const long interval = 1000;
void on_message(char *topic, byte *payload, unsigned int length)
{
    Serial.println("new message recieved");
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

void setup()
{
    Serial.begin(115200);
    Serial.println("Booting Up!");
    delay(5000);
    setup_wifi();
    espClient.setFingerprint(fingerPrint);
    zc.init(MQTT_USERNAME, MQTT_PASSWORD);
    zc.connect();
    char *sub_topic = strcat(strcat("/devices/", "client_id"), "/commands");
    zc.subscribe(sub_topic, on_message);
    Serial.println("Ready!");
}

unsigned long prev_time = 0, current_time = 0;
void loop()
{
    //Watchdog for Wifi & MQTT connection status.
    //Automatically reconnect in case of connection failure.
    setup_wifi();
    zc.connect();
    if ((current_time = millis()) - prev_time >= interval)
    {
        prev_time = current_time;
        zc.addDataPointNumber("voltage", rand() / 100);
        zc.addDataPointNumber("current", rand() / 300);
        Serial.print("dispatch:");
        Serial.println(zc.dispatch());
        Serial.println(millis());
    }
    zc.zyield();
}