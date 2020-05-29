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
  PubSubClient *pub_client = new PubSubClient();
  ZohoIOTClient zc(pub_client, false);
  char mqttUserName[] = "/mqtt_domain_name/v1/devices/client_id/connect";
  char mqttPassword[] = "mqtt_password";
  SECTION("Init_ShouldReturnsFailure_WithNULLArguments")
  {
    // Init method with NULL parameters returns failure
    char *nullMqttUserName = NULL;
    char *nullMqttPassword = NULL;
    REQUIRE(zc.init(nullMqttUserName, nullMqttPassword) == -1);
  }
  SECTION("Init_ShouldReturnsSuccess_WithNotNULLArguments ")
  {
    // Init method with non-NULL parameters returns success.
    REQUIRE(zc.init(mqttUserName, mqttPassword) == 0);
  }
}

TEST_CASE("Connect")
{
  char mqttUserName[] = "/mqtt_domain_name/v1/devices/client_id/connect";
  char mqttPassword[] = "mqtt_password";
  PubSubClient *pub_client = new PubSubClient();
  SECTION("Connect_ShouldreturnFailure_WhenCalledWithoutInitialization")
  {
    // Connect when called without initialization should return Failure.
    Mock<PubSubClient> mock;
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    REQUIRE(zc.connect() == -2);
  }
  SECTION("Connect_ShouldreturnFailure_WithLostConnection")
  {
    // Connect with failure Connection lost return failure success.
    Mock<PubSubClient> mock;
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    fakeit::When(OverloadedMethod(mock, connect, bool(const char *, const char *, const char *))).AlwaysReturn(false);
    fakeit::When(Method(mock, connected)).AlwaysReturn(false);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    zc.init(mqttUserName, mqttPassword);
    REQUIRE(zc.connect() == -2);
  }
  SECTION("Connect_ShouldreturnSuccess_WithExistingConnection")
  {
    // Connect called on existing connection return success.
    Mock<PubSubClient> mock;
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    fakeit::When(Method(mock, connected)).AlwaysReturn(true);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    zc.init(mqttUserName, mqttPassword);
    REQUIRE(zc.connect() == 0);
  }
  SECTION("Connect_ShouldreturnSuccess_WithNewConnection")
  {
    // Connect with new connection return success.
    Mock<PubSubClient> mock;
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    fakeit::When(Method(mock, connected)).Return(2_Times(false), true);
    fakeit::When(OverloadedMethod(mock, connect, bool(const char *, const char *, const char *))).AlwaysReturn(true);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    zc.init(mqttUserName, mqttPassword);
    REQUIRE(zc.connect() == 0);
  }
  SECTION("Connect_ShouldReturnSuccess_WithRetriedConnection")
  {
    // Connect should return success with retried connection.
    Mock<PubSubClient> mock;
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    fakeit::When(Method(mock, connected)).Return(4_Times(false), true);
    fakeit::When(OverloadedMethod(mock, connect, bool(const char *, const char *, const char *))).AlwaysReturn(true);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    zc.init(mqttUserName, mqttPassword);
    REQUIRE(zc.connect() == 0);
  }
}

TEST_CASE("Publish")
{
  char mqttUserName[] = "/mqtt_domain_name/v1/devices/client_id/connect";
  char mqttPassword[] = "mqtt_password";
  PubSubClient *pub_client = new PubSubClient();
  SECTION("Publish_ShouldReturnsFailure_WhenCalledWithoutConnection")
  {
    // Publish returns Failure when called without initialization/connection.
    Mock<PubSubClient> mock;
    fakeit::When(Method(mock, connected)).Return(true);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).Return(true);
    auto &fake_client = mock.get();
    ZohoIOTClient zc(&fake_client, false);
    char message[] = "message";
    REQUIRE(zc.publish(message) == -2);
  }
  SECTION("Publish_ShouldReturnsSuccess_WithNotNULLArguments ")
  {
    // Publish returns success with non-null message argument.
    Mock<PubSubClient> mock;
    fakeit::When(Method(mock, connected)).AlwaysReturn(true);
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).AlwaysReturn(true);
    auto &fake_client = mock.get();
    ZohoIOTClient zc(&fake_client, false);
    zc.init(mqttUserName, mqttPassword);
    zc.connect();
    char message[] = "message";
    REQUIRE(zc.publish(message) == 0);
  }
  SECTION("Publish_ShouldReturnsFailure_WithLostConnection")
  {
    // Publish with lost connnection returns failure.
    Mock<PubSubClient> mock;
    fakeit::When(Method(mock, connected)).Return(1_Times(true), false);
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).Return(true);
    auto &fake_client = mock.get();
    ZohoIOTClient zc(&fake_client, false);
    zc.init(mqttUserName, mqttPassword);
    zc.connect();
    char message[] = "message";
    REQUIRE(zc.publish(message) == -1);
  }
  SECTION("Publish_ShouldReturnFailure_WithNULLMessage")
  {
    // Publish with NULL message returns failure.
    Mock<PubSubClient> mock;
    fakeit::When(Method(mock, connected)).AlwaysReturn(true);
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).Return(true);
    auto &fake_client = mock.get();
    ZohoIOTClient zc(&fake_client, false);
    zc.init(mqttUserName, mqttPassword);
    zc.connect();
    REQUIRE(zc.publish(NULL) == -1);
  }
}

TEST_CASE("AddDataPointNUmber")
{
  Mock<PubSubClient> mock;
  auto &client = mock.get();
  ZohoIOTClient zc(&client, false);
  SECTION("AddDataPointNumber_ShouldReturnSuccess_WhenIntegerAdded")
  {
    // Add integer data point returns success.
    REQUIRE(zc.addDataPointNumber("key", 987) == true);
  }
  SECTION("AddDataPointNumber_ShouldReturnSuccess_WhenFloatAdded")
  {
    // Add float data point returns success.
    REQUIRE(zc.addDataPointNumber("key1", 0.123) == true);
  }
  SECTION("AddDataPointNumber_ShouldReturnFailure_WhenNULLKeyAdded")
  {
    // Add data point number with NULL key returns failure.
    REQUIRE(zc.addDataPointNumber(NULL, 0.123) == false);
  }
  SECTION("AddDataPointNumber_ShouldReturnSuccess_WhenSameKeyAdded")
  {
    // Add data point number with same key return success, i.e replace old value .
    zc.addDataPointNumber("key1", 0.123);
    REQUIRE(zc.addDataPointNumber("key1", 456) == true);
  }

  SECTION("AddDataPointNumber_ShouldReturnSuccess_WhenIntegerAddedwithNotNULLAssetName")
  {
    // Add integer data point returns success with Not NULL asset name.
    REQUIRE(zc.addDataPointNumber("key", 987, "asset") == true);
  }
  SECTION("AddDataPointNumber_ShouldReturnSuccess_WhenFloatAddedwithNotNULLAssetName")
  {
    // Add float data point returns success with Not NULL asset name.
    REQUIRE(zc.addDataPointNumber("key1", 0.123, "asset") == true);
  }
  SECTION("AddDataPointNumber_ShouldReturnFailure_WhenNULLKeyAddedwithNotNULLAssetName")
  {
    // Add data point number with NULL key returns failure with Not NULL asset name.
    REQUIRE(zc.addDataPointNumber(NULL, 0.123, "asset") == false);
  }
  SECTION("AddDataPointNumber_ShouldReturnSuccess_WhenSameKeyAddedwithNotNULLAssetName")
  {
    // Add data point number with same key return success with Not NULL asset name, i.e replace old value .
    zc.addDataPointNumber("key1", 0.123, "asset");
    REQUIRE(zc.addDataPointNumber("key1", 456, "asset") == true);
  }

  SECTION("AddDataPointNumber_ShouldReturnSuccess_WhenIntegerAddedwithNULLAssetName")
  {
    // Add integer data point return success with NULL asset name , i.e adding int to root object.
    REQUIRE(zc.addDataPointNumber("key", 987, NULL) == true);
  }
  SECTION("AddDataPointNumber_ShouldReturnSuccess_WhenIntegerAddedwithNULLAssetName")
  {
    // Add float/double data point return success with NULL asset name , i.e adding float/double to root object.
    REQUIRE(zc.addDataPointNumber("key", 0.123, NULL) == true);
  }
  SECTION("AddDataPointNumber_ShouldReturnSuccess_WhenIntegerAddedwithEmptyAssetName")
  {
    // Add float/double data point return success with empty asset name , i.e adding float/double to root object.
    REQUIRE(zc.addDataPointNumber("key", 0.123, "") == true);
  }
}

TEST_CASE("AddDataPointString")
{
  Mock<PubSubClient> mock;
  auto &client = mock.get();
  ZohoIOTClient zc(&client, false);
  SECTION("AddDataPointString_ShouldReturnSuccess_WhenStringAdded")
  {
    // Add data Point String with non-null arguments returns success
    REQUIRE(zc.addDataPointString("key1", "value1") == true);
  }
  SECTION("AddDataPointString_ShouldReturnFailure_WhenNULLKeyAdded")
  {
    // Add data Point String with null arguments returns success
    REQUIRE(zc.addDataPointString("key", NULL) == false);
  }
  SECTION("AddDataPointString_ShouldReturnSuccess_WhenSameKeyAdded")
  {
    // Add data point String with same key return success, i.e replace old value .
    string str = "value";
    zc.addDataPointString("key1", str);
    REQUIRE(zc.addDataPointString("key1", "456") == true);
  }
  SECTION("AddDataPointString_ShouldReturnSuccess_WhenStringAddedwithNotNULLAssetName")
  {
    // Add data Point String with non-null arguments returns success with Not NULL asset name.
    REQUIRE(zc.addDataPointString("key1", "value1", "asset1") == true);
  }
  SECTION("AddDataPointString_ShouldReturnFailure_WhenNULLKeyAddedwithNotNULLAssetName")
  {
    // Add data Point String with null arguments returns failure with Not NULL asset name
    REQUIRE(zc.addDataPointString("key", NULL, "asset1") == false);
  }
  SECTION("AddDataPointString_ShouldReturnSuccess_WhenSameKeyAddedwithNotNULLAssetName")
  {
    // Add data point String with same key return success with Not NULL asset name, i.e replace old value .
    string str = "value";
    zc.addDataPointString("key1", str);
    REQUIRE(zc.addDataPointString("key1", "456", "asset1") == true);
  }
  SECTION("AddDataPointString_ShouldReturnSuccess_WhenStringAddedwithNULLAssetName")
  {
    // Add data point String return success with NULL asset name , i.e adding string to root object.
    REQUIRE(zc.addDataPointString("key1", "value1", NULL) == true);
  }
  SECTION("AddDataPointString_ShouldReturnSuccess_WhenStringAddedwithNULLAssetName")
  {
    // Add data point String return success with NULL asset name , i.e adding string to root object.
    string str = "value";
    REQUIRE(zc.addDataPointString("key1", str, NULL) == true);
  }
  SECTION("AddDataPointString_ShouldReturnSuccess_WhenStringAddedwithEmptyAssetName")
  {
    // Add data point String return success with empty asset name , i.e adding string to root object.
    string str = "value";
    REQUIRE(zc.addDataPointString("key1", str, "") == true);
  }
}

TEST_CASE("markDataPointAsError")
{
  Mock<PubSubClient> mock;
  auto &client = mock.get();
  ZohoIOTClient zc(&client, false);
  SECTION("markDataPointAsError_ShouldReturnSuccess_WhenDatapointIsMarkedAsErrorWithNoAssetName")
  {
    // marking DataPoint as Error with no assetName should return success when a datapoint is marked as error.
    REQUIRE(zc.markDataPointAsError("key1") == true);
  }
  SECTION("markDataPointAsError_ShouldReturnSuccess_WhenDatapointIsMarkedAsErrorWithAssetName")
  {
    // marking DataPoint as Error with assetName should return success when a datapoint is marked as error.
    REQUIRE(zc.markDataPointAsError("key1", "asset1") == true);
  }
  SECTION("markDataPointAsError_ShouldReturnSuccess_WhenDatapointIsMarkedAsErrorWithNULLAssetName")
  {
    // marking DataPoint as Error with Null assetName should return success , i.e marking root object's key as error.
    REQUIRE(zc.markDataPointAsError("key1", NULL) == true);
  }
}

void callback(char *, uint8_t *, unsigned int) {}
TEST_CASE("Subscribe")
{
  char mqttUserName[] = "/mqtt_domain_name/v1/devices/client_id/connect";
  char mqttPassword[] = "mqtt_password";
  SECTION("Subscribe_ShouldReutrnSuccess_WithNotNULLarguments")
  {
    // Subscribe with non-null arguments return success.
    Mock<PubSubClient> mock;
    PubSubClient *pub_client = new PubSubClient();
    fakeit::When(Method(mock, connected)).AlwaysReturn(true);
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    fakeit::When(OverloadedMethod(mock, setCallback, PubSubClient & (MQTT_CALLBACK_SIGNATURE))).AlwaysReturn(*pub_client);
    fakeit::When(OverloadedMethod(mock, subscribe, bool(const char *))).AlwaysReturn(true);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    zc.init(mqttUserName, mqttPassword);
    zc.connect();
    char topic[] = "topic";
    REQUIRE(zc.subscribe(topic, callback) == 0);
  }
  SECTION("Subscribe_ShouldReutrnFailure_WhenCalledWithoutConnection")
  {
    // Subscribe when called without initialization return Connectionerror.
    Mock<PubSubClient> mock;
    PubSubClient *pub_client = new PubSubClient();
    fakeit::When(Method(mock, connected)).AlwaysReturn(true);
    fakeit::When(OverloadedMethod(mock, setCallback, PubSubClient & (MQTT_CALLBACK_SIGNATURE))).AlwaysReturn(*pub_client);
    fakeit::When(OverloadedMethod(mock, subscribe, bool(const char *))).AlwaysReturn(true);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    char topic[] = "topic";
    REQUIRE(zc.subscribe(topic, callback) == -2);
  }
  SECTION("Subscribe_ShouldReutrnFailure_WithNULLarguments")
  {
    // Subscribe with NULL Arguments return failure.
    Mock<PubSubClient> mock;
    PubSubClient *pub_client = new PubSubClient();
    fakeit::When(Method(mock, connected)).AlwaysReturn(true);
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    fakeit::When(OverloadedMethod(mock, subscribe, bool(const char *))).Return(true);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    zc.init(mqttUserName, mqttPassword);
    zc.connect();
    REQUIRE(zc.subscribe(NULL, callback) == -1);
  }
  SECTION("Subscribe_ShouldReutrnFailure_WithLostConnection")
  {
    // Subscribe with failure case.
    Mock<PubSubClient> mock;
    PubSubClient *pub_client = new PubSubClient();
    fakeit::When(Method(mock, connected)).Return(1_Times(true), false);
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    zc.init(mqttUserName, mqttPassword);
    zc.connect();
    char topic[] = "topic";
    REQUIRE(zc.subscribe(topic, callback) == -1);
  }
}

TEST_CASE("Dispatch")
{
  Mock<PubSubClient> mock;
  PubSubClient *pub_client = new PubSubClient();
  char mqttUserName[] = "/mqtt_domain_name/v1/devices/client_id/connect";
  char mqttPassword[] = "mqtt_password";
  fakeit::When(Method(mock, connected)).AlwaysReturn(true);
  fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).AlwaysReturn(true);
  fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
  auto &client = mock.get();
  ZohoIOTClient zc(&client, false);
  zc.init(mqttUserName, mqttPassword);
  zc.connect();
  char key[] = "key";
  char value[] = "value";
  SECTION("Dispatch_ShouldReturnSuccess_WithIntegerAsDatapoint")
  {
    // Dispatch integer return success.
    zc.addDataPointNumber(key, 10);
    REQUIRE(zc.dispatch() == 0);
  }
  SECTION("Dispatch_ShouldReturnSuccess_WithFloatAsDatapoint")
  {
    // Dispatch float return success.
    zc.addDataPointNumber(key, 10.10);
    REQUIRE(zc.dispatch() == 0);
  }
  SECTION("Dispatch_ShouldReturnSuccess_WithFloatAsDatapoint")
  {
    // Dispatch String return success.
    zc.addDataPointString(key, value);
    REQUIRE(zc.dispatch() == 0);
  }
  SECTION("Dispatch_ShouldReturnFailure_WhenCalledWithoutConnection")
  {
    // Dispatch return faiure when called without connection.
    Mock<PubSubClient> mock;
    fakeit::When(Method(mock, connected)).AlwaysReturn(true);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).AlwaysReturn(false);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    REQUIRE(zc.dispatch() == -2);
  }
  SECTION("Dispatch_ShouldReturnFailure_WithUnPublishedPayload")
  {
    // Dispatch return faiure as Publish failed.
    Mock<PubSubClient> mock;
    fakeit::When(Method(mock, connected)).AlwaysReturn(true);
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).AlwaysReturn(false);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    zc.init(mqttUserName, mqttPassword);
    zc.connect();
    REQUIRE(zc.dispatch() == -1);
  }
}