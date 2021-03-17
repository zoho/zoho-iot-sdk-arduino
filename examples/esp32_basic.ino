#include <WiFi.h>
#include <zoho-iot-client.h>

#define ssid "Wifi_ssid"
#define password "Wifi_password"

#define MQTT_USERNAME (char *)"/mqtt_domain_name/v1/devices/client_id/connect"
#define MQTT_PASSWORD (char *)"mqtt_password"
#define CLIENT_ID (char *)"client_id"

WiFiClientSecure espClient;
ZohoIOTClient zc(&espClient, false);
const long interval = 1000;
ZohoIOTClient::commandAckResponseCodes success_response_code = ZohoIOTClient::SUCCESFULLY_EXECUTED;

char *comm_topic = strcat(strcat("/devices/", CLIENT_ID), "/commands");
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
    if (strcmp(topic, comm_topic) == 0)
    {
        DynamicJsonBuffer on_msg_buff;
        JsonArray &commandMessageArray = on_msg_buff.parseArray(msg);
        int msglength = commandMessageArray.measureLength();
        char response_msg[] = "Successfully completed the operation";
        for (int itr = 0; itr < msglength; itr++)
        {
            JsonObject &commandMessageObj = commandMessageArray.get<JsonObject>(itr);
            const char *correlation_id = commandMessageObj.get<const char *>("correlation_id");
            zc.publishCommandAck(correlation_id, success_response_code, response_msg);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Booting Up!");
    setup_wifi();
    zc.init(MQTT_USERNAME, MQTT_PASSWORD);
    zc.connect();
    zc.subscribe(on_message);
    Serial.println("Ready!");
}

unsigned long prev_time = 0, current_time = 0;
void loop()
{
    //Watchdog for Wifi & MQTT connection status.
    //Automatically reconnect in case of connection failure.
    setup_wifi();
    zc.reconnect();
    if ((current_time = millis()) - prev_time >= interval)
    {
        if (zc.isConnected())
        {
            prev_time = current_time;
            zc.addDataPointNumber("voltage", rand() / 100);
            zc.addDataPointNumber("current", rand() / 300);
            Serial.print("dispatch:");
            Serial.println(zc.dispatch());
        }
        else
        {
            Serial.println("Persist polled datapoints");
            // Write your Own persistance logic to store and publish data.
        }
    }
    zc.zyield();
}