#ifndef Z_ASYNCMQTTCLIENT_H_
#define Z_ASYNCMQTTCLIENT_H_

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#define delay usleep

class AsyncMqttClient{
public:
virtual AsyncMqttClient& setServer(const char* host, uint16_t port);
virtual AsyncMqttClient& setClientId(char* _device_id);
virtual AsyncMqttClient& setCredentials(char* _device_id,char* _device_token);

virtual bool connected();
void connect();

virtual uint16_t publish(const char* topic, uint8_t qos, bool retain, const char* payload, size_t length, bool dup, uint16_t message_id);
void disconnect();
virtual uint16_t subscribe(const char* topic, uint8_t qos);
};
#endif // MOCK_ASYNCMQTTCLIENT_H_