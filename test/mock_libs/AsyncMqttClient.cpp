#include "AsyncMqttClient.h"

void AsyncMqttClient::connect() {}

bool AsyncMqttClient::connected()
{
    return true;
}

uint16_t AsyncMqttClient::publish(const char *topic, uint8_t qos, bool retain, const char *payload, size_t length, bool dup, uint16_t message_id)
{
    return message_id;
}

AsyncMqttClient &AsyncMqttClient::setCredentials(char *_device_id, char *_device_token)
{
    return *this;
}

AsyncMqttClient &AsyncMqttClient::setServer(const char *host, uint16_t port)
{
    return *this;
}

AsyncMqttClient &AsyncMqttClient::setClientId(char *_device_id)
{
    return *this;
}

void AsyncMqttClient::disconnect() {}

uint16_t AsyncMqttClient::subscribe(const char *topic, uint8_t qos)
{
    return qos;
}

AsyncMqttClient &AsyncMqttClient::onMessage(AsyncMqttClient::OnMessageUserCallback callback)
{
    return *this;
}
AsyncMqttClient &AsyncMqttClient::onPublish(AsyncMqttClient::OnPublishUserCallback callback)
{
    return *this;
}
AsyncMqttClient& AsyncMqttClient::onSubscribe(AsyncMqttClient::OnSubscribeUserCallback callback)
{
    return *this;
}
