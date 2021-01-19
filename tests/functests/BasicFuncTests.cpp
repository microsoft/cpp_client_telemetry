// clang-format off
//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif
#include "common/Common.hpp"
#include "common/HttpServer.hpp"
#include "utils/Utils.hpp"
#include "api/LogManagerImpl.hpp"

#include "bond/All.hpp"
#include "CsProtocol_types.hpp"
#include "bond/generated/CsProtocol_readers.hpp"
#include "LogManager.hpp"

#include "CompliantByDefaultFilterApi.hpp"

#include <fstream>
#include <atomic>
#include <assert.h>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <vector>

#include "PayloadDecoder.hpp"

using namespace testing;
using namespace MAT;

// LOGMANAGER_INSTANCE

namespace PAL_NS_BEGIN
{
    class PALTest
    {
       public:

        static long GetPalRefCount()
        {
            return PAL::GetPAL().m_palStarted.load();
        };

        static std::shared_ptr<ITaskDispatcher> GetTaskDispatcher()
        {
            return PAL::GetPAL().m_taskDispatcher;
        };

        static std::shared_ptr<ISystemInformation> GetSystemInformation()
        {
            return PAL::GetPAL().m_SystemInformation;
        }

        static std::shared_ptr<INetworkInformation> GetNetworkInformation()
        {
            return PAL::GetPAL().m_NetworkInformation;
        }

        static std::shared_ptr<IDeviceInformation> GetDeviceInformation()
        {
            return PAL::GetPAL().m_DeviceInformation;
        }
    };
}
PAL_NS_END;

namespace MAT_NS_BEGIN
{
    class ModuleA : public ILogConfiguration
    {
    };
    class LogManagerA : public LogManagerBase<ModuleA>
    {
    };
    class ModuleB : public ILogConfiguration
    {
    };
    class LogManagerB : public LogManagerBase<ModuleB>
    {
    };
    // Two distinct LogManagerX 'singelton' instances
    DEFINE_LOGMANAGER(LogManagerB, ModuleB);
    DEFINE_LOGMANAGER(LogManagerA, ModuleA);
}
MAT_NS_END

char const* const TEST_STORAGE_FILENAME = "BasicFuncTests.db";

// 1DSCppSdktest sandbox key
#define TEST_TOKEN      "7c8b1796cbc44bd5a03803c01c2b9d61-b6e370dd-28d9-4a52-9556-762543cf7aa7-6991"

#define KILLED_TOKEN    "deadbeefdeadbeefdeadbeefdeadbeef-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322"
#define HTTP_PORT       19000

#undef LOCKGUARD
#define LOCKGUARD(macro_mutex) std::lock_guard<decltype(macro_mutex)> TOKENPASTE2(__guard_, __LINE__) (macro_mutex);
class HttpPostListener : public DebugEventListener
{
public:
    virtual void OnDebugEvent(DebugEvent &evt)
    {
        static unsigned seq = 0;
        switch (evt.type)
        {
        case EVT_HTTP_OK:
            {
                seq++;
                std::string out;
                std::vector<uint8_t> reqBody((unsigned char *)evt.data, (unsigned char *)(evt.data) + evt.size);
                MAT::exporters::DecodeRequest(reqBody, out, false);
                printf(">>>> REQUEST [%u]:%s\n", seq, out.c_str());
            }
            break;
        default:
            break;
        };
    };
};
class BasicFuncTests : public ::testing::Test,
    public HttpServer::Callback
{
protected:
    std::mutex                       mtx_requests;
    std::vector<HttpServer::Request> receivedRequests;
    std::string serverAddress;
    HttpServer server;

    ILogger* logger;
    ILogger* logger2;

    std::atomic<bool> isSetup;
    std::atomic<bool> isRunning;

    std::condition_variable cv_gotEvents;
    std::mutex cv_m;
public:

    BasicFuncTests() :
        isSetup(false) ,
        isRunning(false)
    {};

    virtual void SetUp() override
    {
        if (isSetup.exchange(true))
        {
            return;
        }
        int port = server.addListeningPort(HTTP_PORT);
        std::ostringstream os;
        os << "localhost:" << port;
        serverAddress = "http://" + os.str() + "/simple/";
        server.setServerName(os.str());
        server.addHandler("/simple/", *this);
        server.addHandler("/slow/", *this);
        server.addHandler("/503/", *this);
        server.setKeepalive(false); // This test doesn't work well with keep-alive enabled
        server.start();
        isRunning = true;
    }

    virtual void TearDown() override
    {
        if (!isSetup.exchange(false))
            return;
        server.stop();
        isRunning = false;
    }

    virtual void CleanStorage()
    {
        std::string fileName = MAT::GetTempDirectory();
        fileName += PATH_SEPARATOR_CHAR;
        fileName += TEST_STORAGE_FILENAME;
        std::remove(fileName.c_str());
    }

    virtual void Initialize()
    {
        receivedRequests.clear();
        auto configuration = LogManager::GetLogConfiguration();

        configuration[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFF;
#ifdef NDEBUG
        configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Warn;
#else
        configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Trace;
#endif
        configuration[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_CS;

        configuration[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;
        configuration[CFG_STR_CACHE_FILE_PATH] = TEST_STORAGE_FILENAME;
        configuration[CFG_INT_MAX_TEARDOWN_TIME] = 2;   // 2 seconds wait on shutdown
        configuration[CFG_STR_COLLECTOR_URL] = serverAddress.c_str();
        configuration[CFG_MAP_HTTP][CFG_BOOL_HTTP_COMPRESSION] = false;      // disable compression for now
        configuration[CFG_MAP_METASTATS_CONFIG][CFG_INT_METASTATS_INTERVAL] = 30 * 60;   // 30 mins

        configuration["name"] = __FILE__;
        configuration["version"] = "1.0.0";
        configuration["config"] = { { "host", __FILE__ } }; // Host instance

        LogManager::Initialize(TEST_TOKEN, configuration);
        LogManager::SetLevelFilter(DIAG_LEVEL_DEFAULT, { DIAG_LEVEL_DEFAULT_MIN, DIAG_LEVEL_DEFAULT_MAX });
        LogManager::ResumeTransmission();

        logger  = LogManager::GetLogger(TEST_TOKEN, "source1");
        logger2 = LogManager::GetLogger(TEST_TOKEN, "source2");
    }

    virtual void FlushAndTeardown()
    {
        LogManager::FlushAndTeardown();
    }

    virtual int onHttpRequest(HttpServer::Request const& request, HttpServer::Response& response) override
    {
        if (request.uri.compare(0, 5, "/503/") == 0) {
            return 503;
        }

        if (request.uri.compare(0, 6, "/slow/") == 0) {
            PAL::sleep(static_cast<unsigned int>(request.content.size() / DELAY_FACTOR_FOR_SERVER));
        }

        {
            LOCKGUARD(mtx_requests);
            receivedRequests.push_back(request);
        }

        response.headers["Content-Type"] = "text/plain";
        response.content = "{ \"status\": \"0\" }";

        return 200;
    }

    bool waitForRequests(unsigned timeOutSec, unsigned expected_count = 1)
    {
        std::unique_lock<std::mutex> lk(cv_m);
        if (cv_gotEvents.wait_for(lk, std::chrono::milliseconds(1000 * timeOutSec), [&] { return receivedRequests.size() >= expected_count; }))
        {
            return true;
        }
        return false;
    }

    void waitForEvents(unsigned timeOutSec, unsigned expected_count = 1)
    {
        unsigned receivedEvents = 0;
        auto start = PAL::getUtcSystemTimeMs();
        size_t lastIdx = 0;
        while ( ((PAL::getUtcSystemTimeMs()-start)<(1000* timeOutSec)) && (receivedEvents!=expected_count) )
        {
            /* Give time for our friendly HTTP server thread to process incoming request */
            std::this_thread::yield();
            {
                LOCKGUARD(mtx_requests);
                if (receivedRequests.size())
                {
                    size_t size = receivedRequests.size();

                    //requests can come within 100 milisec sleep
                    for (size_t index = lastIdx; index < size; index++)
                    {
                        auto request = receivedRequests.at(index);
                        auto payload = decodeRequest(request, false);
                        receivedEvents+= (unsigned)payload.size();
                    }
                    lastIdx = size;
                }
            }
        }
        ASSERT_EQ(receivedEvents, expected_count);
    }

    std::vector<CsProtocol::Record> decodeRequest(HttpServer::Request const& request, bool decompress)
    {
        UNREFERENCED_PARAMETER(decompress);
        // TODO: [MG] - implement decompression

        std::vector<CsProtocol::Record> vector;

        size_t data = 0;
        size_t length = 0;
        while (data < request.content.size())
        {
            CsProtocol::Record result;
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
                    if (index + 2 < length)
                    {
                        // Search for Version marker after \x3 in Bond stream
                        if (test[index + 1] == ('0'+::CsProtocol::CS_VER_MAJOR) && test[index + 2] == '.')
                        {
                            found = true;
                            break;
                        }
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

    void verifyEvent(EventProperties const& expected, ::CsProtocol::Record const& actual)
    {
        EXPECT_THAT(actual.name, Not(IsEmpty()));
        int64_t now = PAL::getUtcSystemTimeinTicks();
        EXPECT_THAT(actual.time, Gt(now - 60000000000));
        EXPECT_THAT(actual.time, Le(now));
        EXPECT_THAT(actual.name, expected.GetName());

        // If empty event property bag, then verify the name and return
        if (!actual.data.size())
        {
            EXPECT_THAT(actual.name, expected.GetName());
            return;
        }

        for (const auto& prop : expected.GetProperties())
        {
            if (prop.second.piiKind == PiiKind_None)
            {
                std::map<std::string, CsProtocol::Value>::const_iterator iter = actual.data[0].properties.find(prop.first);
                if (iter != actual.data[0].properties.end())
                {
                    CsProtocol::Value temp = iter->second;
                    switch (temp.type)
                    {
                    case ::CsProtocol::ValueInt64:
                    case ::CsProtocol::ValueUInt64:
                    case ::CsProtocol::ValueInt32:
                    case ::CsProtocol::ValueUInt32:
                    case ::CsProtocol::ValueBool:
                    case ::CsProtocol::ValueDateTime:
                    {
                        EXPECT_THAT(temp.longValue, prop.second.as_int64);
                        break;
                    }
                    case ::CsProtocol::ValueDouble:
                    {
                        EXPECT_THAT(temp.doubleValue, prop.second.as_double);
                        break;
                    }
                    case ::CsProtocol::ValueString:
                    {
                        EXPECT_THAT(temp.stringValue, prop.second.as_string);
                        break;
                    }
                    case ::CsProtocol::ValueGuid:
                    {
                        uint8_t guid_bytes[16] = { 0 };
                        prop.second.as_guid.to_bytes(guid_bytes);
                        std::vector<uint8_t> guid = std::vector<uint8_t>(guid_bytes, guid_bytes + sizeof(guid_bytes) / sizeof(guid_bytes[0]));

                        EXPECT_THAT(temp.guidValue[0].size(), guid.size());
                        for (size_t index = 0; index < guid.size(); index++)
                        {
                            uint8_t val1 = temp.guidValue.at(0).at(index);
                            uint8_t val2 = guid[index];
                            EXPECT_THAT(val1, val2);
                        }
                        break;
                    }
                    case ::CsProtocol::ValueArrayInt64:
                    case ::CsProtocol::ValueArrayUInt64:
                    case ::CsProtocol::ValueArrayInt32:
                    case ::CsProtocol::ValueArrayUInt32:
                    case ::CsProtocol::ValueArrayBool:
                    case ::CsProtocol::ValueArrayDateTime:
                    {
                        std::vector<int64_t>& vectror = temp.longArray.at(0);
                        EXPECT_THAT(vectror.size(), prop.second.as_longArray->size());
                        for (size_t index = 0; index < prop.second.as_longArray->size(); index++)
                        {
                            uint64_t val1 = vectror.at(index);
                            uint64_t val2 = prop.second.as_longArray->at(index);
                            EXPECT_THAT(val1, val2);
                        }

                        break;
                    }
                    case ::CsProtocol::ValueArrayDouble:
                    {
                        std::vector<double>& vectror = temp.doubleArray.at(0);
                        EXPECT_THAT(vectror.size(), prop.second.as_doubleArray->size());
                        for (size_t index = 0; index < prop.second.as_doubleArray->size(); index++)
                        {
                            double val1 = vectror.at(index);
                            double val2 = prop.second.as_doubleArray->at(index);
                            EXPECT_THAT(val1, val2);
                        }

                        break;
                    }
                    case ::CsProtocol::ValueArrayString:
                    {
                        std::vector<std::string>& vectror = temp.stringArray.at(0);
                        EXPECT_THAT(vectror.size(), prop.second.as_stringArray->size());
                        for (size_t index = 0; index < prop.second.as_stringArray->size(); index++)
                        {
                            std::string val1 = vectror.at(index);
                            std::string val2 = prop.second.as_stringArray->at(index);
                            EXPECT_THAT(val1, val2);
                        }
                        break;
                    }
                    case ::CsProtocol::ValueArrayGuid:
                    {
                        // EXPECT_THAT(temp.guidArray, prop.second.as_guidArray);
                        std::vector<std::vector<uint8_t>>& vectror = temp.guidArray.at(0);
                        EXPECT_THAT(vectror.size(), prop.second.as_guidArray->size());
                        for (size_t index = 0; index < prop.second.as_guidArray->size(); index++)
                        {
                            uint8_t guid_bytes[16] = { 0 };
                            prop.second.as_guidArray->at(index).to_bytes(guid_bytes);
                            std::vector<uint8_t> guid = std::vector<uint8_t>(guid_bytes, guid_bytes + sizeof(guid_bytes) / sizeof(guid_bytes[0]));

                            EXPECT_THAT(vectror.at(index).size(), guid.size());
                            for (size_t index1 = 0; index1 < guid.size(); index1++)
                            {
                                uint8_t val1 = vectror.at(index).at(index1);
                                uint8_t val2 = guid[index1];
                                EXPECT_THAT(val1, val2);
                            }
                        }

                        break;
                    }
                    default:
                    {
                        break;
                    }
                    }
                }
            }
        }
        for (auto const& property : expected.GetPiiProperties())
        {
            ::CsProtocol::PII pii;
            pii.Kind = static_cast<::CsProtocol::PIIKind>(property.second.second);
            // EXPECT_THAT(actual.PIIExtensions, Contains(Pair(property.first, pii)));
        }

        /*       for (auto const& property : expected.GetCustomerContentProperties())
               {
                   ::CsProtocol::CustomerContent cc;
                   cc.Kind = static_cast< ::CsProtocol::CustomerContentKind>(property.second.second);
                   //EXPECT_THAT(actual.CustomerContentExtensions, Contains(Pair(property.first, cc)));
               }
       */
    }

    int64_t getFileSize(std::string const& filename)
    {
        std::ifstream ifile(filename);
        ifile.seekg(0, std::ios_base::end);
        return ifile.tellg();
    }

    std::vector<CsProtocol::Record> records()
    {
        std::vector<CsProtocol::Record> result;
        if (receivedRequests.size())
        {
            for (auto &request : receivedRequests)
            {
                // TODO: [MG] - add compression support
                auto payload = decodeRequest(request, false);
                for (auto &record : payload)
                {
                    result.push_back(std::move(record));
                }
            }
        }
        return result;
    }

    // Find first matching event
    CsProtocol::Record find(const std::string& name)
    {
        CsProtocol::Record result;
        result.name = "";
        if (receivedRequests.size())
        {
            for (auto &request : receivedRequests)
            {
                // TODO: [MG] - add compression support
                auto payload = decodeRequest(request, false);
                for (auto &record : payload)
                {
                    if (record.name == name)
                    {
                        result = record;
                        return result;
                    }
                }
            }
        }
        return result;
    }
};


TEST_F(BasicFuncTests, doNothing)
{
    CleanStorage();
    Initialize();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    FlushAndTeardown();
}

TEST_F(BasicFuncTests, sendOneEvent_immediatelyStop)
{
    CleanStorage();
    Initialize();
    EventProperties event("first_event");
    event.SetProperty("property", "value");
    logger->LogEvent(event);
    FlushAndTeardown();
    EXPECT_GE(receivedRequests.size(), (size_t)1); // at least 1 HTTP request with customer payload and stats
}

TEST_F(BasicFuncTests, sendNoPriorityEvents)
{
    CleanStorage();
    Initialize();
    /* Verify both:
     - local deserializer implementation in verifyEvent
     - public MAT::exporters::DecodeRequest(...) via debug callback
     */
    HttpPostListener listener;
    LogManager::AddEventListener(EVT_HTTP_OK, listener);

    EventProperties event("first_event");
    event.SetProperty("property", "value");
    logger->LogEvent(event);

    EventProperties event2("second_event");
    event2.SetProperty("property", "value2");
    event2.SetProperty("property2", "another value");
    logger->LogEvent(event2);

    LogManager::UploadNow();
    waitForEvents(1, 3);
    EXPECT_GE(receivedRequests.size(), (size_t)1);
    LogManager::RemoveEventListener(EVT_HTTP_OK, listener);
    FlushAndTeardown();

    if (receivedRequests.size() >= 1)
    {
        verifyEvent(event,  find("first_event"));
        verifyEvent(event2, find("second_event"));
    }

}

TEST_F(BasicFuncTests, sendSamePriorityNormalEvents)
{
    CleanStorage();
    Initialize();

    EventProperties event("first_event");
    event.SetPriority(EventPriority_Normal);
    event.SetProperty("property", "value");
    std::vector<int64_t> intvector(8);
    std::fill(intvector.begin(), intvector.begin() + 4, 5);
    std::fill(intvector.begin() + 3, intvector.end() - 2, 8);
    event.SetProperty("property1", intvector);
    std::vector<double> dvector(8);
    std::fill(dvector.begin(), dvector.begin() + 4, 4.9999);
    std::fill(dvector.begin() + 3, dvector.end() - 2, 7.9999);
    event.SetProperty("property2", dvector);
    std::vector<std::string> svector(8);
    std::fill(svector.begin(), svector.begin() + 4, "string");
    std::fill(svector.begin() + 3, svector.end() - 2, "string2");
    event.SetProperty("property3", svector);
    std::vector<GUID_t> gvector(8);
    std::fill(gvector.begin(), gvector.begin() + 4, GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F"));
    std::fill(gvector.begin() + 3, gvector.end() - 2, GUID_t("00000000-0000-0000-0000-000000000000"));
    event.SetProperty("property4", gvector);
    logger->LogEvent(event);

    EventProperties event2("second_event");
    event2.SetPriority(EventPriority_Normal);
    event2.SetProperty("property", "value2");
    event2.SetProperty("property2", "another value");
    event2.SetProperty("pii_property", "pii_value", PiiKind_Identity);
    event2.SetProperty("cc_property", "cc_value", CustomerContentKind_GenericData);
    logger->LogEvent(event2);

    waitForEvents(3, 3);
    for (const auto &evt : { event, event2 })
    {
        verifyEvent(evt, find(evt.GetName()));
    }

    FlushAndTeardown();
}

TEST_F(BasicFuncTests, sendDifferentPriorityEvents)
{
    CleanStorage();
    Initialize();

    EventProperties event("first_event");
    event.SetPriority(EventPriority_Normal);
    event.SetProperty("property", "value");
    std::vector<int64_t> intvector(8);
    std::fill(intvector.begin(), intvector.begin() + 4, 5);
    std::fill(intvector.begin() + 3, intvector.end() - 2, 8);
    event.SetProperty("property1", intvector);

    std::vector<double> dvector(8);
    std::fill(dvector.begin(), dvector.begin() + 4, 4.9999);
    std::fill(dvector.begin() + 3, dvector.end() - 2, 7.9999);
    event.SetProperty("property2", dvector);

    std::vector<std::string> svector(8);
    std::fill(svector.begin(), svector.begin() + 4, "string");
    std::fill(svector.begin() + 3, svector.end() - 2, "string2");
    event.SetProperty("property3", svector);

    std::vector<GUID_t> gvector(8);
    std::fill(gvector.begin(), gvector.begin() + 4, GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F"));
    std::fill(gvector.begin() + 3, gvector.end() - 2, GUID_t("00000000-0000-0000-0000-000000000000"));
    event.SetProperty("property4", gvector);


    logger->LogEvent(event);

    EventProperties event2("second_event");
    event2.SetPriority(EventPriority_High);
    event2.SetProperty("property", "value2");
    event2.SetProperty("property2", "another value");
    event2.SetProperty("pii_property", "pii_value", PiiKind_Identity);
    event2.SetProperty("cc_property", "cc_value", CustomerContentKind_GenericData);


    logger->LogEvent(event2);

    LogManager::UploadNow();
    // 2 x customer events + 1 x evt_stats on start
    waitForEvents(1, 3);

    for (const auto &evt : { event, event2 })
    {
        verifyEvent(evt, find(evt.GetName()));
    }

    FlushAndTeardown();
}

TEST_F(BasicFuncTests, sendMultipleTenantsTogether)
{
    CleanStorage();
    Initialize();

    EventProperties event1("first_event");
    event1.SetProperty("property", "value");
    std::vector<int64_t> intvector(8);
    std::fill(intvector.begin(), intvector.begin() + 4, 5);
    std::fill(intvector.begin() + 3, intvector.end() - 2, 8);
    event1.SetProperty("property1", intvector);

    std::vector<double> dvector(8);
    std::fill(dvector.begin(), dvector.begin() + 4, 4.9999);
    std::fill(dvector.begin() + 3, dvector.end() - 2, 7.9999);
    event1.SetProperty("property2", dvector);

    std::vector<std::string> svector(8);
    std::fill(svector.begin(), svector.begin() + 4, "string");
    std::fill(svector.begin() + 3, svector.end() - 2, "string2");
    event1.SetProperty("property3", svector);

    std::vector<GUID_t> gvector(8);
    std::fill(gvector.begin(), gvector.begin() + 4, GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F"));
    std::fill(gvector.begin() + 3, gvector.end() - 2, GUID_t("00000000-0000-0000-0000-000000000000"));
    event1.SetProperty("property4", gvector);

    logger->LogEvent(event1);

    EventProperties event2("second_event");
    event2.SetProperty("property", "value2");
    event2.SetProperty("property2", "another value");

    logger2->LogEvent(event2);

    LogManager::UploadNow();

    // 2 x customer events + 1 x evt_stats on start
    waitForEvents(1, 3);
    for (const auto &evt : { event1, event2 })
    {
        verifyEvent(evt, find(evt.GetName()));
    }

    FlushAndTeardown();

}

TEST_F(BasicFuncTests, configDecorations)
{
    CleanStorage();
    Initialize();

    EventProperties event1("first_event");
    logger->LogEvent(event1);

    EventProperties event2("second_event");
    logger->LogEvent(event2);

    EventProperties event3("third_event");
    logger->LogEvent(event3);

    EventProperties event4("4th_event");
    logger->LogEvent(event4);

    LogManager::UploadNow();
    waitForEvents(2, 5);

    for (const auto &evt : { event1, event2, event3, event4 })
    {
        verifyEvent(evt, find(evt.GetName()));
    }

    FlushAndTeardown();
}

TEST_F(BasicFuncTests, restartRecoversEventsFromStorage)
{
    {
        CleanStorage();
        Initialize();
        // This code is a bit racy because ResumeTransmission is done in Initialize
        LogManager::PauseTransmission();
        EventProperties event1("first_event");
        EventProperties event2("second_event");
        event1.SetProperty("property1", "value1");
        event2.SetProperty("property2", "value2");
        event1.SetLatency(MAT::EventLatency::EventLatency_RealTime);
        event1.SetPersistence(MAT::EventPersistence::EventPersistence_Critical);
        event2.SetLatency(MAT::EventLatency::EventLatency_RealTime);
        event2.SetPersistence(MAT::EventPersistence::EventPersistence_Critical);
        logger->LogEvent(event1);
        logger->LogEvent(event2);
        FlushAndTeardown();
    }

    {
        Initialize();
        EventProperties fooEvent("fooEvent");
        fooEvent.SetLatency(EventLatency_RealTime);
        fooEvent.SetPersistence(EventPersistence_Critical);
        LogManager::GetLogger()->LogEvent(fooEvent);
        LogManager::UploadNow();

        // 1st request for realtime event
        waitForEvents(3, 7); // start, first_event, second_event, ongoing, stop, start, fooEvent
        EXPECT_GE(receivedRequests.size(), (size_t)1);
        if (receivedRequests.size() != 0)
        {
            auto payload = decodeRequest(receivedRequests[receivedRequests.size() - 1], false);
        }
        FlushAndTeardown();
    }

    /*
        ASSERT_THAT(receivedRequests, SizeIs(1));
        auto payload = decodeRequest(receivedRequests[0], false);
        ASSERT_THAT(payload.TokenToDataPackagesMap, Contains(Key("functests-tenant-token")));
        ASSERT_THAT(payload.TokenToDataPackagesMap["functests-tenant-token"], SizeIs(1));
        auto const& dp = payload.TokenToDataPackagesMap["functests-tenant-token"][0];
        ASSERT_THAT(payload, SizeIs(2));
        verifyEvent(event1, payload[0]);
        verifyEvent(event2, payload[1]);
        */
}

#if 0 // FIXME: 1445871 [v3][1DS] Offline storage size may exceed configured limit
TEST_F(BasicFuncTests, storageFileSizeDoesntExceedConfiguredSize)
{
    CleanStorage();

    static int64_t const ONE_EVENT_SIZE = 500 * 1024;
    static int64_t const MAX_FILE_SIZE = 8 * 1024 * 1024;
    static int64_t const ALLOWED_OVERFLOW = 10 * MAX_FILE_SIZE / 100;

    auto &configuration = LogManager::GetLogConfiguration();
    configuration[CFG_INT_MAX_TEARDOWN_TIME] = 0;
    configuration[CFG_INT_CACHE_FILE_SIZE] = MAX_FILE_SIZE;

    std::string slowServiceUrl;
    slowServiceUrl.insert(slowServiceUrl.find('/', sizeof("http://")) + 1, "slow/");
    configuration[CFG_STR_COLLECTOR_URL] = slowServiceUrl.c_str();
    {
        Initialize();
        LogManager::PauseTransmission();
        for (int i = 0; i < 50; i++) {
            EventProperties event("event" + toString(i));
            event.SetPriority(EventPriority_Normal);
            event.SetProperty("property", "value");
            event.SetProperty("big_data", std::string(ONE_EVENT_SIZE, '\42'));
            logger->LogEvent(event);
        }
        // Check meta stats after restart. Because of their high priority, they will
        // be sent alone in the very first request regardless of other events.
        FlushAndTeardown();

        std::string fileName = MAT::GetTempDirectory();
        fileName += "\\";
        fileName += TEST_STORAGE_FILENAME;
        size_t fileSize = getFileSize(fileName);
        EXPECT_LE(fileSize, (size_t)(MAX_FILE_SIZE + ALLOWED_OVERFLOW));
    }

    // Restore fast URL
    configuration[CFG_STR_COLLECTOR_URL] = serverAddress.c_str();

    {
        Initialize();
        waitForEvents(2, 8);
        if (receivedRequests.size())
        {
            auto payload = decodeRequest(receivedRequests[0], false);
            /*    auto payload = decodeRequest(receivedRequests[0], false);
                ASSERT_THAT(payload.TokenToDataPackagesMap["metastats-tenant-token"], SizeIs(1));
                auto const& dp = payload.TokenToDataPackagesMap["metastats-tenant-token"][0];
                ASSERT_THAT(payload, SizeIs(2));
                EXPECT_THAT(payload[0].Id, Not(IsEmpty()));
                EXPECT_THAT(payload[0].Type, Eq("client_telemetry"));
                EXPECT_THAT(payload[0].Extension, Contains(Pair("stats_rollup_kind", "stop")));
                // The expected number of dropped events is hard to estimate because of database overhead,
                // varying timing, some events have been sent etc. Just check that it's at least a quarter.
                EXPECT_THAT(payload[0].Extension, Contains(Pair("records_dropped_offline_storage_overflow", StrAsIntGt(50 / 4))));
                */
        }
        FlushAndTeardown();
    }

}
#endif

TEST_F(BasicFuncTests, sendMetaStatsOnStart)
{
    CleanStorage();
    // Run offline
    Initialize();
    LogManager::PauseTransmission();

    EventProperties event1("first_event");
    event1.SetPriority(EventPriority_High);
    event1.SetProperty("property1", "value1");
    logger->LogEvent(event1);

    EventProperties event2("second_event");
    event2.SetProperty("property2", "value2");
    logger->LogEvent(event2);
    FlushAndTeardown();

    auto r1 = records();
    ASSERT_EQ(r1.size(), size_t { 0 });

    // Check
    Initialize();
    LogManager::ResumeTransmission(); // ?
    LogManager::UploadNow();
    PAL::sleep(2000);

    auto r2 = records();
    ASSERT_GE(r2.size(), (size_t)4); // (start + stop) + (2 events + start)

    for (const auto &evt : { event1, event2 })
    {
        verifyEvent(evt, find(evt.GetName()));
    }

    FlushAndTeardown();
}

TEST_F(BasicFuncTests, DiagLevelRequiredOnly_OneEventWithoutLevelOneWithButNotAllowedOneAllowed_OnlyAllowedEventSent)
{
    CleanStorage();
    Initialize();
    LogManager::SetLevelFilter(DIAG_LEVEL_OPTIONAL, { DIAG_LEVEL_REQUIRED });
    EventProperties eventWithoutLevel("EventWithoutLevel");
    logger->LogEvent(eventWithoutLevel);

    EventProperties eventWithNotAllowedLevel("EventWithNotAllowedLevel");
    eventWithNotAllowedLevel.SetLevel(DIAG_LEVEL_OPTIONAL);
    logger->LogEvent(eventWithNotAllowedLevel);

    EventProperties eventWithAllowedLevel("EventWithAllowedLevel");
    eventWithAllowedLevel.SetLevel(DIAG_LEVEL_REQUIRED);
    logger->LogEvent(eventWithAllowedLevel);

    LogManager::UploadNow();
    waitForEvents(1 /*timeout*/, 2 /*expected count*/);  // Start and EventWithAllowedLevel

    ASSERT_EQ(records().size(), static_cast<size_t>(2)); // Start and EventWithAllowedLevel

    verifyEvent(eventWithAllowedLevel, find(eventWithAllowedLevel.GetName()));

    FlushAndTeardown();
}

void SendEventWithOptionalThenRequired(ILogger* logger) noexcept
{
    EventProperties eventWithOptionalLevel("EventWithOptionalLevel");
    eventWithOptionalLevel.SetLevel(DIAG_LEVEL_OPTIONAL);
    logger->LogEvent(eventWithOptionalLevel);

    EventProperties eventWithRequiredLevel("EventWithRequiredLevel");
    eventWithRequiredLevel.SetLevel(DIAG_LEVEL_REQUIRED);
    logger->LogEvent(eventWithRequiredLevel);
}

static std::vector<CsProtocol::Record> GetEventsWithName(const char* name, const std::vector<CsProtocol::Record>& records) noexcept
{
    std::vector<CsProtocol::Record> results;
    for (const auto& record : records)
    {
        if (record.name.compare(name) == 0)
            results.push_back(record);
    }
    return results;
}

TEST_F(BasicFuncTests, DiagLevelRequiredOnly_SendTwoEventsUpdateAllowedLevelsSendTwoEvents_ThreeEventsSent)
{
    CleanStorage();
    Initialize();

    LogManager::SetLevelFilter(DIAG_LEVEL_OPTIONAL, { DIAG_LEVEL_REQUIRED });
    SendEventWithOptionalThenRequired(logger);

    LogManager::SetLevelFilter(DIAG_LEVEL_OPTIONAL, { DIAG_LEVEL_OPTIONAL, DIAG_LEVEL_REQUIRED });
    SendEventWithOptionalThenRequired(logger);

    LogManager::UploadNow();
    waitForEvents(2 /*timeout*/, 4 /*expected count*/);    // Start and EventWithAllowedLevel

    auto sentRecords = records();
    ASSERT_EQ(sentRecords.size(), static_cast<size_t>(4)); // Start and EventWithAllowedLevel
    ASSERT_EQ(GetEventsWithName("EventWithOptionalLevel", sentRecords).size(), size_t{ 1 });
    ASSERT_EQ(GetEventsWithName("EventWithRequiredLevel", sentRecords).size(), size_t{ 2 });

    FlushAndTeardown();
}

class RequestMonitor : public DebugEventListener
{
    const size_t IDX_OK   = 0;
    const size_t IDX_ERR  = 1;
    const size_t IDX_ABRT = 2;

    std::atomic<size_t> counts[3];

public:

    RequestMonitor() : DebugEventListener()
    {
        reset();
    }

    void reset()
    {
        counts[IDX_OK] = 0;
        counts[IDX_ERR] = 0;
        counts[IDX_ABRT] = 0;
    }

    virtual void OnDebugEvent(DebugEvent &evt)
    {
        switch (evt.type)
        {
            // 200 OK
        case EVT_HTTP_OK:
            counts[IDX_OK]++;
            break;
            // xxx ERROR
        case EVT_HTTP_ERROR:
            counts[IDX_ERR]++;
            break;
            // HTTP stack failure (e.g. abort)
        case EVT_HTTP_FAILURE:
            counts[IDX_ABRT]++;
            break;
        default:
            break;
        }
    }

    void dump()
    {
        printf("HTTP request stress stats:\n");
        printf("OK    = %zu\n", counts[IDX_OK].load());
        printf("ERR   = %zu\n", counts[IDX_ERR].load());
        printf("ABRT  = %zu\n", counts[IDX_ABRT].load());
    }
};

class KillSwitchListener : public DebugEventListener {
public :
    std::atomic<unsigned>   numLogged;
    std::atomic<unsigned>   numSent;
    std::atomic<unsigned>   numDropped;
    std::atomic<unsigned>   numReject;
    std::atomic<unsigned>   numHttpError;
    std::atomic<unsigned>   numHttpOK;
    std::atomic<unsigned>   numHttpFailure;
    std::atomic<unsigned>   numCached;

    KillSwitchListener() :
        numLogged(0),
        numSent(0),
        numDropped(0),
        numReject(0),
        numHttpError(0),
        numHttpOK(0),
        numHttpFailure(0),
        numCached(0)
    {
    }

    virtual void OnDebugEvent(DebugEvent &evt) {
        switch (evt.type) {
            case EVT_LOG_SESSION:
                numLogged++;
                break;
            case EVT_REJECTED:
                numReject++;
                break;
            case EVT_ADDED:
                break;
            /* Event counts below would never overflow the size of unsigned int */
            case EVT_CACHED:
                numCached += (unsigned int)evt.param1;
                break;
            case EVT_DROPPED:
                numDropped += (unsigned int)evt.param1;
                break;
            case EVT_SENT:
                numSent += (unsigned int)evt.param1;
                break;
            case EVT_HTTP_FAILURE:
                numHttpFailure++;
                break;
            case EVT_HTTP_ERROR:
                numHttpError++;
                break;
            case EVT_HTTP_OK:
                numHttpOK++;
                break;
            case EVT_UNKNOWN:
            default:
            break;
        };
    }
    void printStats(){
        std::cerr << "[          ] numLogged        = " << numLogged << std::endl;
        std::cerr << "[          ] numSent          = " << numSent << std::endl;
        std::cerr << "[          ] numDropped       = " << numDropped << std::endl;
        std::cerr << "[          ] numReject        = " << numReject << std::endl;
        std::cerr << "[          ] numHttpError     = " << numHttpError << std::endl;
        std::cerr << "[          ] numHttpOK        = " << numHttpOK << std::endl;
        std::cerr << "[          ] numHttpFailure   = " << numHttpFailure << std::endl;
        std::cerr << "[          ] numCached        = " << numCached << std::endl;
    }
};

void addListeners(DebugEventListener &listener) {
    LogManager::AddEventListener(DebugEventType::EVT_LOG_SESSION, listener);
    LogManager::AddEventListener(DebugEventType::EVT_REJECTED, listener);
    LogManager::AddEventListener(DebugEventType::EVT_SENT, listener);
    LogManager::AddEventListener(DebugEventType::EVT_DROPPED, listener);
    LogManager::AddEventListener(DebugEventType::EVT_HTTP_OK, listener);
    LogManager::AddEventListener(DebugEventType::EVT_HTTP_ERROR, listener);
    LogManager::AddEventListener(DebugEventType::EVT_HTTP_FAILURE, listener);
    LogManager::AddEventListener(DebugEventType::EVT_CACHED, listener);
}

void removeListeners(DebugEventListener &listener) {
    LogManager::RemoveEventListener(DebugEventType::EVT_LOG_SESSION, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_REJECTED, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_SENT, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_DROPPED, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_HTTP_OK, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_HTTP_ERROR, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_HTTP_FAILURE, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_CACHED, listener);
}

TEST_F(BasicFuncTests, killSwitchWorks)
{
    CleanStorage();
    // Create the configuration to send to fake server
    auto configuration = LogManager::GetLogConfiguration();

    configuration[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFF;
    configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Warn;
    configuration[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_CS;

    configuration[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;
    configuration[CFG_STR_CACHE_FILE_PATH] = TEST_STORAGE_FILENAME;
    configuration[CFG_INT_MAX_TEARDOWN_TIME] = 2;   // 2 seconds wait on shutdown
    configuration[CFG_STR_COLLECTOR_URL] = serverAddress.c_str();
    configuration[CFG_MAP_HTTP][CFG_BOOL_HTTP_COMPRESSION] = false;      // disable compression for now
    configuration[CFG_MAP_METASTATS_CONFIG]["interval"] = 30 * 60;   // 30 mins

    configuration["name"] = __FILE__;
    configuration["version"] = "1.0.0";
    configuration["config"] = { { "host", __FILE__ } }; // Host instance

    // set the killed token on the server
    server.setKilledToken(KILLED_TOKEN, 6384);
    KillSwitchListener listener;
    addListeners(listener);
    // Log 100 events from valid and invalid 4 times
    int repetitions = 4;
    for (int i = 0; i < repetitions; i++) {
        // Initialize the logger for the valid token and log 100 events
        LogManager::Initialize(TEST_TOKEN, configuration);
        LogManager::ResumeTransmission();
        auto myLogger = LogManager::GetLogger(TEST_TOKEN, "killed");
        int numIterations = 100;
        while (numIterations--) {
            EventProperties event1("fooEvent");
            event1.SetProperty("property", "value");
            myLogger->LogEvent(event1);
        }
        // Initialize the logger for the killed token and log 100 events
        LogManager::Initialize(KILLED_TOKEN, configuration);
        LogManager::ResumeTransmission();
        myLogger = LogManager::GetLogger(KILLED_TOKEN, "killed");
        numIterations = 100;
        while (numIterations--) {
            EventProperties event2("failEvent");
            event2.SetProperty("property", "value");
            myLogger->LogEvent(event2);
        }
    }
    // Try to upload and wait for 2 seconds to complete
    LogManager::UploadNow();
    PAL::sleep(2000);

    // Log 100 events with valid logger
    LogManager::Initialize(TEST_TOKEN, configuration);
    LogManager::ResumeTransmission();
    auto myLogger = LogManager::GetLogger(TEST_TOKEN, "killed");
    int numIterations = 100;
    while (numIterations--) {
        EventProperties event1("fooEvent");
        event1.SetProperty("property", "value");
        myLogger->LogEvent(event1);
    }

    LogManager::Initialize(KILLED_TOKEN, configuration);
    LogManager::ResumeTransmission();
    myLogger = LogManager::GetLogger(KILLED_TOKEN, "killed");
    numIterations = 100;
    while (numIterations--) {
        EventProperties event2("failEvent");
        event2.SetProperty("property", "value");
        myLogger->LogEvent(event2);
    }
    // Expect all events to be dropped
    EXPECT_EQ(uint32_t { 100 }, listener.numDropped);
    LogManager::FlushAndTeardown();

    listener.printStats();
    removeListeners(listener);
    server.clearKilledTokens();
}

TEST_F(BasicFuncTests, killIsTemporary)
{
    CleanStorage();
    // Create the configuration to send to fake server
    auto configuration = LogManager::GetLogConfiguration();

    configuration[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFF;
    configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Warn;
    configuration[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_CS;

    configuration[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;
    configuration[CFG_STR_CACHE_FILE_PATH] = TEST_STORAGE_FILENAME;
    configuration[CFG_INT_MAX_TEARDOWN_TIME] = 2;   // 2 seconds wait on shutdown
    configuration[CFG_STR_COLLECTOR_URL] = serverAddress.c_str();
    configuration[CFG_MAP_HTTP][CFG_BOOL_HTTP_COMPRESSION] = false;      // disable compression for now
    configuration[CFG_MAP_METASTATS_CONFIG]["interval"] = 30 * 60;   // 30 mins

    configuration["name"] = __FILE__;
    configuration["version"] = "1.0.0";
    configuration["config"] = { { "host", __FILE__ } }; // Host instance

    // set the killed token on the server
    server.setKilledToken(KILLED_TOKEN, 10);
    KillSwitchListener listener;
    addListeners(listener);
    // Log 100 events from valid and invalid 4 times
    int repetitions = 4;
    for (int i = 0; i < repetitions; i++) {
        // Initialize the logger for the valid token and log 100 events
        LogManager::Initialize(TEST_TOKEN, configuration);
        LogManager::ResumeTransmission();
        auto myLogger = LogManager::GetLogger(TEST_TOKEN, "killed");
        int numIterations = 100;
        while (numIterations--) {
            EventProperties event1("fooEvent");
            event1.SetProperty("property", "value");
            myLogger->LogEvent(event1);
        }
        // Initialize the logger for the killed token and log 100 events
        LogManager::Initialize(KILLED_TOKEN, configuration);
        LogManager::ResumeTransmission();
        myLogger = LogManager::GetLogger(KILLED_TOKEN, "killed");
        numIterations = 100;
        while (numIterations--) {
            EventProperties event2("failEvent");
            event2.SetProperty("property", "value");
            myLogger->LogEvent(event2);
        }
    }
    // Try and wait to upload
    LogManager::UploadNow();
    PAL::sleep(2000);
    // Sleep for 11 seconds so the killed time has expired, clear the killed tokens on server
    PAL::sleep(11000);
    server.clearKilledTokens();
    // Log 100 events with valid logger
    LogManager::Initialize(TEST_TOKEN, configuration);
    LogManager::ResumeTransmission();
    auto myLogger = LogManager::GetLogger(TEST_TOKEN, "killed");
    int numIterations = 100;
    while (numIterations--) {
        EventProperties event1("fooEvent");
        event1.SetProperty("property", "value");
        myLogger->LogEvent(event1);
    }

    LogManager::Initialize(KILLED_TOKEN, configuration);
    LogManager::ResumeTransmission();
    myLogger = LogManager::GetLogger(KILLED_TOKEN, "killed");
    numIterations = 100;
    while (numIterations--) {
        EventProperties event2("failEvent");
        event2.SetProperty("property", "value");
        myLogger->LogEvent(event2);
    }
    // Expect to 0 events to be dropped
    EXPECT_EQ(uint32_t { 0 }, listener.numDropped);
    LogManager::FlushAndTeardown();

    listener.printStats();
    removeListeners(listener);
    server.clearKilledTokens();
}

#ifdef _WIN32
/* TODO: [MG] - debug why this stress-test is very slow on Mac, then re-enable for Mac */
TEST_F(BasicFuncTests, sendManyRequestsAndCancel)
{
    CleanStorage();
    RequestMonitor listener;

    auto eventsList = {
        DebugEventType::EVT_HTTP_OK,
        DebugEventType::EVT_HTTP_ERROR,
        DebugEventType::EVT_HTTP_FAILURE
    };
    // Add event listeners
    for (auto evt : eventsList)
    {
        LogManager::AddEventListener(evt, listener);
    }

    for (size_t i = 0; i < 20; i++)
    {
        auto &configuration = LogManager::GetLogConfiguration();
        configuration[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;
        configuration[CFG_STR_CACHE_FILE_PATH] = TEST_STORAGE_FILENAME;
        configuration[CFG_MAP_HTTP][CFG_BOOL_HTTP_COMPRESSION] = true;
        configuration[CFG_STR_COLLECTOR_URL] = COLLECTOR_URL_PROD;
        configuration[CFG_INT_MAX_TEARDOWN_TIME] = (int64_t)(i % 2);
        configuration[CFG_INT_TRACE_LEVEL_MASK] = 0;
        configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Warn;
        LogManager::Initialize(TEST_TOKEN);
        auto myLogger = LogManager::GetLogger();
        for (size_t j = 0; j < 200; j++)
        {
            EventProperties myEvent1("sample_realtime");
            myEvent1.SetLatency(EventLatency_RealTime);
            myLogger->LogEvent(myEvent1);
            EventProperties myEvent2("sample_max");
            myEvent2.SetLatency(EventLatency_Max);
            myLogger->LogEvent(myEvent2);
        }
        // force upload
        LogManager::UploadNow();
        if ((i % 3) == 0)
        {
            PAL::sleep(100);
        }
        if (i % 2)
        {
            size_t k = rand() % 10 + 1;
            while (k--)
            {
                std::this_thread::yield();
            }
        }
        LogManager::FlushAndTeardown();
    }

    listener.dump();
    // Remove event listeners
    for (auto evt : eventsList)
    {
        LogManager::RemoveEventListener(evt, listener);
    }
}

#define MAX_TEST_RETRIES 10

TEST_F(BasicFuncTests, raceBetweenUploadAndShutdownMultipleLogManagers)
{
    CleanStorage();

    RequestMonitor listener;
    auto eventsList = {
        DebugEventType::EVT_HTTP_OK,
        DebugEventType::EVT_HTTP_ERROR,
        DebugEventType::EVT_HTTP_FAILURE};
    // Add event listeners
    for (auto evt : eventsList)
    {
        LogManagerA::AddEventListener(evt, listener);
        LogManagerB::AddEventListener(evt, listener);
    };

    // string values in ILogConfiguration must stay immutable for the duration of the run
    {   // LogManager A
        auto& configuration = LogManagerA::GetLogConfiguration();
        configuration[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;
        configuration[CFG_STR_CACHE_FILE_PATH] = TEST_STORAGE_FILENAME;
        configuration[CFG_MAP_HTTP][CFG_BOOL_HTTP_COMPRESSION] = true;
        configuration[CFG_STR_COLLECTOR_URL] = COLLECTOR_URL_PROD;
        configuration[CFG_INT_MAX_TEARDOWN_TIME] = 1;
        configuration[CFG_INT_TRACE_LEVEL_MASK] = 0;
        configuration["name"] = "LogManagerA";
        configuration["version"] = "1.0.0";
        configuration["config"]["host"] = "LogManagerA";
    }
    {   // LogManager B
        auto& configuration = LogManagerB::GetLogConfiguration();
        configuration[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;
        configuration[CFG_STR_CACHE_FILE_PATH] = TEST_STORAGE_FILENAME;
        configuration[CFG_MAP_HTTP][CFG_BOOL_HTTP_COMPRESSION] = true;
        configuration[CFG_STR_COLLECTOR_URL] = COLLECTOR_URL_PROD;
        configuration[CFG_INT_MAX_TEARDOWN_TIME] = 1;
        configuration[CFG_INT_TRACE_LEVEL_MASK] = 0;
        configuration["name"] = "LogManagerB";
        configuration["version"] = "1.0.0";
        configuration["config"]["host"] = "LogManagerB";
    }

    std::atomic<bool> testRunning(true);
    auto t = std::thread([&]() {
        while (testRunning)
        {
            // Abuse LogManagerA and LogManagerB badly.
            // Both may or may not have a valid implementation instance.
            // PAL could be dead while this abuse is happening.
            LogManagerA::UploadNow();
            LogManagerB::UploadNow();
        }
    });

    for (size_t i = 0; i < MAX_TEST_RETRIES; i++)
    {
        auto loggerA = LogManagerA::Initialize(TEST_TOKEN);
        EXPECT_EQ(PAL::PALTest::GetPalRefCount(), 1);

        auto loggerB = LogManagerB::Initialize(TEST_TOKEN);
        EXPECT_EQ(PAL::PALTest::GetPalRefCount(), 2);

        EventProperties evtCritical("BasicFuncTests.stress_test_critical_A");
        evtCritical.SetPriority(EventPriority_Immediate);
        evtCritical.SetPolicyBitFlags(MICROSOFT_EVENTTAG_CORE_DATA | MICROSOFT_EVENTTAG_REALTIME_LATENCY | MICROSOFT_KEYWORD_CRITICAL_DATA);
        loggerA->LogEvent("BasicFuncTests.stress_test_A");
        LogManagerA::UploadNow();

        loggerB->LogEvent("BasicFuncTests.stress_test_B");
        LogManagerB::UploadNow();

        EXPECT_EQ(LogManagerB::FlushAndTeardown(), STATUS_SUCCESS);
        EXPECT_EQ(PAL::PALTest::GetPalRefCount(), 1);

        EXPECT_EQ(LogManagerA::FlushAndTeardown(), STATUS_SUCCESS);
        EXPECT_EQ(PAL::PALTest::GetPalRefCount(), 0);
    }

    testRunning = false;
    try
    {
        t.join();
    }
    catch (std::exception)
    {
        // catch exception if can't join because the thread is already gone
    };
    listener.dump();
    // Remove event listeners
    for (auto evt : eventsList)
    {
        LogManagerB::RemoveEventListener(evt, listener);
        LogManagerA::RemoveEventListener(evt, listener);
    }
    CleanStorage();
}
#endif

TEST_F(BasicFuncTests, logManager_getLogManagerInstance_uninitializedReturnsNull)
{
    auto lm = LogManager::GetInstance();
    EXPECT_EQ(lm,nullptr);
}

TEST_F(BasicFuncTests, logManager_getLogManagerInstance_initializedReturnsNonnull)
{
    LogManager::Initialize();
    auto lm = LogManager::GetInstance();
    EXPECT_NE(lm,nullptr);
    LogManager::FlushAndTeardown();
}

#ifndef ANDROID
TEST_F(BasicFuncTests, deleteEvents)
{
    CleanStorage();
    Initialize();

    const size_t  max_events = 10;
    size_t iteration = 0;

    // pause the transmission so events get collected in storage
    LogManager::PauseTransmission();
    std::string eventset1 = "EventSet1_";
    std::vector<EventProperties> events1;
    while ( iteration++ < max_events )
    {
        EventProperties event(eventset1 + std::to_string(iteration));
        event.SetPriority(EventPriority_Normal);
        event.SetProperty("property1", "value1");
        event.SetProperty("property2", "value2");
        events1.push_back(event);
        logger->LogEvent(event);
    }
    LogManager::DeleteData();
    LogManager::ResumeTransmission();
    LogManager::UploadNow(); //forc upload if something is there in local storage
    PAL::sleep(2000) ; //wait for some time.
    for (auto &e: events1) {
        ASSERT_EQ(find(e.GetName()).name, "");
    }

    std::vector<EventProperties> events2;
    std::string eventset2 = "EventSet2_";
    iteration = 0;

    while ( iteration++ < max_events )
    {
        EventProperties event(eventset2 + std::to_string(iteration));
        event.SetPriority(EventPriority_Normal);
        event.SetProperty("property1", "value1");
        event.SetProperty("property2", "value2");
        events2.push_back(event);
        logger->LogEvent(event);
    }
    LogManager::UploadNow(); //forc upload if something is there in local storage
    waitForEvents(3 /*timeout*/, max_events /*expected count*/); 
    for (auto &e: events2) {
        verifyEvent(e, find(e.GetName()));
    }
}
#endif

#if 0 // TODO: [MG] - re-enable this long-haul test
TEST_F(BasicFuncTests, serverProblemsDropEventsAfterMaxRetryCount)
{
    CleanStorage();

    auto &configuration = LogManager::GetLogConfiguration();

    std::string badServiceUrl;
    badServiceUrl.insert(badServiceUrl.find('/', sizeof("http://")) + 1, "503/");

    configuration[CFG_STR_COLLECTOR_URL] = badServiceUrl.c_str();

    {
        Initialize();

        EventProperties event("event");
        event.SetProperty("property", "value");

        logger->LogEvent(event);

        // After initial delay of 2 seconds, the library will send a request, wait 3 seconds, send 1st retry and stop.
        // 2nd retry after another 3 seconds (using the good URL again) should not come - wait 1 more second to be sure.
        PAL::sleep(2000 + 2 * 3000 + 1000);
        // EXPECT_THAT(receivedRequests, SizeIs(0));

         // Check meta stats on restart (will be first request)
        FlushAndTeardown();
    }

    // Restore fast URL
    configuration[CFG_STR_COLLECTOR_URL] = serverAddress.c_str();

    {
        configuration[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;
        configuration[CFG_STR_CACHE_FILE_PATH] = TEST_STORAGE_FILENAME;
        Initialize();
        waitForEvents(5, 2);
        if (receivedRequests.size())
        {
            auto payload = decodeRequest(receivedRequests[receivedRequests.size() - 1], false);
            /*    auto const& dp = payload.TokenToDataPackagesMap["metastats-tenant-token"][0];
                ASSERT_THAT(payload, SizeIs(1));
                EXPECT_THAT(payload[0].Id, Not(IsEmpty()));
                EXPECT_THAT(payload[0].Type, Eq("client_telemetry"));
                EXPECT_THAT(payload[0].Extension, Contains(Pair("stats_rollup_kind", "stop")));
                EXPECT_THAT(payload[0].Extension, Contains(Pair("records_dropped_retry_exceeded", "2")));
                */
        }
        FlushAndTeardown();
    }
}
#endif
#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT

