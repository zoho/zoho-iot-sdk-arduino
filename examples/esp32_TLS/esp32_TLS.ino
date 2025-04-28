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
ZohoIOTClient::commandAckResponseCodes success_response_code = ZohoIOTClient::SUCCESSFULLY_EXECUTED;
unsigned long prev_time = 0, current_time = 0;

void on_message(char *topic, uint8_t *payload, unsigned int length) {
  Serial.println("new message received");
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
      // Simulate voltage between 210V and 240V
      float voltage1 = 210 + random(0, 3000) / 100.0;
      zClient.addDataPointNumber("l1_to_neutral_voltage", voltage1);
      float voltage2 = 210 + random(0, 3000) / 100.0;
      zClient.addDataPointNumber("l2_to_neutral_voltage", voltage2);
      float voltage3 = 210 + random(0, 3000) / 100.0;
      zClient.addDataPointNumber("l3_to_neutral_voltage", voltage3);
      float voltage = (voltage1+voltage2+voltage3) / 3.0;
      zClient.addDataPointNumber("line_to_neutral_voltage", voltage);

      // Simulate current between 0A and 10A
      float current1 = random(0, 1000) / 100.0;
      zClient.addDataPointNumber("l1_current", current1);
      float current2 = random(0, 1000) / 100.0;
      zClient.addDataPointNumber("l2_current", current2);
      float current3 = random(0, 1000) / 100.0;
      zClient.addDataPointNumber("l3_current", current3);
      float current = (current1+current2+current3) / 3.0;
      zClient.addDataPointNumber("average_current", current);

      // Simulate frequency between 49.5 Hz and 50.5 Hz (for 50 Hz mains)
      float frequency = 49.5 + random(0, 101) / 100.0;
      zClient.addDataPointNumber("frequency", frequency);
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
