//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "offline/LogSessionDataProvider.hpp"
#include <LogSessionData.hpp>
#include "utils/FileUtils.hpp"

using namespace testing;
using namespace Microsoft::Applications::Events;

class TestLogSessionDataProvider : public LogSessionDataProvider
{
public:
    TestLogSessionDataProvider(const std::string &str): LogSessionDataProvider(str) {}
    using LogSessionDataProvider::parse;
};

const char* const PathToTestSesFile = "";
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

TEST(LogSessionDataTests, parse_ValidCrlfInput_ReturnsTrue)
{
   TestLogSessionDataProvider provider(PathToTestSesFile);
   ASSERT_TRUE(provider.parse("1234567890\r\nbar\r\n", sessionFirstTimeLaunch, sessionSDKUid));
   EXPECT_EQ(sessionFirstTimeLaunch, uint64_t{1234567890});
   EXPECT_EQ(sessionSDKUid, "bar");
}

TEST(LogSessionDataTests, parse_TrailingData_ReturnsFalse)
{
   TestLogSessionDataProvider provider(PathToTestSesFile);
   EXPECT_FALSE(provider.parse("1234567890\nbar\nextra", sessionFirstTimeLaunch, sessionSDKUid));
   EXPECT_FALSE(provider.parse("1234567890\r\nbar\r\n\x1a" "hidden",
                              sessionFirstTimeLaunch, sessionSDKUid));
}

TEST(LogSessionDataTests, parse_OversizedInput_ReturnsFalse)
{
   TestLogSessionDataProvider provider(PathToTestSesFile);
   const std::string content = "1234567890\n" + std::string(MAX_FILE_CONTENTS_SIZE, 'x') + "\n";
   ASSERT_FALSE(provider.parse(content, sessionFirstTimeLaunch, sessionSDKUid));
}

TEST(LogSessionDataTests, getLogSessionData_ValidInput_SessionDataPersists)
{
   const std::string sessionFile =
       GetTempDirectory() + "sesfile-" + std::to_string(PAL::getUtcSystemTimeMs());
   std::remove(sessionFile.c_str());

   TestLogSessionDataProvider logSessionDataProvider1(sessionFile);
   logSessionDataProvider1.CreateLogSessionData();
   const auto* logSessionData1 = logSessionDataProvider1.GetLogSessionData();

   // Create another provider instance and validate session data is not re-generated
   TestLogSessionDataProvider logSessionDataProvider2(sessionFile);
   logSessionDataProvider2.CreateLogSessionData();
   const auto* logSessionData2 = logSessionDataProvider2.GetLogSessionData();

   ASSERT_EQ(logSessionData1->getSessionFirstTime(), logSessionData2->getSessionFirstTime());
   ASSERT_EQ(logSessionData1->getSessionSDKUid(), logSessionData2->getSessionSDKUid());

   logSessionDataProvider1.DeleteLogSessionData();
   logSessionDataProvider2.DeleteLogSessionData();
}

class LogSessionFileTests : public Test
{
protected:
   std::string cachePath = GetTempDirectory() + "session-file-" + PAL::generateUuidString();
   std::string sessionPath = cachePath + ".ses";

   ~LogSessionFileTests() override
   {
       FileDelete(sessionPath.c_str());
   }

   void write(const std::string& content)
   {
       FILE* file = FileOpen(sessionPath.c_str(), "wb");
       ASSERT_NE(file, nullptr);
       const size_t written = fwrite(content.data(), 1, content.size(), file);
       const int closed = FileClose(file);
       ASSERT_EQ(written, content.size());
       ASSERT_EQ(closed, 0);
   }
};

TEST_F(LogSessionFileTests, PreservesExistingCrlfSession)
{
   write("1234567890\r\noriginal-id\r\n");
   TestLogSessionDataProvider provider(cachePath);
   provider.CreateLogSessionData();
   ASSERT_NE(provider.GetLogSessionData(), nullptr);
   EXPECT_EQ(provider.GetLogSessionData()->getSessionFirstTime(), uint64_t{1234567890});
   EXPECT_EQ(provider.GetLogSessionData()->getSessionSDKUid(), "original-id");
}

TEST_F(LogSessionFileTests, RegeneratesOversizedSessionWithControlZTail)
{
   write("1234567890\r\noriginal-id\r\n" + std::string(1, '\x1a') +
         std::string(MAX_FILE_CONTENTS_SIZE, 'x'));
   TestLogSessionDataProvider provider(cachePath);
   provider.CreateLogSessionData();
   ASSERT_NE(provider.GetLogSessionData(), nullptr);
   EXPECT_NE(provider.GetLogSessionData()->getSessionFirstTime(), uint64_t{1234567890});
   EXPECT_NE(provider.GetLogSessionData()->getSessionSDKUid(), "original-id");
   EXPECT_FALSE(provider.GetLogSessionData()->getSessionSDKUid().empty());
   EXPECT_LE(FileGetContents(sessionPath.c_str()).size(), MAX_FILE_CONTENTS_SIZE);
}
