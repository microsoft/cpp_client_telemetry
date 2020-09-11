// Copyright (c) Microsoft. All rights reserved.
#include "common/Common.hpp"

#define HAVE_MAT_DEFAULTDATAVIEWER

#if defined __has_include
#if __has_include("modules/PrivacyGuard/PrivacyGuard.hpp")
#include "modules/PrivacyGuard/PrivacyGuard.hpp"
#else
/* Compiling without Data Viewer */
#undef HAVE_MAT_DEFAULTDATAVIEWER
#endif
#endif

#ifdef HAVE_MAT_DEFAULTDATAVIEWER

#include "CheckForExceptionOrAbort.hpp"
#include "ILogger.hpp"
#include "NullObjects.hpp"

#include <functional>

using namespace testing;
using namespace MAT;

class MockLogger : public NullLogger
{
   public:
    std::function<void(const EventProperties& properties)> m_logEventOverride;
    virtual void LogEvent(EventProperties const& properties) override
    {
        if (m_logEventOverride)
        {
            m_logEventOverride(properties);
        }
    }
};

TEST(PrivacyGuardTests, Constructor_LoggerInstanceNotProvided_LoggerInstanceThrowsInvalidArgument)
{
    CheckForExceptionOrAbort<std::invalid_argument>([]() { PrivacyGuard(nullptr, nullptr); });
}

TEST(PrivacyGuardTests, Constructor_LoggerInstanceProvided_InitializedSuccessfully)
{
    auto mockLogger = std::make_shared<MockLogger>();
    PrivacyGuard pg(mockLogger, nullptr);
    ASSERT_TRUE(pg.GetState());
    ASSERT_FALSE(pg.AreCommonPrivacyContextSet());
}

#endif