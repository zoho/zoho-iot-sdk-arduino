#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "PubSubClient.h"
#include <string>
#include <math.h>
#include <fakeit.hpp>
#include "zoho-iot-client.h"

using namespace fakeit;

TEST_CASE("Init")
{
  Mock<PubSubClient> mock;
  PubSubClient client;
  fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).Return(client);
  auto &fake_client = mock.get();
  ZohoIOTClient zc(fake_client);

  SECTION("Init - appropriate arguments")
  {
    // Init method with non-NULL parameters returns success.
    char device_id[] = "device_id";
    char device_token[] = "device_token";
    REQUIRE(zc.init(device_id, device_token) == 0);
  }

  SECTION("Init - NULL arguments")
  {
    // Init method with NULL parameters returns failure
    char *device_id = NULL;
    char *device_token = NULL;
    REQUIRE(zc.init(device_id, device_token) == -1);
  }
}

TEST_CASE("Publish")
{

  Mock<PubSubClient> mock;
  fakeit::When(Method(mock, connected)).Return(true);

  SECTION("Publish - appropriate arguments ")
  {
    // Publish returns success with non-null message argument.
    fakeit::When(Method(mock, connected)).Return(true);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).Return(true);
    auto &fake_client = mock.get();
    ZohoIOTClient zc(fake_client);
    char message[] = "message";
    REQUIRE(zc.publish(message) == 0);
  }

  SECTION("Publish - lost connection")
  {
    // Publish with lost connnection returns failure.
    fakeit::When(Method(mock, connected)).Return(false);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).Return(true);
    auto &fake_client = mock.get();
    ZohoIOTClient zc(fake_client);
    char message[] = "message";
    REQUIRE(zc.publish(message) == -1);
  }

  SECTION("Publish - NULL message")
  {
    // Publish with NULL message returns failure.
    fakeit::When(Method(mock, connected)).Return(true);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).Return(true);
    auto &fake_client = mock.get();
    ZohoIOTClient zc(fake_client);
    REQUIRE(zc.publish(NULL) == -1);
  }
}

TEST_CASE("basic zoho-iot-client test")
{
  Mock<PubSubClient> mock;
  fakeit::When(OverloadedMethod(mock, connect, bool(const char *, const char *, const char *))).Return(true);
  fakeit::When(Method(mock, connected)).Return(true);
  fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).Return(false);
  auto &client = mock.get();
  ZohoIOTClient zc(client);
  char message[] = "message";
  REQUIRE(zc.publish(message) == -1);
}

TEST_CASE("AddDataPointNUmber")
{
  SECTION("AddDataPointNumber - integer")
  {
    // Add integer data point returns success.
    Mock<PubSubClient> mock;
    auto &client = mock.get();
    ZohoIOTClient zc(client);
    REQUIRE(zc.addDataPointNumber("key", 987) == true);
  }

  SECTION("AddDataPointNumber - integer")
  {
    // Add float data point returns success.
    Mock<PubSubClient> mock;
    auto &client = mock.get();
    ZohoIOTClient zc(client);
    REQUIRE(zc.addDataPointNumber("key1", 0.123) == true);
  }

  SECTION("AddDataPointNumber -  NULL key ")
  {
    // Add data point with returns NULL key failure.
    Mock<PubSubClient> mock;
    auto &client = mock.get();
    ZohoIOTClient zc(client);
    REQUIRE(zc.addDataPointNumber(NULL, 0.123) == false);
  }

  SECTION("AddDataPointNumber - repeated key ")
  {
    // Add data point with same key return success.
    Mock<PubSubClient> mock;
    auto &client = mock.get();
    ZohoIOTClient zc(client);
    zc.addDataPointNumber("key1", 0.123);
    REQUIRE(zc.addDataPointNumber("key1", 456) == true);
  }
}

TEST_CASE("AddDataPointString")
{
  Mock<PubSubClient> mock;
  auto &client = mock.get();
  ZohoIOTClient zc(client);
  SECTION("AddDataPointString")
  {
    // Add data Point String with non-null arguments returns success
    REQUIRE(zc.addDataPointString("key1", "value1") == true);
  }
  SECTION("AddDataPointString - with NULL arguments")
  {
    // Add data Point String with null arguments returns success
    REQUIRE(zc.addDataPointString("key", NULL) == false);
  }

  SECTION("AddDataPointString - with repeated key ")
  {
    // Add data point with same key return success.
    string str = "value";
    zc.addDataPointString("key1", str);
    REQUIRE(zc.addDataPointString("key1", "456") == true);
  }
}

TEST_CASE("Connect")
{
  Mock<PubSubClient> mock;
  SECTION("Connect - existing connection")
  {
    // Connect called on existing connection return success.
    fakeit::When(OverloadedMethod(mock, connect, bool(const char *, const char *, const char *))).Return(1_Times(false), true);
    fakeit::When(Method(mock, connected)).Return(true);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).Return(false);
    auto &client = mock.get();
    ZohoIOTClient zc(client);
    REQUIRE(zc.connect() == 0);
  }
  SECTION("Connect - new connection")
  {
    // Connect with new connection return success.
    fakeit::When(OverloadedMethod(mock, connect, bool(const char *, const char *, const char *))).Return(true);
    fakeit::When(Method(mock, connected)).Return(2_Times(false), 1_Times(true));
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).Return(true);
    auto &client = mock.get();
    ZohoIOTClient zc(client);
    REQUIRE(zc.connect() == 0);
  }
  SECTION("Connect - failure Connection lost ")
  {
    // Connect with failure Connection lost return failure success.
    fakeit::When(OverloadedMethod(mock, connect, bool(const char *, const char *, const char *))).Return(true);
    fakeit::When(Method(mock, connected)).Return(3_Times(false), true);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).Return(false);
    auto &client = mock.get();
    ZohoIOTClient zc(client);
    REQUIRE(zc.connect() == -1);
  }

  SECTION("connect")
  {
    fakeit::When(OverloadedMethod(mock, connect, bool(const char *, const char *, const char *))).Return(true);
    fakeit::When(Method(mock, connected)).Return(1_Times(false), true);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).Return(false);
    auto &client = mock.get();
    ZohoIOTClient zc(client);
    REQUIRE(zc.connect() == 0);
  }
}

void callback(char *, uint8_t *, unsigned int) {}
TEST_CASE("Subscribe")
{
  Mock<PubSubClient> mock;
  PubSubClient pub_client;
  fakeit::When(OverloadedMethod(mock, setCallback, PubSubClient & (MQTT_CALLBACK_SIGNATURE))).Return(pub_client);

  SECTION("Subscribe - appropriate arguments")
  {
    fakeit::When(OverloadedMethod(mock, subscribe, bool(const char *))).Return(true);
    auto &client = mock.get();
    ZohoIOTClient zc(client);
    // Subscribe with non-null arguments return success.
    char topic[] = "topic";
    REQUIRE(zc.subscribe(topic, callback) == 0);
  }

  SECTION("Subscribe - null arguments")
  {
    // Subscribe with NULL Arguments return failure.
    fakeit::When(OverloadedMethod(mock, subscribe, bool(const char *))).Return(true);
    auto &client = mock.get();
    ZohoIOTClient zc(client);
    REQUIRE(zc.subscribe(NULL, callback) == -1);
  }

  SECTION("Subscribe - failure")
  {
    // Subscribe with failure case.
    fakeit::When(OverloadedMethod(mock, subscribe, bool(const char *))).Return(false);
    auto &client = mock.get();
    ZohoIOTClient zc(client);
    char topic[] = "topic";
    REQUIRE(zc.subscribe(topic, callback) == -1);
  }
}

TEST_CASE("Dispatch")
{
  Mock<PubSubClient> mock;
  fakeit::When(Method(mock, connected)).Return(true);
  fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).Return(true);
  auto &client = mock.get();
  ZohoIOTClient zc(client);
  char key[] = "key";
  char value[] = "value";
  SECTION("Dispacth integer")
  {
    // Dispatch integer return success.
    zc.addDataPointNumber(key, 10);
    REQUIRE(zc.dispatch() == 0);
  }
  SECTION("Dispacth float")
  {
    // Dispatch float return success.
    zc.addDataPointNumber(key, 10.10);
    REQUIRE(zc.dispatch() == 0);
  }
  SECTION("Dispacth String")
  {
    // Dispatch String return success.
    zc.addDataPointString(key, value);
    REQUIRE(zc.dispatch() == 0);
  }
  SECTION("Dispatch failure")
  {
    // Dispatch return faiure as Publish failed.
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).Return(false);
    auto &client = mock.get();
    ZohoIOTClient zc(client);
    REQUIRE(zc.dispatch() == -1);
  }
}

