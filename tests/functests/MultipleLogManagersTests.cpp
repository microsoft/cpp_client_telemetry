//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
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

class MultipleLogManagersTests : public ::testing::Test,
                                 public HttpServer::Callback
{
   protected:
    std::list<HttpServer::Request> receivedRequests;
    std::string serverAddress;
    ILogConfiguration config1, config2;
    HttpServer server;

   public:
    virtual void SetUp() override
    {
        int port = server.addListeningPort(0);
        std::ostringstream os;
        os << "localhost:" << port;
        server.setServerName(os.str());
        serverAddress = "http://" + os.str();

        server.addHandler("/1/", *this);
        server.addHandler("/2/", *this);

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
        config1["name"] = "Instance2";
        config1["version"] = "1.0.0";
        config1["config"]["host"] = "Instance2";  // host
    }

    virtual void TearDown() override
    {
        sqlite3_shutdown();
        server.stop();
        ::remove(config1["cacheFilePath"]);
        ::remove(config2["cacheFilePath"]);
    }

    virtual int onHttpRequest(HttpServer::Request const& request, HttpServer::Response& response) override
    {
        UNREFERENCED_PARAMETER(response);
        receivedRequests.push_back(request);
        return 200;
    }

    void waitForRequests(unsigned timeout, unsigned expectedCount = 1)
    {
        auto sz = receivedRequests.size();
        auto start = PAL::getUtcSystemTimeMs();
        while (receivedRequests.size() - sz < expectedCount)
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

TEST_F(MultipleLogManagersTests, TwoInstancesCoexist)
{
    std::unique_ptr<ILogManager> lm1(LogManagerFactory::Create(config1));
    std::unique_ptr<ILogManager> lm2(LogManagerFactory::Create(config2));

    lm1->SetContext("test1", "abc");

    lm2->GetSemanticContext().SetAppId("123");

    ILogger* l1a = lm1->GetLogger("aaa");

    ILogger* l2a = lm2->GetLogger("aaa", "aaa-source");
    EventProperties l2a1p("l2a1");
    l2a1p.SetProperty("x", "y");
    l2a->LogEvent(l2a1p);

    EventProperties l1a1p("l1a1");
    l1a1p.SetProperty("X", "Y");
    l1a->LogEvent(l1a1p);

    ILogger* l1b = lm1->GetLogger("bbb");
    EventProperties l1b1p("l1b1");
    l1b1p.SetProperty("asdf", 1234);
    l1b->LogEvent(l1b1p);

    lm1->GetLogController()->UploadNow();
    lm2->GetLogController()->UploadNow();

    waitForRequests(5000, 2);

    // Add more tests

    lm1.reset();
    lm2.reset();
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
    ILogger* logger = lm->GetLogger("aaa");
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
    waitForRequests(10000, 2);
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
    InitializationConfiguration config;
    config.LoggerInstance = &mockLogger;
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

    ILogger* l2a = lm2->GetLogger("aaa", "aaa-source");
    EventProperties l2a1p("l2a1");
    l2a1p.SetProperty("Field1", "http://www.microsoft.com");                             //DataConcernType::Url
    l2a1p.SetProperty("Field2", "HTTPS://www.microsoft.com");                            //DataConcernType::Url
    l2a1p.SetProperty("Field3", "File://www.microsoft.com");                             //DataConcernType::Url & DataConcernType::FileSharingUrl
    l2a1p.SetProperty("Field4", "Download failed for domain https://wopi.dropbox.com");  //DataConcernType::Url
    l2a->LogEvent(l2a1p);
    ASSERT_EQ(5, privacyConcernLogCount);

    privacyConcernLogCount = 0;

    ILogger* l1a = lm1->GetLogger("aaa");
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

