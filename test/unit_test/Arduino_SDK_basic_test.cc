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


