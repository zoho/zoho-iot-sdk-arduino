#include <WiFi.h>
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <zoho-iot-client.h>
#include "certificate.h"

#define SSID "Wifi_ssid"
#define PASSWORD "Wifi_password"

#define MQTT_USERNAME (char *)"/mqtt_domain_name/v1/devices/client_id/connect"
#define MQTT_PASSWORD (char *)"mqtt_password"

WiFiClientSecure espClient;
ZohoIOTClient zClient(&espClient, true);
const long interval = 10000;
ZohoIOTClient::commandAckResponseCodes success_response_code = ZohoIOTClient::SUCCESFULLY_EXECUTED;
unsigned long prev_time = 0, current_time = 0;

void on_message(char *topic, uint8_t *payload, unsigned int length) {
  Serial.println("new message recieved");
  String msg = "";
  for (unsigned int itr = 0; itr < length; itr++) {
    msg += (char)payload[itr];
  }
  Serial.print("[ ");
  Serial.print(topic);
  Serial.print(" ] : ");
  Serial.print(msg);
  Serial.println();
  string command_topic_string;
  zClient.get_command_topic(command_topic_string);
  const char *command_topic = command_topic_string.c_str();

  if (strcmp(topic, command_topic) == 0) {
    JsonDocument commandMessageArray;
    deserializeJson(commandMessageArray, msg);
    int msglength = commandMessageArray.size();
    char response_msg[] = "Successfully completed the operation";
    for (int itr = 0; itr < msglength; itr++) {
      JsonObject commandMessageObj = commandMessageArray[itr];
      const char *correlation_id = commandMessageObj["correlation_id"];
      zClient.publishCommandAck(correlation_id, success_response_code, response_msg);
    }
  }
}

void setup_wifi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  delay(10);
  //Connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  delay(10);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting Up!");
  setup_wifi();
  espClient.setCACert(root_ca);
  zClient.init(MQTT_USERNAME, MQTT_PASSWORD);
  zClient.connect();
  zClient.subscribe(on_message);
  Serial.println("Ready!");
}

void loop() {
  //Watchdog for Wifi & MQTT connection status.
  //Automatically reconnect in case of connection failure.
  setup_wifi();
  zClient.reconnect();
  if ((current_time = millis()) - prev_time >= interval) {
    if (zClient.isConnected()) {
      prev_time = current_time;
      zClient.addDataPointNumber("voltage", random(200, 250));  // Generates a random number between 200 and 250
      zClient.addDataPointNumber("current", random(1, 10));     // Generates a random number between 1 and 10
      String payload = zClient.getPayload().c_str();
      Serial.println("dispatching message: " + payload);
      int rc = zClient.dispatch();
      if (rc == zClient.SUCCESS) {
        Serial.println("Message published successfully");
      } else {
        Serial.println("Failed to published message");
      }
    }
  }
  zClient.zyield();
}
