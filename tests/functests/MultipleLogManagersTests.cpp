//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"
#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#endif
#include "common/Common.hpp"
#include "common/HttpServer.hpp"

#include "api/LogManagerFactory.hpp"
#include "api/LogManagerImpl.hpp"

#include "CsProtocol_types.hpp"
#include "bond/All.hpp"
#include "bond/generated/CsProtocol_readers.hpp"

#include "sqlite3.h"

#include "NullObjects.hpp"

#if defined __has_include && defined(HAVE_MAT_PRIVACYGUARD)
#if __has_include("modules/privacyguard/PrivacyGuard.hpp")
#include "modules/privacyguard/PrivacyGuard.hpp"
#include <IDataInspector.hpp>
#else
/* Compiling without Privacy Guard*/
#undef HAVE_MAT_PRIVACYGUARD
#endif
#endif
//End Privacy Guard Includes

using namespace testing;
using namespace MAT;

class RequestHandler : public HttpServer::Callback 
{
   public:
    RequestHandler(int id) noexcept : m_id(id){}

    int onHttpRequest(HttpServer::Request const& request, HttpServer::Response& /*response*/) override
    {
        std::string expected_url = "/" + std::to_string(m_id) + "/";
        EXPECT_EQ(request.uri, expected_url);
        m_count++;
        return 200;
    }

    size_t GetRequestCount() const noexcept {
        return m_count;
    }

   private:
    size_t m_count {};
    int m_id ;
};

class MultipleLogManagersTests : public ::testing::Test
{
   protected:
    std::string serverAddress;
    ILogConfiguration config1, config2, config3;
    RequestHandler callback1 = RequestHandler(1);
    RequestHandler callback2 = RequestHandler(2);
    RequestHandler callback3 = RequestHandler(3);

    HttpServer server;

   public:
    virtual void SetUp() override
    {
        int port = server.addListeningPort(0);
        std::ostringstream os;
        os << "localhost:" << port;
        server.setServerName(os.str());
        serverAddress = "http://" + os.str();

        server.addHandler("/1/", callback1);
        server.addHandler("/2/", callback2);
        server.addHandler("/3/", callback3);

        server.start();

#if 0
        sqlite3_initialize();
        config1["skipSqliteInitAndShutdown"] = "true";
        config2["skipSqliteInitAndShutdown"] = "true";
#endif

        // Config for instance #1
        config1["cacheFilePath"] = "lm1.db";
        ::remove(config1["cacheFilePath"]);
        config1[CFG_STR_COLLECTOR_URL] = serverAddress + "/1/";
        config1["name"] = "Instance1";
        config1["version"] = "1.0.0";
        config1["config"]["host"] = "Instance1";

        // Config for instance #2
        config2["cacheFilePath"] = "lm2.db";
        ::remove(config2["cacheFilePath"]);
        config2[CFG_STR_COLLECTOR_URL] = serverAddress + "/2/";
        config2["name"] = "Instance2";
        config2["version"] = "1.0.0";
        config2["config"]["host"] = "Instance2";  // host

        // Config for instance #3
        config3["cacheFilePath"] = "lm3.db";
        ::remove(config3["cacheFilePath"]);
        config3[CFG_STR_COLLECTOR_URL] = serverAddress + "/3/";
        config3["name"] = "Instance3";
        config3["version"] = "1.0.0";
        config3["config"]["host"] = "Instance3";  // host

    }

    virtual void TearDown() override
    {
        sqlite3_shutdown();
        server.stop();
        ::remove(config1["cacheFilePath"]);
        ::remove(config2["cacheFilePath"]);
        ::remove(config3["cacheFilePath"]);

    }


    void waitForRequestsMultipleLogManager(unsigned timeout, unsigned expectedCount1 = 1, unsigned expectedCount2 = 1, unsigned expectedCount3 = 1)
    {
        auto start = PAL::getUtcSystemTimeMs();
        while (callback1.GetRequestCount() < expectedCount1 || callback2.GetRequestCount() < expectedCount2 || callback3.GetRequestCount() < expectedCount3)
        {
            if (PAL::getUtcSystemTimeMs() - start >= timeout)
            {
                GTEST_FATAL_FAILURE_("Didn't receive request within given timeout");
            }
            PAL::sleep(100);
        }
    }

    void waitForRequestsSingleLogManager(unsigned timeout, unsigned expectedCount = 1)
    {
        auto sz = callback1.GetRequestCount();
        auto start = PAL::getUtcSystemTimeMs();
        while (callback1.GetRequestCount()  - sz < expectedCount)
        {
            if (PAL::getUtcSystemTimeMs() - start >= timeout)
            {
                GTEST_FATAL_FAILURE_("Didn't receive request within given timeout");
            }
            PAL::sleep(100);
        }
    }

    /*    CsProtocol::ClientToCollectorRequest decodeRequest(HttpServer::Request const& request)
    {
        std::vector<uint8_t> input(request.content.data(), request.content.data() + request.content.size());
        bond_lite::CompactBinaryProtocolReader reader(input);

        CsProtocol::ClientToCollectorRequest result;
        EXPECT_THAT(bond_lite::Deserialize(reader, result), true);

        return result;
    }
    */
};

TEST_F(MultipleLogManagersTests, ThreeInstancesCoexist)
{
    std::unique_ptr<ILogManager> lm1(LogManagerFactory::Create(config1));
    std::unique_ptr<ILogManager> lm2(LogManagerFactory::Create(config2));
    std::unique_ptr<ILogManager> lm3(LogManagerFactory::Create(config3));

    lm1->SetContext("test1", "abc");
    lm2->GetSemanticContext().SetAppId("123");
    
    auto l1 = lm1->GetLogger("lm1_token1", "aaa-source");
    auto l2 = lm2->GetLogger("lm2_token1", "bbb-source");
    auto l3 = lm3->GetLogger("lm3_token1", "ccc-source");

    EventProperties l1_prop("l1a1");
    l1_prop.SetProperty("X", "Y");
    l1->LogEvent(l1_prop);

    EventProperties l2_prop("l2a1");
    l2_prop.SetProperty("x", "y");
    l2->LogEvent(l2_prop);

    EventProperties l3_prop("l3a1");
    l3_prop.SetProperty("test", 1234);
    l3->LogEvent(l3_prop);

    lm1->GetLogController()->UploadNow();
    lm2->GetLogController()->UploadNow();
    lm3->GetLogController()->UploadNow();

    waitForRequestsMultipleLogManager(10000, 1, 1, 1);

    lm1.reset();
    lm2.reset();
    lm3.reset();
}

constexpr static unsigned max_iterations = 2000;

TEST_F(MultipleLogManagersTests, MultiProcessesLogManager)
{
    CAPTURE_PERF_STATS("start");
    config1[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Warn;
    config1[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;  // 80kb
    config1[CFG_STR_CACHE_FILE_PATH] = testing::GetUniqueDBFileName();
    std::unique_ptr<ILogManager> lm(LogManagerFactory::Create(config1));
    CAPTURE_PERF_STATS("LogManager created");
    auto logger = lm->GetLogger("aaa");
    CAPTURE_PERF_STATS("Logger created");
    size_t numIterations = max_iterations;
    while (numIterations--)
    {
        EventProperties props = CreateSampleEvent("event_name", EventPriority_Normal);
        logger->LogEvent(props);
    }
    CAPTURE_PERF_STATS("Events Sent");
    lm->GetLogController()->UploadNow();
    CAPTURE_PERF_STATS("Events Uploaded");
    waitForRequestsSingleLogManager(10000, 2);
    lm.reset();
    CAPTURE_PERF_STATS("Log Manager deleted");
}

#ifdef HAVE_MAT_PRIVACYGUARD
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

TEST_F(MultipleLogManagersTests, PrivacyGuardSharedWithTwoInstancesCoexist)
{
    MockLogger mockLogger;
    auto privacyConcernLogCount = 0;
    InitializationConfiguration config(&mockLogger, CommonDataContext {});
    config.ScanForUrls = true;
    const auto privacyGuard = std::make_shared<PrivacyGuard>(config);
    mockLogger.m_logEventOverride = [&privacyConcernLogCount, &privacyGuard](const EventProperties& properties) {
        if (equalsIgnoreCase(properties.GetName(), privacyGuard->GetNotificationEventName()))
        {
            privacyConcernLogCount++;
        }
    };

    std::unique_ptr<ILogManager> lm1(LogManagerFactory::Create(config1));
    std::unique_ptr<ILogManager> lm2(LogManagerFactory::Create(config2));

    lm1->SetDataInspector(privacyGuard);
    lm2->SetDataInspector(privacyGuard);

    auto l2a = lm2->GetLogger("aaa", "aaa-source");
    EventProperties l2a1p("l2a1");
    l2a1p.SetProperty("Field1", "http://www.microsoft.com");                             //DataConcernType::Url
    l2a1p.SetProperty("Field2", "HTTPS://www.microsoft.com");                            //DataConcernType::Url
    l2a1p.SetProperty("Field3", "File://www.microsoft.com");                             //DataConcernType::Url & DataConcernType::FileSharingUrl
    l2a1p.SetProperty("Field4", "Download failed for domain https://wopi.dropbox.com");  //DataConcernType::Url
    l2a->LogEvent(l2a1p);
    ASSERT_EQ(5, privacyConcernLogCount);

    privacyConcernLogCount = 0;

    auto l1a = lm1->GetLogger("aaa");
    EventProperties l1a1p("l1a1");
    l1a1p.SetProperty("Field1", "Some%2eone%40Microsoft%2ecom");     //ConcernType::InternalEmailAddress  //As happens in escaped URLs
    l1a1p.SetProperty("Field2", "Someone@Microsoft.com");            //ConcernType::InternalEmailAddress
    l1a1p.SetProperty("Field3", "Some.one@Exchange.Microsoft.com");  //ConcernType::InternalEmailAddress
    l1a1p.SetProperty("Field4", "Some_one@microsoft_com");           //ConcernType::InternalEmailAddress
    l1a1p.SetProperty("Field5", "Some_one_AT_microsoft_com");        //ConcernType::InternalEmailAddress
    l1a1p.SetProperty("Field6", "Microsoft.com");
    l1a1p.SetProperty("Field7", "Exchange.Microsoft.com");
    l1a1p.SetProperty("Field8", "Some_one");
    l1a->LogEvent(l1a1p);
    ASSERT_EQ(5, privacyConcernLogCount);

    lm1.reset();
    lm2.reset();
}
#endif  //END HAVE_MAT_PRIVACYGUARD

#endif  // HAVE_MAT_DEFAULT_HTTP_CLIENT

