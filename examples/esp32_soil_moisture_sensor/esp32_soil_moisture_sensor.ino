#include <WiFi.h>
#include <zoho-iot-client.h>

#define SSID "Wifi_ssid"
#define PASSWORD "Wifi_password"

#define MQTT_USERNAME (char *)"/mqtt_domain_name/v1/devices/client_id/connect"
#define MQTT_PASSWORD (char *)"mqtt_password"

const int soilMoistureSensorPin = 34;
const int RelayPin = 26;
WiFiClient espClient;
ZohoIOTClient zClient(&espClient, false);
const long interval = 10000;
int numberOfSamples = 10;
ZohoIOTClient::commandAckResponseCodes success_response_code = ZohoIOTClient::SUCCESFULLY_EXECUTED;
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
    JsonDocument doc;
    deserializeJson(doc, msg);
    int msglength = doc.size();
    char response_msg[] = "Successfully completed the operation";
    for (int itr = 0; itr < msglength; itr++) {
      JsonObject commandMessageObj = doc[itr];
      const char *correlation_id = commandMessageObj["correlation_id"];
      const char *command_name = commandMessageObj["command_name"];
      if(strcmp(command_name,"pump_controll") == 0){
        JsonArray payloadArray = commandMessageObj["payload"].as<JsonArray>();
        const char *value = payloadArray[0]["value"];
        if (strcmp(value, "on") == 0) {
            // Turn on the relay
            Serial.println("Turning on the relay");
            digitalWrite(RelayPin, LOW); 
            zClient.publishCommandAck(correlation_id, success_response_code, "Pump Turned ON");
        } else if (strcmp(value, "off") == 0) {
            // Turn off the relay
            Serial.println("Turning off the relay");
            digitalWrite(RelayPin, HIGH); 
            zClient.publishCommandAck(correlation_id, success_response_code, "Pump Turned OFF");
        } else {
            Serial.println("Unknown value for edge_command_key");
            zClient.publishCommandAck(correlation_id, failure_response_code, "Unknown value for edge_command_key");
        }
      }
      else{
         zClient.publishCommandAck(correlation_id, failure_response_code, "Unknown command");
      }
      
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting Up!");
  pinMode(RelayPin,OUTPUT);
  digitalWrite(RelayPin,HIGH);
  setup_wifi();
  zClient.init(MQTT_USERNAME, MQTT_PASSWORD);
  zClient.connect();
  zClient.subscribe(on_message);
  Serial.println("Ready!");
}

void loop() {
  setup_wifi();
  zClient.reconnect();
  if ((current_time = millis()) - prev_time >= interval) {
    if (zClient.isConnected()) {
      prev_time = current_time;
      int sensor_analog = 0;
      Serial.println("Reading from soil moisture sensor");
      for(int i = 0; i < numberOfSamples ; i++){
        sensor_analog = sensor_analog + analogRead(soilMoistureSensorPin);
        delay(500);
        Serial.print(".");
      }
      Serial.println();
      sensor_analog = sensor_analog/numberOfSamples;
      //convert raw sensor value to percentage
      int moisture = ( 100 - ( (sensor_analog/4095.00) * 100 ) );
      zClient.addDataPointNumber("moisture", moisture);
      String payload = zClient.getPayload().c_str();
      Serial.println("dispatching message: " + payload);
      if (zClient.dispatch() == zClient.SUCCESS) {
        Serial.println("Message published successfully");
      }
    }
  }
  zClient.zyield();
}