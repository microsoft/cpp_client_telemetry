// Copyright (c) Microsoft. All rights reserved.
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include "common/Common.hpp"
#include "common/HttpServer.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include "utils/Utils.hpp"
#include <ILogManager.hpp>
#include <bond_lite/All.hpp>
#include "bond/generated/AriaProtocol_types.hpp"
#include "bond/generated/AriaProtocol_readers.hpp"
#include <fstream>

using namespace testing;
using namespace ARIASDK_NS;

char const* const TEST_STORAGE_FILENAME = "BasicFuncTests.db";


class BasicFuncTests : public ::testing::Test,
                       public HttpServer::Callback
{
  protected:
    std::vector<HttpServer::Request> receivedRequests;
    std::string serverAddress;
    MockIRuntimeConfig runtimeConfig;
    HttpServer server;
    std::unique_ptr<ILogManager> logManager;
    ILogger* logger;
    ILogger* logger2;
    LogConfiguration configuration;

  public:
    virtual void SetUp() override
    {
        int port = server.addListeningPort(0);
        std::ostringstream os;
        os << "localhost:" << port;
        serverAddress = "http://" + os.str() + "/simple/";
        server.setServerName(os.str());
        server.addHandler("/simple/", *this);
        server.addHandler("/slow/", *this);
        server.addHandler("/503/", *this);
                
        configuration.SetIntProperty(CFG_INT_RAM_QUEUE_SIZE, 4096 * 20);
        configuration.SetProperty(CFG_STR_CACHE_FILE_PATH, TEST_STORAGE_FILENAME);
        bool error;
        ::remove(configuration.GetProperty(CFG_STR_CACHE_FILE_PATH, error));

        EXPECT_CALL(runtimeConfig, SetDefaultConfig(_)).WillRepeatedly(DoDefault());
        EXPECT_CALL(runtimeConfig, GetCollectorUrl()).WillRepeatedly(Return(serverAddress));
        EXPECT_CALL(runtimeConfig, IsHttpRequestCompressionEnabled()).WillRepeatedly(Return(false));
        EXPECT_CALL(runtimeConfig, GetOfflineStorageMaximumSizeBytes()).WillRepeatedly(Return(UINT_MAX));
        EXPECT_CALL(runtimeConfig, GetEventLatency(_, _)).WillRepeatedly(Return(EventLatency_Unspecified));
        EXPECT_CALL(runtimeConfig, GetMetaStatsSendIntervalSec()).WillRepeatedly(Return(0));
        EXPECT_CALL(runtimeConfig, GetMetaStatsTenantToken()).WillRepeatedly(Return("metastats-tenant-token"));
        EXPECT_CALL(runtimeConfig, GetMaximumRetryCount()).WillRepeatedly(Return(1));
        EXPECT_CALL(runtimeConfig, GetUploadRetryBackoffConfig()).WillRepeatedly(Return("E,3000,3000,2,0"));
        EXPECT_CALL(runtimeConfig, GetMinimumUploadBandwidthBps()).WillRepeatedly(Return(512));
        EXPECT_CALL(runtimeConfig, GetMaximumUploadSizeBytes()).WillRepeatedly(Return(1 * 1024 * 1024));

        logManager.reset(ILogManager::Create(configuration, &runtimeConfig));
        logger = logManager->GetLogger("functests-Tenant-Token", "source");
        logger2 = logManager->GetLogger("FuncTests2-tenant-token", "Source");

        server.start();
    }

    virtual void TearDown() override
    {
        logManager->FlushAndTeardown();
        ::remove(TEST_STORAGE_FILENAME);
        server.stop();
    }

    virtual int onHttpRequest(HttpServer::Request const& request, HttpServer::Response& response) override
    {
        if (request.uri.compare(0, 5, "/503/") == 0) {
            return 503;
        }

        if (request.uri.compare(0, 6, "/slow/") == 0) {
            PAL::sleep(static_cast<unsigned int>(request.content.size() / DELAY_FACTOR_FOR_SERVER));
        }

        receivedRequests.push_back(request);

        response.headers["Content-Type"] = "text/plain";
        response.content = "It works!";

        return 200;
    }

    bool waitForRequests(unsigned timeout, unsigned expected_count = 1)
    {
        auto sz = receivedRequests.size();
        auto start = PAL::getUtcSystemTimeMs();
        while (receivedRequests.size() - sz < expected_count)
        {
            if (PAL::getUtcSystemTimeMs() - start >= timeout * 1000)
            {
                return false;
            }
            PAL::sleep(100);
        }
        return true;
    }

    void waitForEvents(unsigned timeout, unsigned expected_count = 1)
    {
        unsigned receivedEvnets = 0;
        auto start = PAL::getUtcSystemTimeMs();
        while (receivedEvnets < expected_count)
        {
            unsigned receivedEvnetsLocal = 0;
            if (waitForRequests(timeout))
            {
                for (auto request : receivedRequests)
                {
                    auto payload = decodeRequest(request, false);
                    receivedEvnetsLocal = receivedEvnetsLocal + (unsigned)payload.size();
                }
                receivedEvnets = receivedEvnetsLocal;

                if (PAL::getUtcSystemTimeMs() - start >= timeout * 1000)
                {
                    GTEST_FATAL_FAILURE_("Didn't receive records within given timeout");
                }
                PAL::sleep(100);
            }
            else
            {
                GTEST_FATAL_FAILURE_("Didn't receive request within given timeout");
            }
        }
    }  

    std::vector<AriaProtocol::CsEvent> decodeRequest(HttpServer::Request const& request, bool decompress)
    {
        std::vector<AriaProtocol::CsEvent> vector;

        if (decompress) {
            // TODO
        }
                        
        size_t data = 0;
        size_t length= 0 ;
        while (data < request.content.size())
        {
            AriaProtocol::CsEvent result;
            length = request.content.size() - data;
            std::vector<uint8_t> test(request.content.data() + data, request.content.data() + data + length);
            size_t index = 3;
            bool found = false;
            while (index < length)
            {
                while (index < length && test[index] != '\x3')
                {
                    index++;
                }

                if (index < length)
                {
                    if (test[index + 1] == '3' && test[index + 2] == '.')
                    {
                        found = true;
                        break;
                    }
                    index++;
                }                
            }
            if (!found)
            {
                index += 1;
            }
            std::vector<uint8_t> input(request.content.data() + data, request.content.data() + data + index - 1);

            bond_lite::CompactBinaryProtocolReader reader(input);
            EXPECT_THAT(bond_lite::Deserialize(reader, result), true);
            data += index - 1;
            vector.push_back(result);

        }

        return vector;
    }

    void verifyEvent(EventProperties const& expected, ::AriaProtocol::CsEvent const& actual)
    {
        EXPECT_THAT(actual.name, Not(IsEmpty()));
        int64_t now = PAL::getUtcSystemTimeMs();
        EXPECT_THAT(actual.time, Gt(now - 60000));
        EXPECT_THAT(actual.time, Le(now));
        EXPECT_THAT(actual.baseType, expected.GetName());
        for (std::pair<std::string, EventProperty>  prop : expected.GetProperties())
        {
            if (prop.second.piiKind == PiiKind_None &&
                prop.second.ccKind == CustomerContentKind_None)
            {
                //EXPECT_THAT(actual.data[0].properties[prop.first].stringValue, prop.second.to_string());
            }
        }
        for (auto const& property : expected.GetPiiProperties())
        {
            ::AriaProtocol::PII pii;
            pii.Kind       = static_cast< ::AriaProtocol::PIIKind>(property.second.second);
           // EXPECT_THAT(actual.PIIExtensions, Contains(Pair(property.first, pii)));
        }

        for (auto const& property : expected.GetCustomerContentProperties())
        {
            ::AriaProtocol::CustomerContent cc;
            cc.Kind = static_cast< ::AriaProtocol::CustomerContentKind>(property.second.second);
            //EXPECT_THAT(actual.CustomerContentExtensions, Contains(Pair(property.first, cc)));
        }
    }

    int64_t getFileSize(std::string const& filename)
    {
        std::ifstream ifile(filename);
        ifile.seekg(0, std::ios_base::end);
        return ifile.tellg();
    }
};


TEST_F(BasicFuncTests, doNothing)
{
}

TEST_F(BasicFuncTests, sendOneEvent_immediatelyStop)
{
    EventProperties event("first_event");
    event.SetProperty("property", "value");

    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).WillOnce(Return());
    logger->LogEvent(event);
}

TEST_F(BasicFuncTests, sendNoPriorityEvents)
{
    EventProperties event("first_event");
    event.SetProperty("property", "value");

    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).WillOnce(Return());
    logger->LogEvent(event);

    EventProperties event2("second_event");
    event2.SetProperty("property", "value2");
    event2.SetProperty("property2", "another value");

    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).WillOnce(Return());
    logger->LogEvent(event2);

    waitForEvents(50, 3);
    auto payload = decodeRequest(receivedRequests[receivedRequests.size() - 1], false);

    //ASSERT_THAT(payload.TokenToDataPackagesMap, Contains(Key("functests-tenant-token")));
    //ASSERT_THAT(payload.TokenToDataPackagesMap["functests-tenant-token"], SizeIs(1));
    //auto const& dp = payload.TokenToDataPackagesMap["functests-tenant-token"][0];
    //ASSERT_THAT(dp.Records, SizeIs(2));
    //verifyEvent(event, dp.Records[0]);
    //verifyEvent(event2, dp.Records[1]);
}

TEST_F(BasicFuncTests, sendSamePriorityNormalEvents)
{
    EventProperties event("first_event");
    event.SetPriority(EventPriority_Normal);
    event.SetProperty("property", "value");

    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).WillOnce(Return());
    logger->LogEvent(event);

    EventProperties event2("second_event");
    event2.SetPriority(EventPriority_Normal);
    event2.SetProperty("property", "value2");
    event2.SetProperty("property2", "another value");
    event2.SetProperty("pii_property", "pii_value", PiiKind_Identity);
    event2.SetProperty("cc_property", "cc_value", CustomerContentKind_GenericData);

    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).WillOnce(Return());
    logger->LogEvent(event2);

    waitForEvents(50, 3);
    auto payload = decodeRequest(receivedRequests[receivedRequests.size() - 1], false);
/*
    ASSERT_THAT(payload.TokenToDataPackagesMap, Contains(Key("functests-tenant-token")));
    ASSERT_THAT(payload.TokenToDataPackagesMap["functests-tenant-token"], SizeIs(1));
    auto const& dp = payload.TokenToDataPackagesMap["functests-tenant-token"][0];
    ASSERT_THAT(dp.Records, SizeIs(2));
    verifyEvent(event, dp.Records[0]);
    verifyEvent(event2, dp.Records[1]);
    */
}

TEST_F(BasicFuncTests, sendDifferentPriorityEvents)
{
    EventProperties event("first_event");
    event.SetPriority(EventPriority_Normal);
    event.SetProperty("property", "value");

    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).WillOnce(Return());
    logger->LogEvent(event);

    EventProperties event2("second_event");
    event2.SetPriority(EventPriority_High);
    event2.SetProperty("property", "value2");
    event2.SetProperty("property2", "another value");
    event2.SetProperty("pii_property", "pii_value", PiiKind_Identity);
    event2.SetProperty("cc_property", "cc_value", CustomerContentKind_GenericData);

    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).WillOnce(Return());
    logger->LogEvent(event2);

    PAL::sleep(100); // Some time to let events be saved to DB
    waitForEvents(50, 3);
    auto payload = decodeRequest(receivedRequests[receivedRequests.size() - 1], false);

 //   ASSERT_THAT(payload1.TokenToDataPackagesMap, Contains(Key("functests-tenant-token")));
//    ASSERT_THAT(payload1.TokenToDataPackagesMap["functests-tenant-token"], SizeIs(1));
//    auto const& dp1 = payload1.TokenToDataPackagesMap["functests-tenant-token"][0];
//    ASSERT_THAT(dp1.Records, SizeIs(2));
//    verifyEvent(event2, dp1.Records[0]);
//	verifyEvent(event, dp1.Records[1]);

	/*
    auto payload2 = decodeRequest(receivedRequests[1], false);
    ASSERT_THAT(payload2.TokenToDataPackagesMap, Contains(Key("functests-tenant-token")));
    ASSERT_THAT(payload2.TokenToDataPackagesMap["functests-tenant-token"], SizeIs(1));
    auto const& dp2 = payload2.TokenToDataPackagesMap["functests-tenant-token"][0];
    ASSERT_THAT(dp2.Records, SizeIs(1));
    verifyEvent(event, dp2.Records[0]);
	*/
}

TEST_F(BasicFuncTests, sendMultipleTenantsTogether)
{
    EventProperties event1("first_event");
    event1.SetProperty("property", "value");

    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).WillOnce(Return());
    logger->LogEvent(event1);

    EventProperties event2("second_event");
    event2.SetProperty("property", "value2");
    event2.SetProperty("property2", "another value");

    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).WillOnce(Return());
    logger2->LogEvent(event2);

    waitForEvents(50, 3);
    auto payload = decodeRequest(receivedRequests[receivedRequests.size() - 1], false);

//    ASSERT_THAT(payload.TokenToDataPackagesMap, Contains(Key("functests-tenant-token")));
//    ASSERT_THAT(payload.TokenToDataPackagesMap["functests-tenant-token"], SizeIs(1));
//    auto const& dp1 = payload.TokenToDataPackagesMap["functests-tenant-token"][0];
//    ASSERT_THAT(dp1.Records, SizeIs(1));
//    verifyEvent(event1, dp1.Records[0]);

//    ASSERT_THAT(payload.TokenToDataPackagesMap, Contains(Key("functests2-tenant-token")));
//    ASSERT_THAT(payload.TokenToDataPackagesMap["functests2-tenant-token"], SizeIs(1));
 //   auto const& dp2 = payload.TokenToDataPackagesMap["functests2-tenant-token"][0];
 //   ASSERT_THAT(dp2.Records, SizeIs(1));
//    verifyEvent(event2, dp2.Records[0]);
}

TEST_F(BasicFuncTests, configDecorations)
{
    EventProperties event("first_event");
    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).WillOnce(Return());
    logger->LogEvent(event);

    EventProperties event2("second_event");
    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).WillOnce(Invoke([](std::map<std::string, std::string>& ext, std::string const&, std::string const&) {
        ext["AppInfo.Decoration"] = "123";
    }));
    logger->LogEvent(event2);

    EventProperties event3("third_event");
    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).WillOnce(Invoke([](std::map<std::string, std::string>& ext, std::string const&, std::string const& name) {
        ext["AppInfo.Decoration"] = "abcdef";
        ext["AppInfo.EventName"]  = name;
    }));
    logger->LogEvent(event3);

    EventProperties event4("");
    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).WillOnce(Invoke([](std::map<std::string, std::string>& ext, std::string const&, std::string const& name) {
        ext["AppInfo.EventName"] = name;
    }));
    logger->LogAppLifecycle(AppLifecycleState_Launch, event4);

    PAL::sleep(100);
    waitForEvents(50, 5);
    auto payload = decodeRequest(receivedRequests[receivedRequests.size() - 1], false);

/*    ASSERT_THAT(payload.TokenToDataPackagesMap, Contains(Key("functests-tenant-token")));
//    ASSERT_THAT(payload.TokenToDataPackagesMap["functests-tenant-token"], SizeIs(1));
//    auto const& dp = payload.TokenToDataPackagesMap["functests-tenant-token"][0];
    ASSERT_THAT(dp.Records, SizeIs(4));
    EXPECT_THAT(dp.Records[0].Extension, Not(Contains(Key("AppInfo.Decoration"))));
    EXPECT_THAT(dp.Records[1].Extension, Contains(Pair("AppInfo.Decoration", "123")));
    EXPECT_THAT(dp.Records[2].Extension, Contains(Pair("AppInfo.Decoration", "abcdef")));
    EXPECT_THAT(dp.Records[2].Extension, Contains(Pair("AppInfo.EventName",  "third_event")));
    EXPECT_THAT(dp.Records[3].Extension, Contains(Pair("AppInfo.EventName",  "AppLifecycle")));
*/
}

TEST_F(BasicFuncTests, restartRecoversEventsFromStorage)
{
    // Wait a bit so that the initial check for unsent events does not send our events too early.
    PAL::sleep(200);

    EventProperties event1("first_event");
    EventProperties event2("second_event");
    
    event1.SetProperty("property1", "value1");
    event2.SetProperty("property2", "value2");
    event1.SetLatency(Microsoft::Applications::Telemetry::EventLatency::EventLatency_RealTime);
    event1.SetPersistence(Microsoft::Applications::Telemetry::EventPersistence::EventPersistence_Critical);
    event2.SetLatency(Microsoft::Applications::Telemetry::EventLatency::EventLatency_RealTime);
    event2.SetPersistence(Microsoft::Applications::Telemetry::EventPersistence::EventPersistence_Critical);

    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).Times(2).WillRepeatedly(Return());
    logger->LogEvent(event1);
    logger->LogEvent(event2);

    PAL::sleep(100); // Some time to let events be saved to DB

    logManager->FlushAndTeardown(); logManager.reset(); 

    PAL::sleep(2100); // Some time to let events be saved to DB
        
    configuration.SetIntProperty(CFG_INT_RAM_QUEUE_SIZE, 0);

    configuration.SetProperty(CFG_STR_CACHE_FILE_PATH, TEST_STORAGE_FILENAME);
    logManager.reset(ILogManager::Create(configuration, &runtimeConfig));

    // 1st request is from MetaStats
    waitForEvents(50, 4);
    auto payload = decodeRequest(receivedRequests[receivedRequests.size() - 1], false);
/*    
	ASSERT_THAT(receivedRequests, SizeIs(1));
    auto payload = decodeRequest(receivedRequests[0], false);
    ASSERT_THAT(payload.TokenToDataPackagesMap, Contains(Key("functests-tenant-token")));
    ASSERT_THAT(payload.TokenToDataPackagesMap["functests-tenant-token"], SizeIs(1));
    auto const& dp = payload.TokenToDataPackagesMap["functests-tenant-token"][0];
    ASSERT_THAT(dp.Records, SizeIs(2));
    verifyEvent(event1, dp.Records[0]);
    verifyEvent(event2, dp.Records[1]);
    */
}

TEST_F(BasicFuncTests, restartRecoversEventsFromDiskStorage)
{
	logManager->FlushAndTeardown(); logManager.reset(); 
    // Wait a bit so that the initial check for unsent events does not send our events too early.
    PAL::sleep(200);
 
    configuration.SetIntProperty(CFG_INT_RAM_QUEUE_SIZE,4096 * 20);
    configuration.SetProperty(CFG_STR_CACHE_FILE_PATH, TEST_STORAGE_FILENAME);

    logManager.reset(ILogManager::Create(configuration, &runtimeConfig));
    logger = logManager->GetLogger("functests-Tenant-Token", "source");
    logger2 = logManager->GetLogger("FuncTests2-tenant-token", "Source");

    EventProperties event1("first_event");
    EventProperties event2("second_event");
    event1.SetProperty("property1", "value1");
    event2.SetProperty("property2", "value2");

    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).Times(2).WillRepeatedly(Return());
    logger->LogEvent(event1);
    logger->LogEvent(event2);

    PAL::sleep(100); // Some time to let events be saved to DB

    logManager->FlushAndTeardown(); logManager.reset(); 

	PAL::sleep(100);    
    logManager.reset(ILogManager::Create(configuration, &runtimeConfig));

    // 1st request is from MetaStats
    waitForEvents(100, 5);

/*    auto payload = decodeRequest(receivedRequests[1], false);
    ASSERT_THAT(payload.TokenToDataPackagesMap, Contains(Key("functests-tenant-token")));
    ASSERT_THAT(payload.TokenToDataPackagesMap["functests-tenant-token"], SizeIs(1));
    auto const& dp = payload.TokenToDataPackagesMap["functests-tenant-token"][0];
    ASSERT_THAT(dp.Records, SizeIs(2));
    verifyEvent(event1, dp.Records[0]);
    verifyEvent(event2, dp.Records[1]);
    */
}

TEST_F(BasicFuncTests, restartRecoversEventsFromDiskStorageWithTimeout)
{
    logManager->FlushAndTeardown(); logManager.reset(); 
    // Wait a bit so that the initial check for unsent events does not send our events too early.
    PAL::sleep(200);

    configuration.SetIntProperty(CFG_INT_RAM_QUEUE_SIZE, 4096 * 20);
    configuration.SetIntProperty(CFG_INT_MAX_TEARDOWN_TIME, 5);
   
    configuration.SetProperty(CFG_STR_CACHE_FILE_PATH, TEST_STORAGE_FILENAME);
    
    logManager.reset(ILogManager::Create(configuration, &runtimeConfig));
    logger = logManager->GetLogger("functests-Tenant-Token", "source");
    logger2 = logManager->GetLogger("FuncTests2-tenant-token", "Source");

    EventProperties event1("first_event");
    EventProperties event2("second_event");
    event1.SetProperty("property1", "value1");
    event2.SetProperty("property2", "value2");

    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).Times(2).WillRepeatedly(Return());
    logger->LogEvent(event1);
    logger->LogEvent(event2);

    //PAL::sleep(100); // Some time to let events be saved to DB

    logManager->FlushAndTeardown(); logManager.reset(); 

    logManager.reset(ILogManager::Create(configuration, &runtimeConfig));

   // PAL::sleep(1000); // Some time to let events be saved to DB
    // 1st request is from MetaStats
    waitForEvents(50, 4);
}

TEST_F(BasicFuncTests, storageFileSizeDoesntExceedConfiguredSize)
{
	logManager->FlushAndTeardown(); logManager.reset(); 
	
	// Wait a bit so that the initial check for unsent events does not send our events too early.
	PAL::sleep(200);

    configuration.SetIntProperty(CFG_INT_MAX_TEARDOWN_TIME, 0);
    configuration.SetIntProperty(CFG_INT_RAM_QUEUE_SIZE, 0);
    configuration.SetProperty(CFG_STR_CACHE_FILE_PATH, TEST_STORAGE_FILENAME); 
    logManager.reset(ILogManager::Create(configuration, &runtimeConfig));

	logger = logManager->GetLogger("functests-Tenant-Token", "source");


    static int64_t const ONE_EVENT_SIZE   = 500 * 1024;
    static int64_t const MAX_FILE_SIZE    =   8 * 1024 * 1024;
    static int64_t const ALLOWED_OVERFLOW =  10 * MAX_FILE_SIZE / 100;

    std::string slowServiceUrl = runtimeConfig.GetCollectorUrl();
    slowServiceUrl.insert(slowServiceUrl.find('/', sizeof("http://")) + 1, "slow/");
    EXPECT_CALL(runtimeConfig, GetCollectorUrl())
        .WillRepeatedly(Return(slowServiceUrl));
    EXPECT_CALL(runtimeConfig, GetOfflineStorageMaximumSizeBytes())
        .WillRepeatedly(Return(static_cast<unsigned>(MAX_FILE_SIZE)));
    EXPECT_CALL(runtimeConfig, GetOfflineStorageResizeThresholdPct())
        .WillRepeatedly(Return(25));

    for (int i = 0; i < 50; i++) {
        EventProperties event("event" + toString(i));
        event.SetPriority(EventPriority_Normal);
        event.SetProperty("property", "value");
        event.SetProperty("big_data", std::string(ONE_EVENT_SIZE, '\42'));
        EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _));
        logger->LogEvent(event);

        // Slow down to make sure the events are saved to the database and
        // allow some uploads to start in the meantime as well.
        PAL::sleep(100);

        EXPECT_THAT(getFileSize(TEST_STORAGE_FILENAME), Lt(MAX_FILE_SIZE + ALLOWED_OVERFLOW));
    }

    // Check meta stats after restart. Because of their high priority, they will
    // be sent alone in the very first request regardless of other events.
    logManager->FlushAndTeardown(); logManager.reset(); 
    PAL::sleep(2000);
    receivedRequests.clear();
        
    configuration.SetProperty(CFG_STR_CACHE_FILE_PATH, TEST_STORAGE_FILENAME);
    logManager.reset(ILogManager::Create(configuration, &runtimeConfig));

    waitForEvents(50, 8);
    auto payload = decodeRequest(receivedRequests[0], false);
/*    auto payload = decodeRequest(receivedRequests[0], false);
    ASSERT_THAT(payload.TokenToDataPackagesMap["metastats-tenant-token"], SizeIs(1));
    auto const& dp = payload.TokenToDataPackagesMap["metastats-tenant-token"][0];
    ASSERT_THAT(dp.Records, SizeIs(2));
    EXPECT_THAT(dp.Records[0].Id, Not(IsEmpty()));
    EXPECT_THAT(dp.Records[0].Type, Eq("client_telemetry"));
    EXPECT_THAT(dp.Records[0].Extension, Contains(Pair("stats_rollup_kind", "stop")));
    // The expected number of dropped events is hard to estimate because of database overhead,
    // varying timing, some events have been sent etc. Just check that it's at least a quarter.
    EXPECT_THAT(dp.Records[0].Extension, Contains(Pair("records_dropped_offline_storage_overflow", StrAsIntGt(50 / 4))));
    */
}

TEST_F(BasicFuncTests, sendMetaStatsOnStart)
{
    // Wait a bit so that the initial check for unsent events does not send our events too early.
    PAL::sleep(200);

    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).Times(2).WillRepeatedly(Return());

    EventProperties event1("first_event");
    event1.SetPriority(EventPriority_High);
    event1.SetProperty("property1", "value1");
    logger->LogEvent(event1);

    EventProperties event2("second_event");
    event2.SetProperty("property2", "value2");
    logger->LogEvent(event2);

    PAL::sleep(100); // Some time to let events be saved to DB

    logManager->FlushAndTeardown(); logManager.reset(); 

    configuration.SetIntProperty(CFG_INT_RAM_QUEUE_SIZE, 4096 * 20);
    configuration.SetProperty(CFG_STR_CACHE_FILE_PATH, TEST_STORAGE_FILENAME);
    
    logManager.reset(ILogManager::Create(configuration, &runtimeConfig));

    waitForEvents(50, 2);
    auto payload = decodeRequest(receivedRequests[receivedRequests.size() - 1], false);
 /*   auto payload = decodeRequest(receivedRequests[1], false);
    ASSERT_THAT(payload.TokenToDataPackagesMap, Contains(Key("functests-tenant-token")));
    ASSERT_THAT(payload.TokenToDataPackagesMap["functests-tenant-token"], SizeIs(1));
    auto const& dp = payload.TokenToDataPackagesMap["metastats-tenant-token"][0];
    ASSERT_THAT(dp.Records, SizeIs(2));
    EXPECT_THAT(dp.Records[1].Id, Not(IsEmpty()));
    EXPECT_THAT(dp.Records[1].Type, Eq("client_telemetry"));
    EXPECT_THAT(dp.Records[1].Extension, Contains(Pair("records_received_count", "3")));
    EXPECT_THAT(dp.Records[1].Extension, Contains(Pair("stats_rollup_kind",      "stop")));
    */
}

TEST_F(BasicFuncTests, networkProblemsDoNotDropEvents)
{
    // Wait a bit so that the initial check for unsent events does not send our event too early.
    PAL::sleep(200);

    std::string badPortUrl = runtimeConfig.GetCollectorUrl();
    badPortUrl.replace(0, badPortUrl.find('/', sizeof("http://")), "http://127.0.0.1:65535");

    // Drop events without retrying, not even once.
    EXPECT_CALL(runtimeConfig, GetMaximumRetryCount())
        .WillRepeatedly(Return(0));

    // Use the bad URL twice, then retire and start using the good one again (defined in SetUp() earlier).
    EXPECT_CALL(runtimeConfig, GetCollectorUrl())
        .Times(2)
        .WillRepeatedly(Return(badPortUrl))
        .RetiresOnSaturation();

    EventProperties event("event");
    event.SetProperty("property", "value");
    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _))
        .WillOnce(Return());
    logger->LogEvent(event);

    // After initial delay of 2 seconds, the library will send a request, wait 3 seconds, send 1st retry, wait 3 seconds, send 2nd retry.
    // Stop waiting 1 second before the 2nd retry (which will use the good URL again) and check that nothing has been received yet.
    PAL::sleep(2000 + 2 * 3000 - 1000);
    ASSERT_THAT(receivedRequests, SizeIs(0));

    // If the code works correctly, the event was not dropped (despite being retried twice)
    // because the retry was caused by network connectivity failures only - validate it.
    waitForEvents(50, 2);
    auto payload = decodeRequest(receivedRequests[receivedRequests.size() - 1], false);

//    EXPECT_THAT(payload.TokenToDataPackagesMap, Contains(Key("functests-tenant-token")));
}

TEST_F(BasicFuncTests, serverProblemsDropEventsAfterMaxRetryCount)
{
    // Wait a bit so that the initial check for unsent events does not send our event too early.
    PAL::sleep(200);

    std::string badServiceUrl = runtimeConfig.GetCollectorUrl();
    badServiceUrl.insert(badServiceUrl.find('/', sizeof("http://")) + 1, "503/");

    // Drop events after just 1 retry to avoid long test run time.
    EXPECT_CALL(runtimeConfig, GetMaximumRetryCount())
        .WillRepeatedly(Return(1));

    // Use the bad URL twice, then retire and start using the good one again (defined in SetUp() earlier).
    EXPECT_CALL(runtimeConfig, GetCollectorUrl())
        .Times(2)
        .WillRepeatedly(Return(badServiceUrl))
        .RetiresOnSaturation();

    EventProperties event("event");
    event.SetProperty("property", "value");
    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _))
        .WillOnce(Return());
    logger->LogEvent(event);

    // After initial delay of 2 seconds, the library will send a request, wait 3 seconds, send 1st retry and stop.
    // 2nd retry after another 3 seconds (using the good URL again) should not come - wait 1 more second to be sure.
    PAL::sleep(2000 + 2 * 3000 + 1000);
   // EXPECT_THAT(receivedRequests, SizeIs(0));

    // Check meta stats on restart (will be first request)
    logManager->FlushAndTeardown(); logManager.reset(); 

    configuration.SetIntProperty(CFG_INT_RAM_QUEUE_SIZE, 4096 * 20);
    configuration.SetProperty(CFG_STR_CACHE_FILE_PATH, TEST_STORAGE_FILENAME);
    
    logManager.reset(ILogManager::Create(configuration, &runtimeConfig));

    waitForEvents(100, 2);
    auto payload = decodeRequest(receivedRequests[receivedRequests.size() - 1], false);
/*    auto const& dp = payload.TokenToDataPackagesMap["metastats-tenant-token"][0];
    ASSERT_THAT(dp.Records, SizeIs(1));
    EXPECT_THAT(dp.Records[0].Id, Not(IsEmpty()));
    EXPECT_THAT(dp.Records[0].Type, Eq("client_telemetry"));
    EXPECT_THAT(dp.Records[0].Extension, Contains(Pair("stats_rollup_kind", "stop")));
    EXPECT_THAT(dp.Records[0].Extension, Contains(Pair("records_dropped_retry_exceeded", "2")));
    */
}

TEST_F(BasicFuncTests, metaStatsAreSentOnlyWhenNewDataAreAvailable)
{
    EXPECT_CALL(runtimeConfig, GetMetaStatsSendIntervalSec())
        .WillRepeatedly(Return(3));

    // Wait to see what is coming after start. There should be no uploads since nothing has happened yet.
    PAL::sleep(3500);
    ASSERT_THAT(receivedRequests, SizeIs(1));

	receivedRequests.clear();
    EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _))
        .WillOnce(Return());
    EventProperties event("some_event");
    event.SetPriority(EventPriority_Immediate);
    logger->LogEvent(event);

    // 2 requests should be sent:
    //   1st with "some_event" and immediate priority
    //   2nd with act_stats about that
    waitForEvents(3500, 2);
    auto payload = decodeRequest(receivedRequests[receivedRequests.size() - 1], false);

/*    ASSERT_THAT(payload.TokenToDataPackagesMap, Contains(Key("metastats-tenant-token")));
    ASSERT_THAT(payload.TokenToDataPackagesMap["metastats-tenant-token"], SizeIs(1));
    auto const& dp = payload.TokenToDataPackagesMap["metastats-tenant-token"][0];
    ASSERT_THAT(dp.Records, SizeIs(1));
    EXPECT_THAT(dp.Records[0].Extension, Contains(Pair("stats_rollup_kind", "ongoing")));
    EXPECT_THAT(dp.Records[0].Extension, Contains(Pair("records_received_count", "2")));
    EXPECT_THAT(dp.Records[0].Extension, Contains(Pair("requests_acked_succeeded", "2")));

    // Wait a few more seconds to see that no more events are being sent.
    PAL::sleep(3500);
    EXPECT_THAT(receivedRequests.size(), 2);
    */
}
