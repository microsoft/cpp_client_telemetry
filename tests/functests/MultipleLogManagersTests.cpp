// Copyright (c) Microsoft. All rights reserved.

#include "Common/Common.hpp"
#include "common/HttpServer.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include <ILogManager.hpp>
#include <bond_lite/All.hpp>
#include "AriaProtocol_types.hpp"
#include "AriaProtocol_readers.hpp"
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
    MockIRuntimeConfig             runtimeConfig1, runtimeConfig2;
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
        expectRuntimeConfig(runtimeConfig1, serverAddress + "/1/");
        expectRuntimeConfig(runtimeConfig2, serverAddress + "/2/");

        server.start();

        sqlite3_initialize();
        config1.skipSqliteInitAndShutdown = true;
        config2.skipSqliteInitAndShutdown = true;

        config1.cacheFilePath = "lm1.db";
        config1.runtimeConfig = &runtimeConfig1;
        ::remove(config1.cacheFilePath.c_str());

        config2.cacheFilePath = "lm2.db";
        config2.runtimeConfig = &runtimeConfig2;
        ::remove(config2.cacheFilePath.c_str());
    }

    virtual void TearDown() override
    {
        sqlite3_shutdown();
        server.stop();
        ::remove(config1.cacheFilePath.c_str());
        ::remove(config2.cacheFilePath.c_str());
    }

    void expectRuntimeConfig(MockIRuntimeConfig& rc, std::string const& url)
    {
        EXPECT_CALL(rc, SetDefaultConfig(_)).WillRepeatedly(DoDefault());
        EXPECT_CALL(rc, GetCollectorUrl()).WillRepeatedly(Return(url));
        EXPECT_CALL(rc, IsHttpRequestCompressionEnabled()).WillRepeatedly(Return(false));
        EXPECT_CALL(rc, GetOfflineStorageMaximumSizeBytes()).WillRepeatedly(Return(UINT_MAX));
        EXPECT_CALL(rc, GetEventPriority(_, _)).WillRepeatedly(Return(EventPriority_Unspecified));
        EXPECT_CALL(rc, GetMetaStatsSendIntervalSec()).WillRepeatedly(Return(0));
        EXPECT_CALL(rc, GetMetaStatsTenantToken()).WillRepeatedly(Return("metastats-token"));
        EXPECT_CALL(rc, GetMaximumRetryCount()).WillRepeatedly(Return(1));
        EXPECT_CALL(rc, GetMinimumUploadBandwidthBps()).WillRepeatedly(Return(512));
        EXPECT_CALL(rc, GetMaximumUploadSizeBytes()).WillRepeatedly(Return(1 * 1024 * 1024));
    }

    virtual int onHttpRequest(HttpServer::Request const& request, HttpServer::Response& response) override
    {
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

    AriaProtocol::ClientToCollectorRequest decodeRequest(HttpServer::Request const& request)
    {
        std::vector<uint8_t> input(request.content.data(), request.content.data() + request.content.size());
        bond_lite::CompactBinaryProtocolReader reader(input);

        AriaProtocol::ClientToCollectorRequest result;
        EXPECT_THAT(bond_lite::Deserialize(reader, result), true);

        return result;
    }
};


TEST_F(MultipleLogManagersTests, TwoInstancesCoexist)
{
    std::unique_ptr<ILogManager> lm1(ILogManager::Create(config1));

    std::unique_ptr<ILogManager> lm2(ILogManager::Create(config2));

    lm1->SetContext("test1", "abc");

    lm2->GetSemanticContext().SetAppId("123");

    ILogger* l1a = lm1->GetLogger("aaa");

    ILogger* l2a = lm2->GetLogger("aaa", "aaa-source");
    EventProperties l2a1p("l2a1");
    l2a1p.SetProperty("x", "y");
    EXPECT_CALL(runtimeConfig2, DecorateEvent(_, _, _)).WillOnce(Return());
    l2a->LogEvent(l2a1p);

    EventProperties l1a1p("l1a1");
    l1a1p.SetProperty("X", "Y");
    EXPECT_CALL(runtimeConfig1, DecorateEvent(_, _, _)).WillOnce(Return());
    l1a->LogEvent(l1a1p);

    ILogger* l1b = lm1->GetLogger("bbb");
    EventProperties l1b1p("l1b1");
    l1b1p.SetProperty("asdf", 1234);
    EXPECT_CALL(runtimeConfig1, DecorateEvent(_, _, _)).WillOnce(Return());
    l1b->LogEvent(l1b1p);

    waitForRequests(5000, 2);

    // Add more tests

    lm1.reset();

    lm2.reset();
}
