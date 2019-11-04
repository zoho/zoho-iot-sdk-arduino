#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "AsyncMqttClient.h"
#include <fakeit.hpp>
#include "zoho-iot-client.h"

using namespace fakeit;
char mqtt_user_name[] = "/domain_name/v1/devices/client_id/connect";
char mqtt_password[] = "mqtt_password";
Mock<AsyncMqttClient> mock;

TEST_CASE("Init")
{
    Fake(Method(mock, setServer));
    Fake(Method(mock, setClientId));
    Fake(Method(mock, setCredentials));
    AsyncMqttClient &fake_client = mock.get();
    ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
    SECTION("InitMethod_WithNotNULLArguments_ShouldSucceed")
    {
        // Init method with non-NULL parameters returns success.
        REQUIRE(zc->init(mqtt_user_name, mqtt_password) == 0);
    }
    SECTION("InitMehod_WithNULLArguments_ShouldFail")
    {
        // Init method with NULL parameters returns failure
        REQUIRE(zc->init(NULL, mqtt_password) == -1);
    }
}

TEST_CASE("Connect")
{
    Fake(Method(mock, setServer));
    Fake(Method(mock, setClientId));
    Fake(Method(mock, setCredentials));
    SECTION("ConnectMehod_WithExistingConnection_ShouldSucceed")
    {
        // Connect called on existing connection return success.
        fakeit::When(Method(mock, connected)).AlwaysReturn(true);
        AsyncMqttClient &fake_client = mock.get();
        ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
        zc->init(mqtt_user_name, mqtt_password);
        REQUIRE(zc->connect() == 0);
    }
    SECTION("ConnectMehod_WithNewConnection_ShouldSucceeed")
    {
        // Connect with new connection return success.
        fakeit::When(Method(mock, connected)).Return(2_Times(false), 1_Times(true));
        AsyncMqttClient &fake_client = mock.get();
        ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
        REQUIRE(zc->connect() == 0);
    }
    SECTION("ConnectMehod_WithLostConnection_ShouldFail")
    {
        // Connect with failure Connection lost return failure success.
        fakeit::When(Method(mock, connected)).AlwaysReturn(false);
        AsyncMqttClient &fake_client = mock.get();
        ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
        REQUIRE(zc->connect() == -2);
    }
}

TEST_CASE("AddDataPointNumber")
{
    AsyncMqttClient &fake_client = mock.get();
    ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
    SECTION("AddDataPointNumberMehod_WhenIntegerAdded_ShouldSucceed")
    {
        // Add integer data point returns success.
        REQUIRE(zc->addDataPointNumber("key", 987) == true);
    }
    SECTION("AddDataPointNumberMehod_WhenFloatAdded_ShouldSucceed")
    {
        // Add float data point returns success.
        REQUIRE(zc->addDataPointNumber("key1", 0.123) == true);
    }
    SECTION("AddDataPointNumberMehod_WhenNULLKeyAdded_ShouldFail")
    {
        // Add data point with returns NULL key failure.
        REQUIRE(zc->addDataPointNumber(NULL, 0.123) == false);
    }
    SECTION("AddDataPointNumberMehod_WhenSameKeyAdded_ShouldSucceed")
    {
        // Add data point with same key return success.
        zc->addDataPointNumber("key1", 0.123);
        REQUIRE(zc->addDataPointNumber("key1", 456) == true);
    }
}

TEST_CASE("AddDataPointString")
{
    AsyncMqttClient &fake_client = mock.get();
    ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
    SECTION("AddDataPointStringMehod_WhenStringAdded_ShouldSucceed")
    {
        // Add data Point String with non-null arguments returns success
        REQUIRE(zc->addDataPointString("key1", "value1") == true);
    }
    SECTION("AddDataPointStringMehod_WhenNULLKeyAdded_ShouldFail")
    {
        // Add data Point String with null arguments returns success
        REQUIRE(zc->addDataPointString("key", NULL) == false);
    }
    SECTION("AddDataPointStringMehod_WhenSameKeyAdded_ShouldSucceed")
    {
        // Add data point String with same key return success.
        string str = "value";
        zc->addDataPointString("key1", str);
        REQUIRE(zc->addDataPointString("key1", "456") == true);
    }
}

TEST_CASE("Publish")
{
    char message[] = "message";
    SECTION("PublishMehod_WithNotNULLArguments_ShouldSucceed")
    {
        // Publish returns success with non-null message argument.
        fakeit::When(Method(mock, connected)).Return(true);
        fakeit::When(OverloadedMethod(mock, publish, uint16_t(const char *, uint8_t, bool, const char *, size_t, bool, uint16_t))).AlwaysReturn(1);
        auto &fake_client = mock.get();
        ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
        zc->init(mqtt_user_name, mqtt_password);
        REQUIRE(zc->publish(message) == 0);
    }

    SECTION("PublishMehod_WithLostConnection_ShouldFail")
    {
        // Publish with lost connnection returns failure.
        fakeit::When(Method(mock, connected)).Return(false);
        fakeit::When(OverloadedMethod(mock, publish, uint16_t(const char *, uint8_t, bool, const char *, size_t, bool, uint16_t))).AlwaysReturn(1);
        auto &fake_client = mock.get();
        ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
        zc->init(mqtt_user_name, mqtt_password);
        REQUIRE(zc->publish(message) == -1);
    }

    SECTION("PublishMehod_WithNULLMessage_ShouldFail")
    {
        // Publish with NULL message returns failure.
        fakeit::When(Method(mock, connected)).Return(true);
        fakeit::When(OverloadedMethod(mock, publish, uint16_t(const char *, uint8_t, bool, const char *, size_t, bool, uint16_t))).AlwaysReturn(1);
        auto &fake_client = mock.get();
        ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
        zc->init(mqtt_user_name, mqtt_password);
        REQUIRE(zc->publish(NULL) == -1);
    }
}

TEST_CASE("Subscribe")
{
    char topic[] = "topic";
    fakeit::When(OverloadedMethod(mock, subscribe, uint16_t(const char *, uint8_t))).AlwaysReturn(1);
    SECTION("SubscribeMehod_WithNotNULLarguments_ShouldSucceed")
    {
        fakeit::When(Method(mock, connected)).AlwaysReturn(true);
        auto &fake_client = mock.get();
        ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
        REQUIRE(zc->subscribe(topic) == 0);
    }

    SECTION("SubscribeMehod_WithNULLarguments_ShouldFail")
    {
        // Subscribe with NULL Arguments return failure.
        fakeit::When(Method(mock, connected)).AlwaysReturn(true);
        auto &fake_client = mock.get();
        ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
        REQUIRE(zc->subscribe(NULL) == -1);
    }

    SECTION("SubscribeMehod_WithLostConnection_ShouldFail")
    {
        // Subscribe with failure case.
        fakeit::When(Method(mock, connected)).AlwaysReturn(false);
        auto &fake_client = mock.get();
        ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
        REQUIRE(zc->subscribe(topic) == -1);
    }
}

TEST_CASE("Dispatch")
{
    fakeit::When(Method(mock, connected)).Return(true);
    fakeit::When(OverloadedMethod(mock, publish, uint16_t(const char *, uint8_t, bool, const char *, size_t, bool, uint16_t))).AlwaysReturn(1);
    auto &fake_client = mock.get();
    ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
    zc->init(mqtt_user_name, mqtt_password);
    char key[] = "key";
    char value[] = "value";
    SECTION("DispatchMehod_WithIntegerAsDatapoint_ShouldSucceed")
    {
        // Dispatch integer return success.
        zc->addDataPointNumber(key, 10);
        REQUIRE(zc->dispatch() == 0);
    }
    SECTION("DispatchMehod_WithFloatAsDatapoint_ShouldSucceed")
    {
        // Dispatch float return success.
        zc->addDataPointNumber(key, 10.10);
        REQUIRE(zc->dispatch() == 0);
    }
    SECTION("DispatchMehod_WithStringAsDatapoint_ShouldSucceed")
    {
        // Dispatch String return success.
        zc->addDataPointString(key, value);
        REQUIRE(zc->dispatch() == 0);
    }
    SECTION("DispatchMehod_WithUnPublishedPayload_ShouldFail")
    {
        // Dispatch return faiure as Publish failed.
        fakeit::When(OverloadedMethod(mock, publish, uint16_t(const char *, uint8_t, bool, const char *, size_t, bool, uint16_t))).Return(0);
        auto &fake_client = mock.get();
        ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
        zc->init(mqtt_user_name, mqtt_password);
        REQUIRE(zc->dispatch() == -1);
    }
}

TEST_CASE("Disconnect")
{
    SECTION("DisconnectMehod_WithActiveConnection_ShouldSucceed")
    {
        fakeit::When(Method(mock, connected)).Return(1_Times(true), false);
        auto &fake_client = mock.get();
        ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
        REQUIRE(zc->disconnect() == 0);
    }
    SECTION("DisconnectMehod_WithAlreadyDisconnectedClient_ShouldSucceed")
    {
        fakeit::When(Method(mock, connected)).AlwaysReturn(false);
        auto &fake_client = mock.get();
        ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
        REQUIRE(zc->disconnect() == 0);
    }
    SECTION("DisconnectMehod_WithRejectedDisconnect_ShouldFail")
    {
        fakeit::When(Method(mock, connected)).AlwaysReturn(true);
        auto &fake_client = mock.get();
        ZohoIOTClient *zc = new ZohoIOTClient(&fake_client);
        REQUIRE(zc->disconnect() == -1);
    }
}