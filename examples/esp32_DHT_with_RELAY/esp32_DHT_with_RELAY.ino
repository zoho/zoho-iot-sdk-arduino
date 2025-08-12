#include <WiFi.h>
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <zoho-iot-client.h>
#include "certificate.h"

#define SSID "Wifi_ssid"
#define PASSWORD "Wifi_password"

#define MQTT_USERNAME (char *)"/mqtt_domain_name/v1/devices/client_id/connect"
#define MQTT_PASSWORD (char *)"mqtt_password"

#include "DHT.h"

#define DHT_PIN 13
#define RELAY_PIN 12
#define COMMAND_KEY "ac_control"
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

#define TEMPERATURE_IN_FAHRENHEIT false
#define RELAY_ACTIVE_LOW true

WiFiClientSecure espClient;
ZohoIOTClient zClient(&espClient, true);
const long interval = 10000;

ZohoIOTClient::commandAckResponseCodes success_response_code = ZohoIOTClient::SUCCESSFULLY_EXECUTED;
ZohoIOTClient::commandAckResponseCodes failure_response_code = ZohoIOTClient::EXECUTION_FAILURE;
unsigned long prev_time = 0, current_time = 0;

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
    JsonDocument doc;
    deserializeJson(doc, msg);
    int msglength = doc.size();
    char response_msg[] = "Successfully completed the operation";
    for (int itr = 0; itr < msglength; itr++) {
      JsonObject commandMessageObj = doc[itr];
      const char *correlation_id = commandMessageObj["correlation_id"];
      JsonArray payloadArray = commandMessageObj["payload"].as<JsonArray>();
      const char *command_key = payloadArray[0]["edge_command_key"];
      const char *value = payloadArray[0]["value"];
      if (strcmp(command_key, COMMAND_KEY) == 0) {
        if (strcmp(value, "on") == 0) {
          // Turn on the relay
          Serial.println("Turning on the relay");
          turn_on_relay();
          zClient.publishCommandAck(correlation_id, success_response_code, "AC Turned ON");
        } else if (strcmp(value, "off") == 0) {
          // Turn off the relay
          Serial.println("Turning off the relay");
          turn_off_relay();
          zClient.publishCommandAck(correlation_id, success_response_code, "AC Turned OFF");
        } else {
          Serial.println("Unknown value for edge_command_key");
          zClient.publishCommandAck(correlation_id, failure_response_code, "Unknown value for edge_command_key");
        }
      } else {
        Serial.println("Unknown command, command not handled");
        zClient.publishCommandAck(correlation_id, failure_response_code, "Unknown command");
      }
    }
  }
}

void turn_on_relay(){
  if(RELAY_ACTIVE_LOW){
    digitalWrite(RELAY_PIN, LOW);
  }
  else{
    digitalWrite(RELAY_PIN, HIGH);
  }
}

void turn_off_relay(){
   if(RELAY_ACTIVE_LOW){
    digitalWrite(RELAY_PIN, HIGH);
  }
  else{
    digitalWrite(RELAY_PIN, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting Up!");
  pinMode(RELAY_PIN, OUTPUT);
  turn_off_relay();
  setup_wifi();
  espClient.setCACert(root_ca);
  zClient.init(MQTT_USERNAME, MQTT_PASSWORD);
  zClient.connect();
  zClient.subscribe(on_message);
  Serial.println("Ready!");
  dht.begin();
}

void loop() {
  //Watchdog for Wifi & MQTT connection status.
  //Automatically reconnect in case of connection failure.
  setup_wifi();
  zClient.reconnect();
  if ((current_time = millis()) - prev_time >= interval) {
    if (zClient.isConnected()) {
      float humidity = dht.readHumidity(true);
      // Read temperature as Celsius (the default)
      float temperature = dht.readTemperature(TEMPERATURE_IN_FAHRENHEIT);
      // Check if any reads failed and exit early (to try again).
      if (isnan(humidity) || isnan(temperature)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
      }
      prev_time = current_time;
      //Add datapoints
      zClient.addDataPointNumber("humidity", humidity);
      zClient.addDataPointNumber("temperature", temperature);
      String payload = zClient.getPayload().c_str();
      Serial.println("dispatching message: " + payload);
      //Publish payload
      if (zClient.dispatch() == zClient.SUCCESS) {
        Serial.println("Message published successfully");
      }
    }
  }
  zClient.zyield();
}
