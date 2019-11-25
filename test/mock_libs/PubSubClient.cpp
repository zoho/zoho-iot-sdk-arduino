#include "PubSubClient.h"

bool PubSubClient::connect(const char *id, const char *user, const char *pass)
{
    return true;
}
bool PubSubClient::publish(const char *topic, const char *payload)
{
    return true;
}
PubSubClient &PubSubClient::setServer(const char *domain, uint16_t port)
{
    return *this;
}
PubSubClient &PubSubClient::setClient(Client& client){
    return *this;
}
bool PubSubClient::connected()
{
    return false;
}
bool PubSubClient::subscribe(const char *topic)
{
    return true;
}
PubSubClient &PubSubClient::setCallback(MQTT_CALLBACK_SIGNATURE)
{
    return *this;
}
size_t PubSubClient::write(uint8_t)
{
    return 10;
}
size_t PubSubClient::write(const uint8_t *buffer, size_t size)
{
    return 10;
}
bool PubSubClient::loop()
{
    return true;
}
void PubSubClient::disconnect(){}