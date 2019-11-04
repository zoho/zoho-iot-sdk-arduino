#include "zoho-iot-client.h"
#include <sstream>
unsigned int retryCount = 0;

void ZohoIOTClient::formMqttTopics(char *client_id)
{
    //TODO: To find alternative for new operator for string concatenation.
    int publish_topic_size = strlen(topic_prefix) + strlen(client_id) + strlen(telemetry) + 1;
    int command_topic_size = strlen(topic_prefix) + strlen(client_id) + strlen(command) + 1;
    _publish_topic = new char[publish_topic_size];
    _command_topic = new char[command_topic_size];
    snprintf(_publish_topic, publish_topic_size, "%s%s%s", topic_prefix, client_id, telemetry);
    snprintf(_command_topic, command_topic_size, "%s%s%s", topic_prefix, client_id, command);
}

bool ZohoIOTClient::extractMqttServerAndDeviceDetails(const string &mqttUserName)
{
    vector<string> mqttUserNameVector;
    stringstream stringStream(mqttUserName);
    string item;

    while (getline(stringStream, item, '/'))
    {
        mqttUserNameVector.push_back(item);
    }

    if (mqttUserNameVector.size() != 6 || mqttUserNameVector[1].empty() || mqttUserNameVector[4].empty())
    {
        return false;
    }

    _mqtt_server = strdup(mqttUserNameVector[1].c_str());
    _client_id = strdup(mqttUserNameVector[4].c_str());
    return true;
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    //Serial.println("Publish received.");
    //Serial.print("  topic: ");
    //Serial.println(topic);
    //Serial.print(" Payload: ");
    //Serial.println(payload);
}

void onMqttPublish(uint16_t packetId)
{
    //Serial.println("Publish acknowledged.");
    //Serial.print("  packetId: ");
    //Serial.println(packetId);
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
    // Serial.println("Subscribe acknowledged.");
    // Serial.print("  packetId: ");
    // Serial.println(packetId);
    // Serial.print("  qos: ");
    //Serial.println(qos);
}

int ZohoIOTClient::init(char *mqttUserName, char *mqttPassword)
{
    if (mqttUserName == NULL || mqttPassword == NULL)
    {
        return FAILURE;
    }
    //TODO: Empty validation
    //TODO: unsubscribe old subscriptions.
    _mqtt_user_name = mqttUserName;
    _mqtt_password = mqttPassword;

    if (!extractMqttServerAndDeviceDetails(mqttUserName))
    {
        return FAILURE;
    }
    _mqtt_client->setServer(_mqtt_server, _port);
    _mqtt_client->setClientId(_client_id);
    _mqtt_client->setCredentials(_mqtt_user_name, _mqtt_password);
    _mqtt_client->onMessage(onMqttMessage);
    _mqtt_client->onPublish(onMqttPublish);
    _mqtt_client->onSubscribe(onMqttSubscribe);
    formMqttTopics(_client_id);
    return SUCCESS;
}

int ZohoIOTClient::publish(char *message)
{
    if (_mqtt_client->connected() != true || message == NULL)
    {
        return FAILURE;
    }
    string msg = message;
    if (_mqtt_client->publish(_publish_topic, 1, false, msg.c_str(), msg.length(), false, 1) != 0)
    {
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}

int ZohoIOTClient::dispatch()
{
    //Form json payload and publish to HUB...
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["device_id"] = _client_id;
    JsonObject &dataObj = root.createNestedObject("data");

    std::map<std::string, data>::iterator it = dataPointsMap.begin();
    while (it != dataPointsMap.end())
    {
        data value = it->second;
        switch (value.type)
        {
        case TYPE_INT:
        {
            dataObj[it->first.c_str()] = value.val.i_val;
            break;
        }
        case TYPE_DOUBLE:
        {
            dataObj[it->first.c_str()] = value.val.d_val;
            break;
        }
        case TYPE_CHAR:
        {
            dataObj[it->first.c_str()] = value.val.s_val;
            break;
        }
        default:
        {
            return FAILURE;
        }
        }
        it++;
    }
    dataPointsMap.clear();
    int size = root.measureLength() + 1;
    char payloadMsg[size];
    root.printTo(payloadMsg, size);
    //TODO: remove below debug message(payload message).
    //Serial.print("Payload message : ");
    //Serial.println(payloadMsg);

    return publish(payloadMsg);
}

int ZohoIOTClient::connect()
{
    //TODO: Empty validation
    //TODO: resubscribe old subscriptions if reconnecting.
    retryCount = 0;
    if (_mqtt_client->connected())
    {
        //Already having an active connecting with HUB... No job to do here..
        return SUCCESS;
    }
    // Serial.println("Connecting..");
    while (!_mqtt_client->connected())
    {
        _mqtt_client->connect();
        if (retryCount > _retry_limit)
        {
            break;
        }
        if (_mqtt_client->connected())
        {
            retryCount = 0;
            return SUCCESS;
        }
        else
        {
            delay(5000);
        }
        retryCount++;
    }
    return CONNECTION_ERROR;
}

int ZohoIOTClient::subscribe(char *topic)
{
    if (topic == NULL || !_mqtt_client->connected())
    {
        return FAILURE;
    }
    uint8_t qos = 0;
    if (_mqtt_client->subscribe(topic, qos) == 0)
    {
        return FAILURE;
    }
    return SUCCESS;
}

int ZohoIOTClient::disconnect()
{
    if (_mqtt_client->connected())
    {
        _mqtt_client->disconnect();
        if (_mqtt_client->connected())
        {
            return FAILURE;
        }
    }
    return SUCCESS;
}