//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "common/Common.hpp"
#include <functional>
#include <LogSessionData.hpp>
#include "offline/LogSessionDataProvider.hpp"
#include "utils/FileUtils.hpp"
#include "utils/StringUtils.hpp"

using namespace testing;
using namespace MAT;

const std::string SessionFileArgument = "test";
const char* const SessionFile = "test.ses";

class LogSessionDataFuncTests : public ::testing::Test
{
    void CleanupLocalSessionFile()
    {
        if (MAT::FileExists(SessionFile))
        {
            MAT::FileDelete(SessionFile);
        }
    }

    virtual void SetUp() override
    {
        CleanupLocalSessionFile();
    }

    virtual void TearDown() override
    {
        CleanupLocalSessionFile();
    }
};

void ConstructSesFile(const char* sessionFile, const std::string& contents)
{
    ASSERT_TRUE(MAT::FileWrite(sessionFile, contents.c_str()));
}

void ConstructSesFile(const char* sessionFile, const std::string& utcTimeMs, const std::string& skuID)
{
    std::ostringstream stream;
    stream << utcTimeMs << '\n' << skuID;
    ConstructSesFile(sessionFile, stream.str());
}

std::string RemoveWhitespace(std::string& str)
{
    std::string trimmedString{ str };
    if (!trimmedString.empty() && trimmedString[trimmedString.length() - 1] == '\n')
    {
        trimmedString.erase(trimmedString.length() - 1);
    }
    return trimmedString;
}

std::pair<unsigned long long, std::string> ReadPropertiesFromSessionFile(const char* sessonFile)
{
    auto contents = MAT::FileGetContents(sessonFile);
    std::vector<std::string> splitContents;
    StringUtils::SplitString(contents, '\n', splitContents);
    auto sessionFirstTime = RemoveWhitespace(splitContents[0]);
    auto skuID = RemoveWhitespace(splitContents[1]);

    return std::make_pair(std::stoull(sessionFirstTime), skuID);
}

TEST(LogSessionDataFuncTests, Constructor_SessionFile_FileCreated)
{
    auto logSessionDataProvider = LogSessionDataProvider(SessionFileArgument);
    logSessionDataProvider.CreateLogSessionData();
    auto logSessionData =  logSessionDataProvider.GetLogSessionData();
    UNREFERENCED_PARAMETER(logSessionData);
    ASSERT_TRUE(MAT::FileExists(SessionFile));
}

TEST(LogSessionDataFuncTests, Constructor_ValidSessionFileExists_MembersSetToExistingFile)
{
    const std::string validSessionFirstTime{ "123456" };
    const std::string validSkuId{ "abc123" };
    ConstructSesFile(SessionFile, validSessionFirstTime, validSkuId);

    auto logSessionDataProvider = LogSessionDataProvider(SessionFileArgument);
    logSessionDataProvider.CreateLogSessionData();
    auto logSessionData =  logSessionDataProvider.GetLogSessionData();

    ASSERT_EQ(logSessionData->getSessionFirstTime(), 123456ull);
    ASSERT_EQ(logSessionData->getSessionSDKUid(), validSkuId);
}

TEST(LogSessionDataFuncTests, Constructor_InvalidSessionFileExists_MembersRegenerated)
{
    const std::string invalidSessionFirstTime{ "not-a-number" };
    const std::string validSkuId{ "abc123" };
    ConstructSesFile(SessionFile, invalidSessionFirstTime, validSkuId);

    auto logSessionDataProvider = LogSessionDataProvider(SessionFileArgument);
    logSessionDataProvider.CreateLogSessionData();
    auto logSessionData =  logSessionDataProvider.GetLogSessionData();

    ASSERT_NE(logSessionData->getSessionFirstTime(), 123456ull);
    ASSERT_NE(logSessionData->getSessionSDKUid(), validSkuId);
}

TEST(LogSessionDataFuncTests, Constructor_InvalidSessionFileExists_NewFileWritten)
{
    const std::string invalidSessionFirstTime{ "not-a-number" };
    const std::string validSkuId{ "abc123" };
    ConstructSesFile(SessionFile, invalidSessionFirstTime, validSkuId);

    auto logSessionDataProvider = LogSessionDataProvider(SessionFileArgument);
    logSessionDataProvider.CreateLogSessionData();
    auto logSessionData =  logSessionDataProvider.GetLogSessionData();
    auto properties = ReadPropertiesFromSessionFile(SessionFile);

    ASSERT_EQ(logSessionData->getSessionFirstTime(), properties.first);
    ASSERT_EQ(logSessionData->getSessionSDKUid(), properties.second);
}

