// Copyright (c) Microsoft. All rights reserved.
//
// TODO: re-enable TPM testcases for backoff configuration change
//
#include <map>

#include "common/Common.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include "common/MockIBandwidthController.hpp"
#include "TransmitProfiles.hpp"

using namespace testing;
using namespace MAT;

class TransmitProfilesTests : public StrictMock<Test> {};

TEST_F(TransmitProfilesTests, NetCostSelectsTimer)
{
  const char* fredProfile = R"(
[{
    "name": "Fred_Profile",
    "rules": [
    { "netCost": "restricted", "timers": [  -1,  -1,  5 ] },
    { "netCost": "high",       "timers": [  -1,  -1,  4 ] },
    { "netCost": "low",        "timers": [  -1,  -1,  3 ] },
    { "netCost": "unknown",    "timers": [  -1,  -1,  2 ] },
    {                          "timers": [  -1,  -1,  1 ] }
    ]
}]
)";
  std::map<NetworkCost, int8_t> expectations;
  expectations[NetworkCost_Roaming] = 5;
  expectations[NetworkCost_Metered] = 4;
  expectations[NetworkCost_Unmetered] = 3;
  expectations[NetworkCost_Unknown] = 2;
  EXPECT_TRUE(TransmitProfiles::load(fredProfile));
  EXPECT_TRUE(TransmitProfiles::setProfile("Fred_Profile"));
  std::vector<int> timers;
  for (auto & i: expectations) {
    EXPECT_TRUE(TransmitProfiles::updateStates(i.first, PowerSource_Battery));
    TransmitProfiles::getTimers(timers);
    ASSERT_EQ(3, timers.size());
    EXPECT_EQ(1000 * i.second, timers[2]);
  }
}

TEST_F(TransmitProfilesTests, SelectBoth)
{
    const char* fredProfile = R"(
[{
    "name": "Fred_Profile",
    "rules": [
    { "netCost": "low",         "powerState": "battery",    "timers": [ 24, 12,  6 ] },
    { "timers": [  -1,  -1,  4 ] }
    ]
}]
)";
    EXPECT_TRUE(TransmitProfiles::load(fredProfile));
    EXPECT_TRUE(TransmitProfiles::setProfile("Fred_Profile"));
    EXPECT_TRUE(TransmitProfiles::updateStates(NetworkCost_Unmetered, PowerSource_Battery));
    std::vector<int> timers;
    TransmitProfiles::getTimers(timers);
    ASSERT_EQ(3, timers.size());
    EXPECT_EQ(6000, timers[2]);
}

TEST_F(TransmitProfilesTests, NeedBoth)
{
    const char* fredProfile = R"(
[{
    "name": "Fred_Profile",
    "rules": [
    { "netCost": "low",         "powerState": "battery",    "timers": [ 24, 12,  6 ] },
    { "timers": [  -1,  -1,  4 ] }
    ]
}]
)";
    EXPECT_TRUE(TransmitProfiles::load(fredProfile));
    EXPECT_TRUE(TransmitProfiles::setProfile("Fred_Profile"));
    EXPECT_TRUE(TransmitProfiles::updateStates(NetworkCost_Unmetered, PowerSource_Unknown));
    std::vector<int> timers;
    TransmitProfiles::getTimers(timers);
    ASSERT_EQ(3, timers.size());
    EXPECT_EQ(4000, timers[2]);
}
