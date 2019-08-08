// Copyright (c) Microsoft. All rights reserved.
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
};

class LoggerTests : public ::testing::Test
{
public:
    LoggerTests() noexcept
        : logManager(configuration, nullptr)
        , runtimeConfig(configuration)
        , logger("", "", "", logManager, contextFieldsProvider, runtimeConfig)
    { }

    ILogConfiguration configuration;
    LogManagerImpl logManager;
    ContextFieldsProvider contextFieldsProvider;
    RuntimeConfig_Default runtimeConfig;
    TestLogger logger;

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