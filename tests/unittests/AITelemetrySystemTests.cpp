// Copyright (c) Microsoft. All rights reserved .

#if defined __has_include
#  if __has_include ("modules/azmon/AITelemetrySystem.hpp")
#    include "modules/azmon/AITelemetrySystem.hpp"
#  else
   /* Compiling without Azure Monitor */
#  undef HAVE_MAT_AI
#  endif
#endif

#ifdef HAVE_MAT_AI

#include "common/Common.hpp"
#include "common/MockIOfflineStorage.hpp"
#include "common/MockIHttpClient.hpp"
#include "common/MockILogManagerInternal.hpp"
#include "config/RuntimeConfig_Default.hpp"

using namespace testing;

class AITelemetrySystemTests : public StrictMock<Test>
{
   protected:
    ILogConfiguration logConfig;
    RuntimeConfig_Default config;
    LogManagerImpl logManagerMock;
    MockIOfflineStorage offlineStorageMock;
    MockIHttpClient httpClientMock;
    std::shared_ptr<ITaskDispatcher> taskDispatcher;
    std::unique_ptr<LogSessionDataProvider> logSessionDataProvider;
    std::unique_ptr<ITelemetrySystem> system;

   protected:
    AITelemetrySystemTests() :
        config(logConfig),
        logManagerMock(logConfig, static_cast<bool>(nullptr))
    {
        taskDispatcher = PAL::getDefaultTaskDispatcher();
        logSessionDataProvider.reset(new LogSessionDataProvider(&offlineStorageMock));
    }
};

TEST_F(AITelemetrySystemTests, testDefaultConfiguration)
{
    config[CFG_MAP_METASTATS_CONFIG][CFG_INT_METASTATS_INTERVAL] = 30;
    config[CFG_MAP_HTTP]["contentEncoding"] = "deflate";
    EXPECT_EQ(0, strcmp(COLLECTOR_URL_PROD, config[CFG_STR_COLLECTOR_URL]));

    system.reset(new AITelemetrySystem(
        logManagerMock, config, offlineStorageMock, httpClientMock,
        *taskDispatcher, nullptr, *logSessionDataProvider
    ));

    unsigned int stats = config[CFG_MAP_METASTATS_CONFIG][CFG_INT_METASTATS_INTERVAL];
    EXPECT_EQ(0u, stats);
    EXPECT_EQ(0, strcmp("gzip", config[CFG_MAP_HTTP]["contentEncoding"]));
    EXPECT_EQ(0, strcmp("https://dc.services.visualstudio.com/v2/track", config[CFG_STR_COLLECTOR_URL]));
}

TEST_F(AITelemetrySystemTests, testUrlOverride)
{
    config[CFG_STR_COLLECTOR_URL] = "http://localhost";

    system.reset(new AITelemetrySystem(
        logManagerMock, config, offlineStorageMock, httpClientMock,
        *taskDispatcher, nullptr, *logSessionDataProvider));

    EXPECT_EQ(0, strcmp("http://localhost", config[CFG_STR_COLLECTOR_URL]));
}

#endif // HAVE_MAT_AI