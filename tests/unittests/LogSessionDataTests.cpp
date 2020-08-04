// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include "offline/LogSessionDataProvider.hpp"
#include <LogSessionData.hpp>

using namespace testing;
using namespace Microsoft::Applications::Events;

class TestLogSessionDataProvider : public LogSessionDataProvider
{
public:
   using LogSessionDataProvider::parse;
};

const char* const PathToTestSesFile = "";
std::string sessionSDKUid;
unsigned long long sessionFirstTimeLaunch;


TEST(LogSessionDataTests, parse_EmptyString_ReturnsFalse)
{
   auto logSessionData = TestLogSessionDataProvider::CreateLogSessionData(std::string { PathToTestSesFile });
   //TestLogSessionData sessionData { std::string { PathToTestSesFile } };
   ASSERT_FALSE(TestLogSessionDataProvider::parse(std::string {}, sessionFirstTimeLaunch, sessionSDKUid));
}

TEST(LogSessionDataTests, parse_OneLine_ReturnsFalse)
{
   //TestLogSessionData sessionData { std::string { PathToTestSesFile } };
   auto logSessionData = TestLogSessionDataProvider::CreateLogSessionData(std::string { PathToTestSesFile });
   ASSERT_FALSE(TestLogSessionDataProvider::parse(std::string {"foo" }, sessionFirstTimeLaunch, sessionSDKUid));
}

TEST(LogSessionDataTests, parse_ThreeLines_ReturnsFalse)
{
   //TestLogSessionData sessionData { std::string { PathToTestSesFile } };
   auto logSessionData = TestLogSessionDataProvider::CreateLogSessionData(std::string { PathToTestSesFile });
   ASSERT_FALSE(TestLogSessionDataProvider::parse(std::string { "foo\nbar\n\baz" }, sessionFirstTimeLaunch, sessionSDKUid));
}

TEST(LogSessionDataTests, parse_TwoLinesFirstLaunchNotNumber_ReturnsFalse)
{
   //TestLogSessionData sessionData { std::string { PathToTestSesFile } };
   auto logSessionData = TestLogSessionDataProvider::CreateLogSessionData(std::string { PathToTestSesFile });
   ASSERT_FALSE(TestLogSessionDataProvider::parse(std::string { "foo\nbar" }, sessionFirstTimeLaunch, sessionSDKUid));
}

TEST(LogSessionDataTests, parse_TwoLinesFirstLaunchTooLarge_ReturnsFalse)
{
   //TestLogSessionData sessionData { std::string { PathToTestSesFile } };
   auto logSessionData = TestLogSessionDataProvider::CreateLogSessionData(std::string { PathToTestSesFile });
   ASSERT_FALSE(TestLogSessionDataProvider::parse(std::string { "1000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\nbar" },
               sessionFirstTimeLaunch, sessionSDKUid));
}

TEST(LogSessionDataTests, parse_ValidInput_ReturnsTrue)
{
   //TestLogSessionData sessionData { std::string { PathToTestSesFile } };
   auto logSessionData = TestLogSessionDataProvider::CreateLogSessionData(std::string { PathToTestSesFile });
   ASSERT_TRUE(TestLogSessionDataProvider::parse(std::string { "1234567890\nbar" }, sessionFirstTimeLaunch, sessionSDKUid));
}
