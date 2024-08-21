#include "zoho-iot-client.h"

char connectionStringBuff[256] = "";
unsigned long start_time = 0;
bool retried = false;
void ZohoIOTClient::formMqttPublishTopic(const char *client_id)
{
    //TODO: To find alternative for new operator for string concatenation.
    uint8_t topic_common_length = strlen(topic_prefix) + strlen(client_id) + 1;
    uint8_t publish_topic_size = topic_common_length + strlen(telemetry);
    uint8_t event_topic_size = topic_common_length + strlen(event);
    uint8_t command_topic_size = topic_common_length + strlen(command);
    uint8_t command_ack_topic_size = topic_common_length + strlen(commandAck);
    _publish_topic = new char[publish_topic_size];
    _event_topic = new char[event_topic_size];
    _command_topic = new char[command_topic_size];
    _command_ack_topic = new char[command_ack_topic_size];
    snprintf(_publish_topic, publish_topic_size, "%s%s%s", topic_prefix, client_id, telemetry);
    snprintf(_event_topic, event_topic_size, "%s%s%s", topic_prefix, client_id, event);
    snprintf(_command_topic, command_topic_size, "%s%s%s", topic_prefix, client_id, command);
    snprintf(_command_ack_topic, command_ack_topic_size, "%s%s%s", topic_prefix, client_id, commandAck);
}

bool ZohoIOTClient::extractMqttServerAndDeviceDetails(const string &mqttUserName)
{
    vector<string> mqttUserNameVector;
    string str = mqttUserName;
    char *token = strtok(const_cast<char *>(str.c_str()), "/");
    while (token != nullptr)
    {
        mqttUserNameVector.push_back(std::string(token));
        token = strtok(nullptr, "/");
    }

    if (mqttUserNameVector.size() != 5 || mqttUserNameVector[1].empty() || mqttUserNameVector[4].empty())
    {
        return false;
    }
    _mqtt_server = strdup(mqttUserNameVector[0].c_str());
    _client_id = strdup(mqttUserNameVector[3].c_str());
    return true;
}

int8_t ZohoIOTClient::init(const char *mqttUserName, const char *mqttPassword)
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
    formMqttPublishTopic(_client_id);
    currentState = INITIALIZED;
    return SUCCESS;
}

int8_t ZohoIOTClient::dispatchEventFromEventDataObject(const char *eventType, const char *eventDescription, const char *assetName)
{
    String jsonString;
    serializeJson(eventDataObject, jsonString);
    int size = jsonString.length()+1;
    char eventDataJSONString[size];
    jsonString.toCharArray(eventDataJSONString,size);
    return dispatchEventFromJSONString(eventType, eventDescription, eventDataJSONString, assetName);
}

int8_t ZohoIOTClient::dispatchEventFromJSONString(const char *eventType, const char *eventDescription, char *eventDataJSONString, const char *assetName)
{
    if (currentState != CONNECTED)
    {
        return CLIENT_ERROR;
    }

    if (!_mqtt_client->connected())
    {
        currentState = CONNECTION_LOST;
        return FAILURE;
    }

    if (!checkStringIsValid(eventType) || eventDescription == NULL || !checkStringIsValid(eventDataJSONString))
    {
        // Serial.println("Event Type/Description or dataJsonString cant be Null or empty");
        return FAILURE;
    }

    JsonDocument event_dispatch_doc;
    JsonDocument event_object_doc;
    JsonDocument event_data_doc;
    JsonObject eventObject = event_object_doc.to<JsonObject>();
    eventObject["event_type"] = eventType;
    eventObject["event_descr"] = eventDescription;
    DeserializationError error = deserializeJson(event_data_doc, eventDataJSONString);
    if (error != DeserializationError::Ok)
    {
        return FAILURE;
    }
    eventObject["event_data"] = event_data_doc;
    int size;
    bool pub_status = false;
    if (!checkStringIsValid(assetName))
    {
        String jsonString;
        serializeJson(eventObject, jsonString);
        int size = jsonString.length()+1;
        char payloadMsg[size];
        jsonString.toCharArray(payloadMsg,size);
        pub_status = _mqtt_client->publish(_event_topic, payloadMsg);
        eventObject.remove("event_type");
        eventObject.remove("event_data");
        eventObject.remove("event_descr");
        event_data_doc.clear();
    }
    else
    {
        JsonObject eventDisptachObject = event_dispatch_doc.to<JsonObject>();
        eventDisptachObject[assetName] = eventObject;
        String jsonString;
        serializeJson(eventDisptachObject, jsonString);
        int size = jsonString.length()+1;
        char payloadMsg[size];
        jsonString.toCharArray(payloadMsg,size);
        pub_status = _mqtt_client->publish(_event_topic, payloadMsg);
        eventDisptachObject.remove(assetName);
        event_dispatch_doc.clear();
    }
    if (pub_status == true)
    {
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}

int8_t ZohoIOTClient::publish(const char *message)
{
    if (currentState != CONNECTED)
    {
        return CLIENT_ERROR;
    }
    if (message == NULL)
    {
        return FAILURE;
    }
    if (!_mqtt_client->connected())
    {
        currentState = CONNECTION_LOST;
        return FAILURE;
    }
    //TODO: Empty validation
    if (_mqtt_client->publish(_publish_topic, message) == true)
    {
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}

int8_t ZohoIOTClient::publishCommandAck(const char *correlation_id, commandAckResponseCodes status_code, const char *responseMessage)
{
    if (currentState != CONNECTED)
    {
        return CLIENT_ERROR;
    }
    if (!checkStringIsValid(correlation_id) || responseMessage == NULL)
    {
        // Serial.println("Correlation_id or responseMessage cant be Null or empty");
        return FAILURE;
    }

    JsonDocument command_ack_message_doc;
    JsonObject commandAckMessageObj = command_ack_message_doc.to<JsonObject>();
    JsonObject commandAckObject = commandAckMessageObj[correlation_id].to<JsonObject>();
    commandAckObject["status_code"] = (int)status_code;
    commandAckObject["response"] = responseMessage;
    String jsonString;
    serializeJson(commandAckMessageObj, jsonString);
    int size = jsonString.length()+1;
    char payloadMsg[size];
    jsonString.toCharArray(payloadMsg,size);
    if (_mqtt_client->publish(_command_ack_topic, payloadMsg) == true)
    {
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}

int8_t ZohoIOTClient::dispatch()
{
    //Form json payload and publish to HUB...
    if (currentState != CONNECTED)
    {
        return CLIENT_ERROR;
    }
    String jsonString;
    serializeJson(root, jsonString);
    int size = jsonString.length() + 1;
    char payloadMsg[size];
    jsonString.toCharArray(payloadMsg,size);
    return publish(payloadMsg);
}
void ZohoIOTClient::addConnectionParameter(char *connectionParamKey, char *connectionParamValue)
{
    sprintf(connectionStringBuff, "%s%s%s%s%s", connectionStringBuff, connectionParamKey, "=", connectionParamValue, "&");
}

char *ZohoIOTClient::formConnectionString(const char *username)
{
    sprintf(connectionStringBuff, "%s%s", username, "?");
    addConnectionParameter((char *)"sdk_name", sdk_name);
    addConnectionParameter((char *)"sdk_version", sdk_version);
    // addConnectionParameter("sdk_url", sdk_url);
    connectionStringBuff[strlen(connectionStringBuff) - 1] = '\0';
    return connectionStringBuff;
}

int16_t getRetryInterval(unsigned long *curr_retry_interval)
{
    if (*curr_retry_interval <= 0)
    {
        *curr_retry_interval = MIN_RETRY_INTERVAL;
    }
    if (*curr_retry_interval < MAX_RETRY_INTERVAL)
    {
        *curr_retry_interval = *curr_retry_interval * 2;
    }
    else
    {
        *curr_retry_interval = MAX_RETRY_INTERVAL;
    }
    return *curr_retry_interval;
}
int8_t ZohoIOTClient::setMaxPayloadSize(int size)
{
 if(size > MAX_PAYLOAD_SIZE)
 {
    _mqtt_client->setBufferSize(MAX_PAYLOAD_SIZE);
    return FAILURE;
 }
 else if (size<DEFAULT_PAYLOAD_SIZE)
 {
    _mqtt_client->setBufferSize(DEFAULT_PAYLOAD_SIZE);
    return FAILURE;
 }
 else
 {
    _mqtt_client->setBufferSize(size);
    return SUCCESS;
 }
}

int8_t ZohoIOTClient::reconnect()
{
    if (currentState < INITIALIZED)
    {
        return CLIENT_ERROR;
    }

    if (_mqtt_client->connected())
    {
        //Already having an active connecting with HUB... No job to do here..
        currentState = CONNECTED;
        return SUCCESS;
    }
    if (start_time <= 0)
    {
        start_time = millis();
    }

    if (millis() - start_time > current_retry_interaval * 1000)
    {
        // Serial.println("trying to reconnect");
        int rc = connect();
        if (rc == SUCCESS)
        {
            // Serial.println("connected");
            currentState = CONNECTED;
            retryCount = 0;
            current_retry_interaval = (unsigned long)MIN_RETRY_INTERVAL;
            start_time = 0;
            _mqtt_client->subscribe(_command_topic);
            return SUCCESS;
        }
        start_time = millis();
        current_retry_interaval = getRetryInterval(&current_retry_interaval);
        retryCount = retryCount + 1;
        // Serial.println("current time");
        // Serial.println(millis());
        // Serial.print("Retrying in ");
        // Serial.print((int)current_retry_interaval);
        // Serial.println(" seconds");
        if (currentState != DISCONNECTED && currentState != CONNECTED)
        {
            return CLIENT_ERROR;
        }
    }
    return FAILURE;
}

int8_t ZohoIOTClient::connect()
{
    //TODO: Empty validation
    //TODO: resubscribe old subscriptions if reconnecting.
    if (currentState < INITIALIZED)
    {
        return CLIENT_ERROR;
    }
    // bool connectionState = _mqtt_client->connected();
    if (_mqtt_client->connected())
    {
        //Already having an active connecting with HUB... No job to do here..
        currentState = CONNECTED;
        return SUCCESS;
    }
    _mqtt_client->connect(_client_id, formConnectionString(_mqtt_user_name), _mqtt_password);
    if (_mqtt_client->connected())
    {
        currentState = CONNECTED;
        return SUCCESS;
    }
    else
    {
        currentState = RETRYING;
        return FAILURE;
    }
}

void ZohoIOTClient::onMessageReceived(char *topic, uint8_t *payload, unsigned int length)
{
    JsonDocument on_message_handler_buffer;
    JsonDocument ack_message_buffer;
    char payload_msg[length + 1];
    uint8_t frwd_payload[length];
    uint8_t len = strlen(topic);
    char frwd_topic[len];
    strcpy(frwd_topic, topic);
    for (unsigned int itr = 0; itr < length; itr++)
    {
        frwd_payload[itr] = payload[itr];
        payload_msg[itr] = (char)payload[itr];
    }
    if (strcmp(topic, _command_topic) == 0)
    {
        DeserializationError error = deserializeJson(on_message_handler_buffer,payload_msg);
        if (error != DeserializationError::Ok)
        {
            Serial.print("Cannot able to deserialize the received Message");
            return;
        }
        JsonArray commandMessageArray = on_message_handler_buffer.as<JsonArray>();
        int msglength = commandMessageArray.size();
        JsonObject commandAckMessage = ack_message_buffer.to<JsonObject>();
        for (int itr = 0; itr < msglength; itr++)
        {
            JsonDocument object;
            JsonObject commandMessageObj = commandMessageArray[itr];
            const char *correlation_id = commandMessageObj["correlation_id"];
            object["status_code"] = COMMAND_RECIEVED_ACK_CODE;
            object["response"] = "";
            commandAckMessage[correlation_id] = object;
        }
        String jsonString;
        serializeJson(commandAckMessage, jsonString);
        int size = jsonString.length() + 1;
        char commandAckMessageString[size];
        jsonString.toCharArray(commandAckMessageString,size);
        _mqtt_client->publish(_command_ack_topic, commandAckMessageString);
        on_message_handler_buffer.clear();
        ack_message_buffer.clear();
    }
    this->callback(frwd_topic, frwd_payload, length);
}

int8_t ZohoIOTClient::subscribe(MQTT_CALLBACK_SIGNATURE)
{
    //Subscribe to topic and set method to be called message on that topic.
    //TODO: Empty validation
    if (currentState != CONNECTED)
    {
        return CLIENT_ERROR;
    }
    if (callback == NULL)
    {
        return FAILURE;
    }
    if (!_mqtt_client->connected())
    {
        currentState = CONNECTION_LOST;
        return FAILURE;
    }
    _mqtt_client->setCallback([this](char *topic, uint8_t *payload, unsigned int length) { this->onMessageReceived(topic, payload, length); });
    this->callback = callback;
    if (!_mqtt_client->subscribe(_command_topic))
    {
        return FAILURE;
    }
    return SUCCESS;
}
int8_t ZohoIOTClient::get_command_topic(string& command_topic_string)
{
    if (currentState == NOT_INITIALIZED){
        return FAILURE ;
    }
    command_topic_string.assign(_command_topic);
    return SUCCESS;
}

int8_t ZohoIOTClient::disconnect()
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

string ZohoIOTClient::getPayload()
{
    String jsonString;
    serializeJson(root, jsonString);
    return std::string(jsonString.c_str());
}
