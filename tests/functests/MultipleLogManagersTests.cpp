// Copyright (c) Microsoft. All rights reserved.
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include "Common/Common.hpp"
#include "common/HttpServer.hpp"
#include <api/ILogManagerInternal.hpp>
#include <bond_lite/All.hpp>
#include "bond/generated/AriaProtocol_types.hpp"
#include "bond/generated/AriaProtocol_readers.hpp"
#include "../../sqlite/sqlite3.h"

using namespace testing;
using namespace ARIASDK_NS;


class MultipleLogManagersTests : public ::testing::Test,
                                 public HttpServer::Callback
{
  protected:
    std::list<HttpServer::Request> receivedRequests;
    std::string                    serverAddress;
    LogConfiguration               config1, config2;
    HttpServer                     server;

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
        // expectRuntimeConfig(runtimeConfig1, serverAddress + "/1/");
        // expectRuntimeConfig(runtimeConfig2, serverAddress + "/2/");

        server.start();

        sqlite3_initialize();
        config1.SetProperty("skipSqliteInitAndShutdown", "true");
        config2.SetProperty("skipSqliteInitAndShutdown", "true"); 

        config1.SetProperty("cacheFilePath","lm1.db");
        //config1.runtimeConfig = &runtimeConfig1;
        EVTStatus error;
        ::remove(config1.GetProperty("cacheFilePath", error));

        config2.SetProperty("cacheFilePath", "lm2.db");
        //config2.runtimeConfig = &runtimeConfig2;

        ::remove(config2.GetProperty("cacheFilePath", error));
    }

    virtual void TearDown() override
    {
        sqlite3_shutdown();
        server.stop();
        EVTStatus error;
        ::remove(config1.GetProperty("cacheFilePath", error));
        ::remove(config2.GetProperty("cacheFilePath", error));
    }

    void expectRuntimeConfig(MockIRuntimeConfig& rc, std::string const& url)
    {
        EXPECT_CALL(rc, SetDefaultConfig(_)).WillRepeatedly(DoDefault());
        EXPECT_CALL(rc, GetCollectorUrl()).WillRepeatedly(Return(url));
        EXPECT_CALL(rc, IsHttpRequestCompressionEnabled()).WillRepeatedly(Return(false));
        EXPECT_CALL(rc, GetOfflineStorageMaximumSizeBytes()).WillRepeatedly(Return(UINT_MAX));
        EXPECT_CALL(rc, GetEventLatency(_, _)).WillRepeatedly(Return(EventLatency_Unspecified));
        EXPECT_CALL(rc, GetMetaStatsSendIntervalSec()).WillRepeatedly(Return(0));
        EXPECT_CALL(rc, GetMetaStatsTenantToken()).WillRepeatedly(Return("metastats-token"));
        EXPECT_CALL(rc, GetMaximumRetryCount()).WillRepeatedly(Return(1));
        EXPECT_CALL(rc, GetMinimumUploadBandwidthBps()).WillRepeatedly(Return(512));
        EXPECT_CALL(rc, GetMaximumUploadSizeBytes()).WillRepeatedly(Return(1 * 1024 * 1024));
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
        while (receivedRequests.size() - sz < expectedCount) {
            if (PAL::getUtcSystemTimeMs() - start >= timeout) {
                GTEST_FATAL_FAILURE_("Didn't receive request within given timeout");
            }
            PAL::sleep(100);
        }
    }

/*    AriaProtocol::ClientToCollectorRequest decodeRequest(HttpServer::Request const& request)
    {
        std::vector<uint8_t> input(request.content.data(), request.content.data() + request.content.size());
        bond_lite::CompactBinaryProtocolReader reader(input);

        AriaProtocol::ClientToCollectorRequest result;
        EXPECT_THAT(bond_lite::Deserialize(reader, result), true);

        return result;
    }
    */
};


TEST_F(MultipleLogManagersTests, TwoInstancesCoexist)
{
    std::unique_ptr<ILogManagerInternal> lm1(ILogManagerInternal::Create(config1, &runtimeConfig1));

    std::unique_ptr<ILogManagerInternal> lm2(ILogManagerInternal::Create(config2, &runtimeConfig2));

    lm1->SetContext("test1", "abc");

    lm2->GetSemanticContext().SetAppId("123");
    ContextFieldsProvider temp;
    ILogger* l1a = lm1->GetLogger("aaa", &temp);

    ILogger* l2a = lm2->GetLogger("aaa", &temp, "aaa-source");
    EventProperties l2a1p("l2a1");
    l2a1p.SetProperty("x", "y");
    l2a->LogEvent(l2a1p);

    EventProperties l1a1p("l1a1");
    l1a1p.SetProperty("X", "Y");
    l1a->LogEvent(l1a1p);

    ILogger* l1b = lm1->GetLogger("bbb", &temp);
    EventProperties l1b1p("l1b1");
    l1b1p.SetProperty("asdf", 1234);
    l1b->LogEvent(l1b1p);

    waitForRequests(5000, 2);

    // Add more tests

    lm1.reset();

    lm2.reset();
}
