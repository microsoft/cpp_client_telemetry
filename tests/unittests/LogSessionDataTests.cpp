// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include <LogSessionData.hpp>

using namespace testing;
using namespace Microsoft::Applications::Events;

class TestLogSessionData : public LogSessionData
{
public:
   TestLogSessionData(const std::string& cacheFilePath)
      : LogSessionData(cacheFilePath) { }

   using LogSessionData::parse;
};

const char* const PathToTestSesFile = "";

TEST(LogSessionDataTests, parse_EmptyString_ReturnsFalse)
{
   TestLogSessionData sessionData { std::string { PathToTestSesFile } };
   ASSERT_FALSE(sessionData.parse(std::string {}));
}

TEST(LogSessionDataTests, parse_OneLine_ReturnsFalse)
{
   TestLogSessionData sessionData { std::string { PathToTestSesFile } };
   ASSERT_FALSE(sessionData.parse(std::string {"foo" }));
}

TEST(LogSessionDataTests, parse_ThreeLines_ReturnsFalse)
{
   TestLogSessionData sessionData { std::string { PathToTestSesFile } };
   ASSERT_FALSE(sessionData.parse(std::string { "foo\nbar\n\baz" }));
}

TEST(LogSessionDataTests, parse_TwoLinesFirstLaunchNotNumber_ReturnsFalse)
{
   TestLogSessionData sessionData { std::string { PathToTestSesFile } };
   ASSERT_FALSE(sessionData.parse(std::string { "foo\nbar" }));
}

TEST(LogSessionDataTests, parse_TwoLinesFirstLaunchTooLarge_ReturnsFalse)
{
   TestLogSessionData sessionData { std::string { PathToTestSesFile } };
   ASSERT_FALSE(sessionData.parse(std::string { "1000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\nbar" }));
}

TEST(LogSessionDataTests, parse_ValidInput_ReturnsTrue)
{
   TestLogSessionData sessionData { std::string { PathToTestSesFile } };
   ASSERT_TRUE(sessionData.parse(std::string { "1234567890\nbar" }));
}