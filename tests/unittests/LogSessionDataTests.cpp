// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include <LogSessionData.hpp>

using namespace testing;
using namespace Microsoft::Applications::Events;

class TestLogSessionDataFile: public LogSessionDataFile
{
public:
   TestLogSessionDataFile(const std::string& cacheFilePath)
      : LogSessionDataFile(cacheFilePath) { }

   using LogSessionDataFile::parse;
};

const char* const PathToTestSesFile = "";

TEST(LogSessionDataFileTests, parse_EmptyString_ReturnsFalse)
{
   TestLogSessionDataFile sessionData { std::string { PathToTestSesFile } };
   ASSERT_FALSE(sessionData.parse(std::string {}));
}

TEST(LogSessionDataFileTests, parse_OneLine_ReturnsFalse)
{
   TestLogSessionDataFile sessionData { std::string { PathToTestSesFile } };
   ASSERT_FALSE(sessionData.parse(std::string {"foo" }));
}

TEST(LogSessionDataFileTests, parse_ThreeLines_ReturnsFalse)
{
   TestLogSessionDataFile sessionData { std::string { PathToTestSesFile } };
   ASSERT_FALSE(sessionData.parse(std::string { "foo\nbar\n\baz" }));
}

TEST(LogSessionDataFileTests, parse_TwoLinesFirstLaunchNotNumber_ReturnsFalse)
{
   TestLogSessionDataFile sessionData { std::string { PathToTestSesFile } };
   ASSERT_FALSE(sessionData.parse(std::string { "foo\nbar" }));
}

TEST(LogSessionDataFileTests, parse_TwoLinesFirstLaunchTooLarge_ReturnsFalse)
{
   TestLogSessionDataFile sessionData { std::string { PathToTestSesFile } };
   ASSERT_FALSE(sessionData.parse(std::string { "1000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\nbar" }));
}

TEST(LogSessionDataFileTests, parse_ValidInput_ReturnsTrue)
{
   TestLogSessionDataFile sessionData { std::string { PathToTestSesFile } };
   ASSERT_TRUE(sessionData.parse(std::string { "1234567890\nbar" }));
}
