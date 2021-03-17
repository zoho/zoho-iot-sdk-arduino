#ifndef CATCH_CONFIG_MAIN
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
    REQUIRE(zc.init(nullMqttUserName, nullMqttPassword) == ZohoIOTClient::FAILURE);
  }
  SECTION("Init_ShouldReturnsSuccess_WithNotNULLArguments ")
  {
    // Init method with non-NULL parameters returns success.
    REQUIRE(zc.init(mqttUserName, mqttPassword) == ZohoIOTClient::SUCCESS);
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
    REQUIRE(zc.connect() == ZohoIOTClient::CLIENT_ERROR);
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
    REQUIRE(zc.connect() == ZohoIOTClient::FAILURE);
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
    REQUIRE(zc.connect() == ZohoIOTClient::SUCCESS);
  }
  SECTION("Connect_ShouldreturnSuccess_WithNewConnection")
  {
    // Connect with new connection return success.
    Mock<PubSubClient> mock;
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    fakeit::When(Method(mock, connected)).Return(1_Times(false), true);
    fakeit::When(OverloadedMethod(mock, connect, bool(const char *, const char *, const char *))).AlwaysReturn(true);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    zc.init(mqttUserName, mqttPassword);
    REQUIRE(zc.connect() == ZohoIOTClient::SUCCESS);
  }
  // SECTION("Connect_ShouldReturnSuccess_WithRetriedConnection")
  // {
  //   // Connect should return success with retried connection.
  //   Mock<PubSubClient> mock;
  //   fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
  //   fakeit::When(Method(mock, connected)).Return(4_Times(false), true);
  //   fakeit::When(OverloadedMethod(mock, connect, bool(const char *, const char *, const char *))).AlwaysReturn(true);
  //   auto &client = mock.get();
  //   ZohoIOTClient zc(&client, false);
  //   zc.init(mqttUserName, mqttPassword);
  //   REQUIRE(zc.connect() == ZohoIOTClient::SUCCESS);
  // }
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
    REQUIRE(zc.publish(message) == ZohoIOTClient::CLIENT_ERROR);
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
    REQUIRE(zc.publish(message) == ZohoIOTClient::SUCCESS);
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
    REQUIRE(zc.publish(message) == ZohoIOTClient::FAILURE);
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
    REQUIRE(zc.publish(NULL) == ZohoIOTClient::FAILURE);
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
  SECTION("AddDataPointNumber_ShouldReturnSuccess_WhenFloatAddedwithNULLAssetName")
  {
    // Add float/double data point return success with NULL asset name , i.e adding float/double to root object.
    REQUIRE(zc.addDataPointNumber("key", 0.123, NULL) == true);
  }
  SECTION("AddDataPointNumber_ShouldReturnSuccess_WhenFloatAddedwithEmptyAssetName")
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
    // Add data Point String with null arguments returns failure
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

void subscribe_callback(char *, uint8_t *, unsigned int) {}
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
    REQUIRE(zc.subscribe(subscribe_callback) == 0);
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
    REQUIRE(zc.subscribe(subscribe_callback) == ZohoIOTClient::CLIENT_ERROR);
  }
  SECTION("Subscribe_ShouldReutrnFailure_WithNULLarguments")
  {
    // Subscribe with NULL Arguments return failure.
    Mock<PubSubClient> mock;
    PubSubClient *pub_client = new PubSubClient();
    fakeit::When(Method(mock, connected)).AlwaysReturn(true);
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    fakeit::When(OverloadedMethod(mock, setCallback, PubSubClient & (MQTT_CALLBACK_SIGNATURE))).AlwaysReturn(*pub_client);
    fakeit::When(OverloadedMethod(mock, subscribe, bool(const char *))).Return(true);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    zc.init(mqttUserName, mqttPassword);
    zc.connect();
    REQUIRE(zc.subscribe(NULL) == ZohoIOTClient::FAILURE);
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
    REQUIRE(zc.subscribe(subscribe_callback) == ZohoIOTClient::FAILURE);
  }
}

TEST_CASE("DispatchEventFromEventDataObject")
{
  char mqttUserName[] = "/mqtt_domain_name/v1/devices/client_id/connect";
  char mqttPassword[] = "mqtt_password";
  char eventType[] = "eventType";
  char assetName[] = "assetName";
  char emptyString[] = "";
  char eventDescription[] = "{\"key1\":1}";
  PubSubClient *pub_client = new PubSubClient();
  Mock<PubSubClient> mock;
  fakeit::When(Method(mock, connected)).AlwaysReturn(true);
  fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
  fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).AlwaysReturn(true);
  auto &fake_client = mock.get();
  ZohoIOTClient zc(&fake_client, false);
  zc.init(mqttUserName, mqttPassword);
  zc.connect();
  SECTION("DispatchEventFromEventDataObject_ShouldReturnSuccess_WithoutAssetName")
  {
    // DispatchEventFromEventDataObject return success without AssetName.
    zc.addEventDataPointNumber("key1", 10);
    REQUIRE(zc.dispatchEventFromEventDataObject(eventType, eventDescription, emptyString) == 0);
  }
  SECTION("DispatchEventFromEventDataObject_ShouldReturnSuccess_WithAssetName")
  {
    // DispatchEventFromEventDataObject return success with AssetName.
    zc.addEventDataPointNumber("key2", 20);
    REQUIRE(zc.dispatchEventFromEventDataObject(eventType, eventDescription, assetName) == 0);
  }
  SECTION("DispatchEventFromEventDataObject_ShouldReturnFailure_WithImporperArguments")
  {
    // DispatchEventFromEventDataObject return failure without Proper arguments.
    zc.addEventDataPointNumber("key1", 20);
    REQUIRE(zc.dispatchEventFromEventDataObject(emptyString, eventDescription, assetName) == ZohoIOTClient::FAILURE);
    REQUIRE(zc.dispatchEventFromEventDataObject(eventType, NULL, assetName) == ZohoIOTClient::FAILURE);
  }
  SECTION("DispatchEventFromEventDataObject_ShouldReturnFailure_WithoutConnection")
  {
    // DispatchEventFromEventDataObject return faiure when called without connection.
    Mock<PubSubClient> mock;
    fakeit::When(Method(mock, connected)).AlwaysReturn(true);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).AlwaysReturn(false);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    zc.addEventDataPointNumber("key1", 10);
    REQUIRE(zc.dispatchEventFromEventDataObject(eventType, eventDescription, assetName) == ZohoIOTClient::CLIENT_ERROR);
  }
  SECTION("DispatchEventFromEventDataObject_ShouldReturnFailure_WithUnPublishedPayload")
  {
    // DispatchEventFromEventDataObject return faiure as Publish failed.
    Mock<PubSubClient> mock;
    fakeit::When(Method(mock, connected)).AlwaysReturn(true);
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).AlwaysReturn(false);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    zc.init(mqttUserName, mqttPassword);
    zc.connect();
    zc.addEventDataPointNumber("key1", 40);
    REQUIRE(zc.dispatchEventFromEventDataObject(eventType, eventDescription, assetName) == ZohoIOTClient::FAILURE);
  }
}

TEST_CASE("AddEventDataPointNumber")
{
  Mock<PubSubClient> mock;
  auto &client = mock.get();
  ZohoIOTClient zc(&client, false);
  SECTION("AddEventDataPointNumber_ShouldReturnSuccess_WhenIntegerAdded")
  {
    // Add addEventDataPointNumber with int data returns success.
    REQUIRE(zc.addEventDataPointNumber("key1", 123) == true);
  }
  SECTION("AddEventDataPointNumber_ShouldReturnSuccess_WhenFloatAdded")
  {
    // AddEventDataPointNumber with float data returns success.
    REQUIRE(zc.addEventDataPointNumber("key1", 0.123) == true);
  }
  SECTION("AddEventDataPointNumber_ShouldReturnFailure_WhenImproperKeysAreAdded")
  {
    // Add addEventDataPointNumber with null/empty arguments returns False
    REQUIRE(zc.addEventDataPointNumber("", 123) == false);
    REQUIRE(zc.addEventDataPointNumber(NULL, 0.123) == false);
  }
  SECTION("AddEventDataPointNumber_ShouldReturnSuccess_WhenSameKeyAdded")
  {
    // AddEventDataPointNumber with same key return success, i.e replace old value .
    zc.addEventDataPointNumber("key1", 0.123);
    REQUIRE(zc.addEventDataPointNumber("key1", 456) == true);
  }
}

TEST_CASE("addEventDataPointString")
{
  Mock<PubSubClient> mock;
  auto &client = mock.get();
  ZohoIOTClient zc(&client, false);
  SECTION("AddEventDataPointString_ShouldReturnSuccess_WhenStringAdded")
  {
    // AddEventDataPointString with non-null arguments returns success
    REQUIRE(zc.addEventDataPointString("key1", (char *)"value1") == true);
  }
  SECTION("AddEventDataPointString_ShouldReturnFailure_WhenNULLKeyAdded")
  {
    // AddEventDataPointString with null arguments returns failure
    REQUIRE(zc.addEventDataPointString("key", NULL) == false);
  }
  SECTION("AddEventDataPointString_ShouldReturnSuccess_WhenSameKeyAdded")
  {
    // AddEventDataPointString with same key return success, i.e replace old value .
    string str = "value1";
    zc.addDataPointString("key1", str);
    REQUIRE(zc.addEventDataPointString("key1", (char *)"value2") == true);
  }
}

TEST_CASE("DispatchEventFromJSONString")
{
  char mqttUserName[] = "/mqtt_domain_name/v1/devices/client_id/connect";
  char mqttPassword[] = "mqtt_password";
  char eventType[] = "eventType";
  char assetName[] = "assetName";
  char emptyString[] = "";
  char eventDescription[] = "{\"key1\":1}";
  PubSubClient *pub_client = new PubSubClient();
  Mock<PubSubClient> mock;
  fakeit::When(Method(mock, connected)).AlwaysReturn(true);
  fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
  fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).AlwaysReturn(true);
  char eventDataJsonString[] = "{\"abc\":1}";
  auto &fake_client = mock.get();
  ZohoIOTClient zc(&fake_client, false);
  zc.init(mqttUserName, mqttPassword);
  zc.connect();
  SECTION("DispatchEventFromJSONString_ShouldReturnSuccess_WithoutAssetName")
  {
    // DispatchEventFromJSONString return success without AssetName.
    REQUIRE(zc.dispatchEventFromJSONString(eventType, eventDescription, eventDataJsonString, emptyString) == 0);
  }
  SECTION("DispatchEventFromJSONString_ShouldReturnSuccess_WithAssetName")
  {
    // DispatchEventFromJSONString return success with AssetName.
    REQUIRE(zc.dispatchEventFromJSONString(eventType, eventDescription, eventDataJsonString, assetName) == 0);
  }
  SECTION("DispatchEventFromJSONString_ShouldReturnFailure_WithImporperArguments")
  {
    // DispatchEventFromJSONString return failure without Proper arguments.
    REQUIRE(zc.dispatchEventFromJSONString(emptyString, eventDescription, eventDataJsonString, assetName) == ZohoIOTClient::FAILURE);
    REQUIRE(zc.dispatchEventFromJSONString(eventType, NULL, eventDataJsonString, assetName) == ZohoIOTClient::FAILURE);
  }
  SECTION("DispatchEventFromJSONString_ShouldReturnFailure_WithImporperEventDataJsonString")
  {
    // DispatchEventFromJSONString return failure without ImProper EventDataJsonString.
    char wrongDataJsonString[] = "{\"str\" 3}";
    REQUIRE(zc.dispatchEventFromJSONString(eventType, eventDescription, wrongDataJsonString, emptyString) == ZohoIOTClient::FAILURE);
  }
  SECTION("DispatchEventFromJSONString_ShouldReturnFailure_WithoutConnection")
  {
    // DispatchEventFromJSONString return faiure when called without connection.
    Mock<PubSubClient> mock;
    fakeit::When(Method(mock, connected)).AlwaysReturn(true);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).AlwaysReturn(false);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    REQUIRE(zc.dispatchEventFromJSONString(eventType, eventDescription, eventDataJsonString, assetName) == ZohoIOTClient::CLIENT_ERROR);
  }
  SECTION("DispatchEventFromJSONString_ShouldReturnFailure_WithUnPublishedPayload")
  {
    // DispatchEventFromJSONString return faiure as Publish failed.
    Mock<PubSubClient> mock;
    fakeit::When(Method(mock, connected)).AlwaysReturn(true);
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).AlwaysReturn(false);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    zc.init(mqttUserName, mqttPassword);
    zc.connect();
    REQUIRE(zc.dispatchEventFromJSONString(eventType, eventDescription, eventDataJsonString, assetName) == ZohoIOTClient::FAILURE);
  }
}

TEST_CASE("PublishCommandAck")
{
  char mqttUserName[] = "/mqtt_domain_name/v1/devices/client_id/connect";
  char mqttPassword[] = "mqtt_password";
  ZohoIOTClient::commandAckResponseCodes code = ZohoIOTClient::SUCCESFULLY_EXECUTED;
  PubSubClient *pub_client = new PubSubClient();
  Mock<PubSubClient> mock;
  auto &fake_client = mock.get();
  fakeit::When(Method(mock, connected)).AlwaysReturn(true);
  fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
  fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).AlwaysReturn(true);
  ZohoIOTClient zc(&fake_client, false);
  zc.init(mqttUserName, mqttPassword);
  zc.connect();
  char response[] = "Succesfull response";
  char correlation_id[] = "Correltaion_id";
  SECTION("PublishCommandAck_ShouldReturnSuccess_WithProperArguments")
  {
    // PublishCommandAck return success WithProperArguments.
    REQUIRE(zc.publishCommandAck(correlation_id, code, response) == 0);
  }
  SECTION("PublishCommandAck_ShouldReturnFailure_WithImporperArguments")
  {
    // PublishCommandAck return failure without Proper arguments.
    REQUIRE(zc.publishCommandAck(NULL, code, response) == ZohoIOTClient::FAILURE);
    REQUIRE(zc.publishCommandAck(correlation_id, code, NULL) == ZohoIOTClient::FAILURE);
  }
  SECTION("PublishCommandAck_ShouldReturnFailure_WithoutConnection")
  {
    // PublishCommandAck return faiure when called without connection.
    Mock<PubSubClient> mock;
    fakeit::When(Method(mock, connected)).AlwaysReturn(true);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).AlwaysReturn(false);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    REQUIRE(zc.publishCommandAck(correlation_id, code, response) == ZohoIOTClient::CLIENT_ERROR);
  }
  SECTION("PublishCommandAck_ShouldReturnFailure_WithUnPublishedPayload")
  {
    // PublishCommandAck return faiure as Publish failed.
    Mock<PubSubClient> mock;
    fakeit::When(Method(mock, connected)).AlwaysReturn(true);
    fakeit::When(OverloadedMethod(mock, setServer, PubSubClient & (const char *, uint16_t))).AlwaysReturn(*pub_client);
    fakeit::When(OverloadedMethod(mock, publish, bool(const char *, const char *))).AlwaysReturn(false);
    auto &client = mock.get();
    ZohoIOTClient zc(&client, false);
    zc.init(mqttUserName, mqttPassword);
    zc.connect();
    REQUIRE(zc.publishCommandAck(correlation_id, code, response) == ZohoIOTClient::FAILURE);
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
    REQUIRE(zc.dispatch() == ZohoIOTClient::CLIENT_ERROR);
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
    REQUIRE(zc.dispatch() == ZohoIOTClient::FAILURE);
  }
}
#endif