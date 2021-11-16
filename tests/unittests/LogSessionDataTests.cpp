//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "offline/LogSessionDataProvider.hpp"
#include <LogSessionData.hpp>

using namespace testing;
using namespace Microsoft::Applications::Events;

class TestLogSessionDataProvider : public LogSessionDataProvider
{
public:
    TestLogSessionDataProvider(const std::string &str): LogSessionDataProvider(str) {}
    using LogSessionDataProvider::parse;
};

const char* const PathToTestSesFile = "";
const char* const PathToNonEmptyTestSesFile = "sesfile";

std::string sessionSDKUid;
uint64_t sessionFirstTimeLaunch;


TEST(LogSessionDataTests, parse_EmptyString_ReturnsFalse)
{   
   TestLogSessionDataProvider logSessionDataProvider(PathToTestSesFile);
   ASSERT_FALSE(logSessionDataProvider.parse(std::string {}, sessionFirstTimeLaunch, sessionSDKUid));
}

TEST(LogSessionDataTests, parse_OneLine_ReturnsFalse)
{
   TestLogSessionDataProvider logSessionDataProvider(PathToTestSesFile);
   ASSERT_FALSE(logSessionDataProvider.parse(std::string {"foo" }, sessionFirstTimeLaunch, sessionSDKUid));
}

TEST(LogSessionDataTests, parse_ThreeLines_ReturnsFalse)
{
   TestLogSessionDataProvider logSessionDataProvider(PathToTestSesFile);
   ASSERT_FALSE(logSessionDataProvider.parse(std::string { "foo\nbar\n\baz" }, sessionFirstTimeLaunch, sessionSDKUid));
}

TEST(LogSessionDataTests, parse_TwoLinesFirstLaunchNotNumber_ReturnsFalse)
{
   TestLogSessionDataProvider logSessionDataProvider(PathToTestSesFile);
   ASSERT_FALSE(logSessionDataProvider.parse(std::string { "foo\nbar" }, sessionFirstTimeLaunch, sessionSDKUid));
}

TEST(LogSessionDataTests, parse_TwoLinesFirstLaunchTooLarge_ReturnsFalse)
{
   TestLogSessionDataProvider logSessionDataProvider(PathToTestSesFile);
   ASSERT_FALSE(logSessionDataProvider.parse(std::string { "1111111111111111111111111111111111111111111111111111111111111111111\nbar" },
               sessionFirstTimeLaunch, sessionSDKUid));
}

TEST(LogSessionDataTests, parse_MissingNewLineAtEnd_ReturnsFalse)
{
   TestLogSessionDataProvider logSessionDataProvider(PathToTestSesFile);
   ASSERT_FALSE(logSessionDataProvider.parse(std::string { "1234567890\nbar" }, sessionFirstTimeLaunch, sessionSDKUid));
}

TEST(LogSessionDataTests, parse_ValidInput_ReturnsTrue)
{
   TestLogSessionDataProvider logSessionDataProvider(PathToTestSesFile);
   ASSERT_TRUE(logSessionDataProvider.parse(std::string { "1234567890\nbar\n" }, sessionFirstTimeLaunch, sessionSDKUid));
   ASSERT_EQ(sessionFirstTimeLaunch, (uint64_t)1234567890);
   ASSERT_EQ(sessionSDKUid, "bar");
}

TEST(LogSessionDataTests, getLogSessionData_ValidInput_SessionDataPersists)
{
   TestLogSessionDataProvider logSessionDataProvider1(PathToNonEmptyTestSesFile);
   logSessionDataProvider1.CreateLogSessionData();
   auto logSessionData1 = logSessionDataProvider1.GetLogSessionData();

   // Create another provider instance and valiate session data is not re-generated
   TestLogSessionDataProvider logSessionDataProvider2(PathToNonEmptyTestSesFile);
   logSessionDataProvider2.CreateLogSessionData();
   auto logSessionData2 = logSessionDataProvider2.GetLogSessionData();

   ASSERT_EQ(logSessionData1->getSessionFirstTime(), logSessionData2->getSessionFirstTime());
   ASSERT_EQ(logSessionData1->getSessionSDKUid(), logSessionData2->getSessionSDKUid());
}

