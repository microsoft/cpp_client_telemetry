//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "common/Common.hpp"
#include "api/Logger.hpp"

using namespace testing;
using namespace MAT;

class TestLogger : public Logger
{
public:
    TestLogger(const std::string& tenantToken,
        const std::string& source,
        const std::string& scope,
        ILogManagerInternal& logManager,
        ContextFieldsProvider& parentContext,
        IRuntimeConfig& runtimeConfig) noexcept
        : Logger(tenantToken, source, scope, logManager, parentContext, runtimeConfig) { }
    using Logger::CanEventPropertiesBeSent;

    bool SubmitCalled = {};
    void submit(::CsProtocol::Record&, const EventProperties&) override
    {
        SubmitCalled = true;
    }
};

class LoggerTests : public ::testing::Test
{
public:
    LoggerTests() noexcept
        : logManager(configuration)
        , runtimeConfig(configuration)
        , logger("", "", "", logManager, contextFieldsProvider, runtimeConfig)
    { }

    ILogConfiguration configuration;
    LogManagerImpl logManager;
    ContextFieldsProvider contextFieldsProvider;
    RuntimeConfig_Default runtimeConfig;
    TestLogger logger;

    virtual void SetUp() override
    {
        logManager.GetEventFilters().UnregisterAllFilters();
    }

protected:
    class LoggerTestEventFilter : public IEventFilter
    {
    public:
        LoggerTestEventFilter(bool canEventPropertiesBeSentReturnValue) noexcept
            : CanEventPropertiesBeSentReturnValue(canEventPropertiesBeSentReturnValue) { }
        const char* GetName() const noexcept override { return "TestEventFilter"; }
        bool CanEventPropertiesBeSentReturnValue{};
        bool CanEventPropertiesBeSent(const EventProperties&) const noexcept override { return CanEventPropertiesBeSentReturnValue; }
    };

    static std::unique_ptr<IEventFilter> MakeTestEventFilter(bool returnValue) noexcept
    {
        return std::unique_ptr<LoggerTestEventFilter>(new LoggerTestEventFilter(returnValue));
    }
};

TEST_F(LoggerTests, CanEventPropertiesBeSent_NoFiltersInLoggerOrLogManager_ReturnsTrue)
{
    EXPECT_TRUE(logger.CanEventPropertiesBeSent(EventProperties{}));
}

TEST_F(LoggerTests, CanEventPropertiesBeSent_FilterInLoggerReturnsTrue_ReturnsTrue)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    EXPECT_TRUE(logger.CanEventPropertiesBeSent(EventProperties{}));
}

TEST_F(LoggerTests, CanEventPropertiesBeSent_FilterInLogManagerReturnsFalse_ReturnsTrue)
{
    logManager.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    EXPECT_TRUE(logger.CanEventPropertiesBeSent(EventProperties{}));
}

TEST_F(LoggerTests, CanEventPropertiesBeSent_FilterInLoggerReturnsFalse_ReturnsFalse)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    EXPECT_FALSE(logger.CanEventPropertiesBeSent(EventProperties{}));
}

TEST_F(LoggerTests, CanEventPropertiesBeSent_FilterInLogManagerReturnsFalse_ReturnsFalse)
{
    logManager.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    EXPECT_FALSE(logger.CanEventPropertiesBeSent(EventProperties{}));
}

TEST_F(LoggerTests, CanEventPropertiesBeSent_FilterInLoggerReturnsTrueFilterInLogManagerReturnsFalse_ReturnsFalse)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logManager.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    EXPECT_FALSE(logger.CanEventPropertiesBeSent(EventProperties{}));
}

TEST_F(LoggerTests, CanEventPropertiesBeSent_FilterInLoggerReturnsFalseFilterInLogManagerReturnsTrue_ReturnsFalse)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    logManager.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    EXPECT_FALSE(logger.CanEventPropertiesBeSent(EventProperties{}));
}

TEST_F(LoggerTests, CanEventPropertiesBeSent_FilterInLoggerReturnsTrueFilterInLogManagerReturnsTrue_ReturnsTrue)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logManager.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    EXPECT_TRUE(logger.CanEventPropertiesBeSent(EventProperties{}));
}

TEST_F(LoggerTests, LogAppLifecycle_CanEventPropertiesBeSentReturnsFalse_DoesNotCallSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    logger.LogAppLifecycle(AppLifecycleState::AppLifecycleState_Background, EventProperties{});
    EXPECT_FALSE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogAppLifecycle_CanEventPropertiesBeSentReturnsTrue_CallsSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logger.LogAppLifecycle(AppLifecycleState::AppLifecycleState_Background, EventProperties{});
    EXPECT_TRUE (logger.SubmitCalled);
}

TEST_F(LoggerTests, LogEvent_CanEventPropertiesBeSentReturnsFalse_DoesNotCallSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    logger.LogEvent(EventProperties{});
    EXPECT_FALSE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogEvent_CanEventPropertiesBeSentReturnsTrue_CallsSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logger.LogEvent(EventProperties{});
    EXPECT_TRUE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogEvent_String_CanEventPropertiesBeSentReturnsFalse_DoesNotCallSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    logger.LogEvent("EventName");
    EXPECT_FALSE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogEvent_String_CanEventPropertiesBeSentReturnsTrue_CallsSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logger.LogEvent("EventName");
    EXPECT_TRUE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogFailure_CanEventPropertiesBeSentReturnsFalse_DoesNotCallSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    logger.LogFailure("Signature", "Detail", "Category", "Id", EventProperties{});
    EXPECT_FALSE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogFailure_CanEventPropertiesBeSentReturnsTrue_CallsSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logger.LogFailure("Signature", "Detail", "Category", "Id", EventProperties{});
    EXPECT_TRUE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogFailure_SignatureAndDetail_CanEventPropertiesBeSentReturnsFalse_DoesNotCallSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    logger.LogFailure("Signature", "Detail", EventProperties{});
    EXPECT_FALSE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogFailure_SignatureAndDetail_CanEventPropertiesBeSentReturnsTrue_CallsSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logger.LogFailure("Signature", "Detail", EventProperties{});
    EXPECT_TRUE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogPageView_CanEventPropertiesBeSentReturnsFalse_DoesNotCallSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    logger.LogPageView("id", "pageName", "category", "uri", "referrer", EventProperties{});
    EXPECT_FALSE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogPageView_CanEventPropertiesBeSentReturnsTrue_CallsSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logger.LogPageView("id", "pageName", "category", "uri", "referrer", EventProperties{});
    EXPECT_TRUE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogPageView_IdPageName_CanEventPropertiesBeSentReturnsFalse_DoesNotCallSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    logger.LogPageView("id", "pageName", "category", "uri", "referrer", EventProperties{});
    EXPECT_FALSE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogPageView_IdPageName_CanEventPropertiesBeSentReturnsTrue_CallsSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logger.LogPageView("id", "pageName", EventProperties{});
    EXPECT_TRUE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogPageAction_PageActionData_CanEventPropertiesBeSentReturnsFalse_DoesNotCallSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    logger.LogPageAction(PageActionData{ "pvId", ActionType::ActionType_Click }, EventProperties{});
    EXPECT_FALSE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogPageAction_PageActionData_CanEventPropertiesBeSentReturnsTrue_CallsSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logger.LogPageAction(PageActionData{ "pvId", ActionType::ActionType_Click }, EventProperties{});
    EXPECT_TRUE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogPageAction_ViewIdActionType_CanEventPropertiesBeSentReturnsFalse_DoesNotCallSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    logger.LogPageAction(PageActionData{ "pvId", ActionType::ActionType_Click }, EventProperties{});
    EXPECT_FALSE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogPageAction_ViewIdActionType_CanEventPropertiesBeSentReturnsTrue_CallsSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logger.LogPageAction("pvId", ActionType::ActionType_Click, EventProperties{});
    EXPECT_TRUE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogSampledMetric_CanEventPropertiesBeSentReturnsFalse_DoesNotCallSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    logger.LogSampledMetric("Name", 3.14, "Units", "InstanceName", "ObjectClass", "ObjectId", EventProperties{});
    EXPECT_FALSE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogSampledMetric_CanEventPropertiesBeSentReturnsTrue_CallsSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logger.LogSampledMetric("Name", 3.14, "Units", "InstanceName", "ObjectClass", "ObjectId", EventProperties{});
    EXPECT_TRUE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogSampledMetric_NameValueUnits_CanEventPropertiesBeSentReturnsFalse_DoesNotCallSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    logger.LogSampledMetric("Name", 3.14, "Units", EventProperties{});
    EXPECT_FALSE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogSampledMetric_NameValueUnits_CanEventPropertiesBeSentReturnsTrue_CallsSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logger.LogSampledMetric("Name", 3.14, "Units", EventProperties{});
    EXPECT_TRUE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogAggregatedMetric_CanEventPropertiesBeSentReturnsFalse_DoesNotCallSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    logger.LogAggregatedMetric(AggregatedMetricData{ "aggName", 0, 0 }, EventProperties{});
    EXPECT_FALSE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogAggregatedMetric_CanEventPropertiesBeSentReturnsTrue_CallsSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logger.LogAggregatedMetric(AggregatedMetricData{"aggName", 0, 0}, EventProperties{});
    EXPECT_TRUE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogTrace_CanEventPropertiesBeSentReturnsFalse_DoesNotCallSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    logger.LogTrace(TraceLevel::TraceLevel_None, "message", EventProperties{});
    EXPECT_FALSE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogTrace_CanEventPropertiesBeSentReturnsTrue_CallsSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logger.LogTrace(TraceLevel::TraceLevel_None, "message", EventProperties{});
    EXPECT_TRUE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogUserState_CanEventPropertiesBeSentReturnsFalse_DoesNotCallSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    logger.LogUserState(UserState::UserState_Unknown, 0, EventProperties{});
    EXPECT_FALSE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogUserState_CanEventPropertiesBeSentReturnsTrue_CallsSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logger.LogUserState(UserState::UserState_Unknown, 0, EventProperties{});
    EXPECT_TRUE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogSession_CanEventPropertiesBeSentReturnsFalse_DoesNotCallSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(false));
    logger.LogSession(SessionState::Session_Started, EventProperties{});
    EXPECT_FALSE(logger.SubmitCalled);
}

TEST_F(LoggerTests, LogSession_CanEventPropertiesBeSentReturnsTrue_CallsSubmit)
{
    logger.GetEventFilters().RegisterEventFilter(MakeTestEventFilter(true));
    logger.LogSession(SessionState::Session_Started, EventProperties{});
    EXPECT_TRUE(logger.SubmitCalled);
}


