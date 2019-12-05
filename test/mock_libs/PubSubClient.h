#ifndef PubSubClient_h
#define PubSubClient_h

#include <stdint.h>
#include <stdlib.h>
#include <Client.h>

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define MQTT_CALLBACK_SIGNATURE std::function<void(char*, uint8_t*, unsigned int)> callback
#else
#define MQTT_CALLBACK_SIGNATURE void (*callback)(char*, uint8_t*, unsigned int)
#endif

#define delay(x) { }

class PubSubClient
{
    public:
    inline PubSubClient(){}
    PubSubClient(Client& client);
    virtual bool connect(const char* id, const char* user, const char* pass);
    virtual bool publish(const char* topic, const char* payload);
    virtual PubSubClient& setServer(const char * domain, uint16_t port);
    virtual PubSubClient& setClient(Client& client);
    virtual bool connected();
    virtual bool subscribe(const char* topic); 
    virtual PubSubClient& setCallback(MQTT_CALLBACK_SIGNATURE);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buffer, size_t size);
    virtual bool loop();
};

#endif 