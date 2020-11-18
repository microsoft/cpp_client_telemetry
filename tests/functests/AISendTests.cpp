//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

#ifdef HAVE_MAT_AI
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#endif
#include "common/Common.hpp"
#include "common/HttpServer.hpp"

#include <utils/ZlibUtils.hpp>

#include "LogManager.hpp"

#include "json.hpp"

using namespace testing;

#undef LOCKGUARD
#define LOCKGUARD(macro_mutex) std::lock_guard<decltype(macro_mutex)> TOKENPASTE2(__guard_, __LINE__)(macro_mutex);

// Application Insights test key
#define TEST_TOKEN "12345678-1234-1234-1234-123456789abc"
#define HTTP_PORT 19002
char const* const TEST_STORAGE_FILENAME = "AISendTests.db";

class AITestDebugEventListener : public DebugEventListener
{
    const size_t IDX_OK   = 0;
    const size_t IDX_FAIL = 1;
    const size_t IDX_PART = 2;
    const size_t IDX_ERR = 3;
    const size_t IDX_ADDED = 4;

    std::atomic<size_t> counts[5];

   public:

    AITestDebugEventListener() 
    {
        counts[IDX_OK] = 0;
        counts[IDX_FAIL] = 0;
        counts[IDX_PART] = 0;
        counts[IDX_ERR] = 0;
        counts[IDX_ADDED] = 0;
    }

    virtual void OnDebugEvent(DebugEvent &evt)
    {
        switch (evt.type) {
        case EVT_HTTP_FAILURE:
            counts[IDX_FAIL]++;
            break;
        case EVT_HTTP_OK:
            counts[IDX_OK]++;
            break;
        case EVT_HTTP_ERROR:
            counts[IDX_ERR]++;
            break;
        case EVT_HTTP_STATE:
            if (evt.param1 == 206) {
                counts[IDX_PART]++;
            }
            break;
        case EVT_ADDED: 
			counts[IDX_ADDED]++;
            break;
        default:
            break;
        };
    };

    unsigned total()
    {
        return counts[IDX_ADDED] + counts[IDX_OK] + counts[IDX_FAIL] + counts[IDX_PART] + counts[IDX_ERR];
    }

    void expect(unsigned added, unsigned ok, unsigned failed, unsigned partial, unsigned error)
    {
        EXPECT_EQ(added, counts[IDX_ADDED]);
        EXPECT_GE(ok, counts[IDX_OK]);
        EXPECT_EQ(failed, counts[IDX_FAIL]);
        EXPECT_EQ(partial, counts[IDX_PART]);
        EXPECT_EQ(error, counts[IDX_ERR]);
    }
};

class AISendTests : public ::testing::Test,
                    public HttpServer::Callback
{
   protected:
    std::mutex mtx_requests;
    std::vector<HttpServer::Request> receivedRequests;
    std::string serverAddress;
    HttpServer server;

    ILogger* logger;

    std::atomic<bool> isSetup;
    std::atomic<bool> isRunning;

    std::condition_variable cv_gotEvents;
    std::mutex cv_m;

   public:
    AISendTests() :
        isSetup(false),
        isRunning(false){};

    virtual void SetUp() override
    {
        if (isSetup.exchange(true))
        {
            return;
        }
        int port = server.addListeningPort(HTTP_PORT);
        std::ostringstream os;
        os << "localhost:" << port;
        serverAddress = "http://" + os.str() + "/v2/track";
        server.setServerName(os.str());
        server.addHandler("/v2/track", *this);
        server.addHandler("/", *this);
        server.setKeepalive(false);  // This test doesn't work well with keep-alive enabled
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

    virtual void Initialize(DebugEventListener& debugListener, std::string const& path, bool compression)
    {
        receivedRequests.clear();
        auto configuration = LogManager::GetLogConfiguration();
        configuration[CFG_INT_SDK_MODE] = SdkModeTypes_AI;
        configuration[CFG_STR_COLLECTOR_URL] = (serverAddress + path).c_str();
        configuration[CFG_MAP_HTTP][CFG_BOOL_HTTP_COMPRESSION] = compression;

        configuration[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFF;
#ifdef NDEBUG
        configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Warn;
#else
        configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Trace;
#endif

        configuration[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;
        configuration[CFG_STR_CACHE_FILE_PATH] = TEST_STORAGE_FILENAME;
        configuration[CFG_INT_MAX_TEARDOWN_TIME] = 2;  // 2 seconds wait on shutdown

        configuration["name"] = __FILE__;
        configuration["version"] = "1.0.0";
        configuration["config"] = {{"host", __FILE__}};  // Host instance

        LogManager::Initialize(TEST_TOKEN, configuration);
        LogManager::SetLevelFilter(DIAG_LEVEL_DEFAULT, {DIAG_LEVEL_DEFAULT_MIN, DIAG_LEVEL_DEFAULT_MAX});
        LogManager::ResumeTransmission();

        LogManager::AddEventListener(DebugEventType::EVT_HTTP_OK, debugListener);
        LogManager::AddEventListener(DebugEventType::EVT_HTTP_ERROR, debugListener);
        LogManager::AddEventListener(DebugEventType::EVT_HTTP_FAILURE, debugListener);
        LogManager::AddEventListener(DebugEventType::EVT_HTTP_STATE, debugListener);
        LogManager::AddEventListener(DebugEventType::EVT_ADDED, debugListener);

        logger = LogManager::GetLogger(TEST_TOKEN);
    }

    virtual void FlushAndTeardown(DebugEventListener& debugListener)
    {
        LogManager::Flush();

        LogManager::RemoveEventListener(DebugEventType::EVT_HTTP_OK, debugListener);
        LogManager::RemoveEventListener(DebugEventType::EVT_HTTP_ERROR, debugListener);
        LogManager::RemoveEventListener(DebugEventType::EVT_HTTP_FAILURE, debugListener);
        LogManager::RemoveEventListener(DebugEventType::EVT_HTTP_STATE, debugListener);
        LogManager::RemoveEventListener(DebugEventType::EVT_ADDED, debugListener);

        LogManager::FlushAndTeardown();

        auto &configuration = LogManager::GetLogConfiguration();
        configuration[CFG_INT_SDK_MODE] = SdkModeTypes_CS;
    }

    virtual int onHttpRequest(HttpServer::Request const& request, HttpServer::Response& response) override
    {
        {
            LOCKGUARD(mtx_requests);
            receivedRequests.push_back(request);
        }

        if (request.uri.compare("/v2/track/400/") == 0)
        {
            return 400;
        }

        if (request.uri.compare("/v2/track/500/") == 0)
        {
            return 500;
        }

        if (request.uri.compare("/v2/track/206/errors") == 0)
        {
            nlohmann::json errors = nlohmann::json::array({
                { {"index", 0 }, {"statusCode", 500} }
            });
            nlohmann::json content = {
                {"itemsReceived", 2},
                {"itemsAccepted", 1}, 
                {"errors", errors }
            };
            response.headers["Content-Type"] = "application/json";
            response.content = content.dump();
            return 206;
        }

        if (request.uri.compare("/v2/track/206/") == 0)
        {
            nlohmann::json content = {
                {"itemsReceived", 2},
                {"itemsAccepted", 1}, 
                {"errors", nlohmann::json::array() }
            };
            response.headers["Content-Type"] = "application/json";
            response.content = content.dump();
            return 206;
        }

        nlohmann::json content = {
            { "itemsReceived" , 1 },
            { "itemsAccepted" , 1 }
        };
        response.headers["Content-Type"] = "application/json";
        response.content = content.dump();

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

    void waitForEvents(unsigned timeOutSec, unsigned expectedRequests, bool compression)
    {
        size_t receivedEvents = 0;
        unsigned timeoutMs = 1000 * timeOutSec;
        auto start = PAL::getUtcSystemTimeMs();
        while (((PAL::getUtcSystemTimeMs() - start) < timeoutMs) 
            && (receivedEvents != expectedRequests))
        {
            /* Give time for our friendly HTTP server thread to process incoming request */
            std::this_thread::yield();
            {
                LOCKGUARD(mtx_requests);
                if (receivedRequests.size())
                {
                    auto request = receivedRequests.at(0);
                    nlohmann::json body;
                    auto it = request.headers.find("Content-Encoding");
                    if (it != request.headers.end())
                    {
                        EXPECT_TRUE(compression);
                        std::vector<uint8_t> content(request.content.begin(), request.content.end());
                        std::vector<uint8_t> inflated;
                        ZlibUtils::InflateVector(content, inflated, true);
                        body = nlohmann::json::parse(inflated.begin(), inflated.end());
                    }
                    else
                    {
                        EXPECT_FALSE(compression);
                        body = nlohmann::json::parse(request.content.begin(), request.content.end());
                    }
                    EXPECT_TRUE(body.is_array());
                    receivedEvents += body.size();
                }
            }
        }
        ASSERT_EQ(receivedEvents, expectedRequests);
    }

    void waitForResponse(unsigned timeOutSec, unsigned expectedDebugEvents, AITestDebugEventListener& debugListener)
    {
        unsigned receivedDebugEvents = 0;
        unsigned timeoutMs = 1000 * timeOutSec;
        auto start = PAL::getUtcSystemTimeMs();
        while (((PAL::getUtcSystemTimeMs() - start) < timeoutMs) 
            && (receivedDebugEvents != expectedDebugEvents))
        {
            /* Give time for our friendly HTTP server thread to process incoming request */
            std::this_thread::yield();
            {
                receivedDebugEvents = debugListener.total();
            }
        }
        ASSERT_EQ(receivedDebugEvents, expectedDebugEvents);
    }

    void validateRequest(EventProperties const& expectedEvent, bool compressed)
    {
        HttpServer::Request request;
        {
            LOCKGUARD(mtx_requests);
            request = receivedRequests.at(0);
        }

        nlohmann::json body;
        if (compressed)
        {
            std::vector<uint8_t> content(request.content.begin(), request.content.end());
            std::vector<uint8_t> inflated;
            ZlibUtils::InflateVector(content, inflated, true);
            body = nlohmann::json::parse(inflated.begin(), inflated.end());
        }
        else
        {
            body = nlohmann::json::parse(request.content.begin(), request.content.end());
        }
        EXPECT_TRUE(body.is_array());
        nlohmann::json actualEvent = body[0];
        EXPECT_TRUE(actualEvent.is_object());
        auto dbg = actualEvent.dump();
        auto actualToken = actualEvent["iKey"].get<std::string>();
        auto actualBaseType = actualEvent["data"]["baseType"].get<std::string>();
        auto actualEventName = actualEvent["data"]["baseData"]["name"].get<std::string>();
        nlohmann::json actualProperties = actualEvent["data"]["baseData"]["properties"];

        EXPECT_EQ(TEST_TOKEN, actualToken);
        EXPECT_EQ("EventData", actualBaseType);
        EXPECT_EQ(expectedEvent.GetName(), actualEventName);

        for (auto& kv : expectedEvent.GetProperties())
        {
            const auto& k = kv.first;
            const auto& v = kv.second;
            nlohmann::json actualProperty = actualProperties[k];
            EXPECT_TRUE(actualProperty.is_primitive());

            switch (v.type)
            {
            case EventProperty::TYPE_STRING:
            {
                EXPECT_EQ(v.as_string, actualProperty.get<std::string>());
                break;
            }
            case EventProperty::TYPE_INT64:
            {
                EXPECT_EQ(v.as_int64, actualProperty.get<int>());
                break;
            }
            default:
                FAIL() << "Unknown value type" << v.type;
            }
        }
    }
};

TEST_F(AISendTests, sendOneEvent)
{
    AITestDebugEventListener debugListener;
    bool compression = false;

    CleanStorage();
    Initialize(debugListener, "", compression);

    EventProperties event("ai_event");
    event.SetProperty("property", "value");
    logger->LogEvent(event);
    waitForEvents(5, 1, compression);
    waitForResponse(5, 2, debugListener);
    FlushAndTeardown(debugListener);
    EXPECT_GE(receivedRequests.size(), (size_t)1);  // at least 1 HTTP request with customer payload and stats
    validateRequest(event, compression);
    debugListener.expect(1, 1, 0, 0, 0);
}

TEST_F(AISendTests, sendMultipleEvent)
{
    AITestDebugEventListener debugListener;
    bool compression = false;

    CleanStorage();
    Initialize(debugListener, "", compression);

    EventProperties event1("ai_event1");
    event1.SetProperty("property", "value1");
    logger->LogEvent(event1);
    EventProperties event2("ai_event2");
    event1.SetProperty("property", "value2");
    logger->LogEvent(event2);
    EventProperties event3("ai_event3");
    event1.SetProperty("property", "value3");
    logger->LogEvent(event3);

    waitForEvents(5, 3, compression);
    waitForResponse(5, 4, debugListener);
    FlushAndTeardown(debugListener);

    EXPECT_GE(receivedRequests.size(), (size_t)1);  // at least 1 HTTP request with customer payload and stats
    debugListener.expect(3, 1, 0, 0, 0);
}

TEST_F(AISendTests, receiveServerError)
{
    AITestDebugEventListener debugListener;
    bool compression = false;

    CleanStorage();
    Initialize(debugListener, "/500/", compression);

    EventProperties event("ai_event");
    event.SetProperty("property", "value");
    logger->LogEvent(event);
    waitForEvents(5, 1, compression);
    waitForResponse(5, 2, debugListener);

    FlushAndTeardown(debugListener);
    EXPECT_GE(receivedRequests.size(), (size_t)1);  // at least 1 HTTP request with customer payload and stats
    validateRequest(event, compression);
    debugListener.expect(1, 0, 1, 0, 0);
}

TEST_F(AISendTests, receivePartialSuccess)
{
    AITestDebugEventListener debugListener;
    bool compression = false;

    CleanStorage();
    Initialize(debugListener, "/206/", compression);

    EventProperties event("ai_event");
    event.SetProperty("property", "value");
    logger->LogEvent(event);
    waitForEvents(5, 1, compression);
    waitForResponse(5, 2, debugListener);
    FlushAndTeardown(debugListener);
    EXPECT_GE(receivedRequests.size(), (size_t)1);  // at least 1 HTTP request with customer payload and stats
    validateRequest(event, compression);
    debugListener.expect(1, 0, 0, 1, 0);
}

TEST_F(AISendTests, receivePartialCompressedSuccess)
{
    AITestDebugEventListener debugListener;
    bool compression = true;

    CleanStorage();
    Initialize(debugListener, "/206/errors", compression);

    EventProperties event("ai_event");
    event.SetProperty("property", "value");
    logger->LogEvent(event);
    waitForEvents(5, 1, compression);
    waitForResponse(20, 3, debugListener);

    FlushAndTeardown(debugListener);
    EXPECT_GE(receivedRequests.size(), (size_t)1);  // at least 1 HTTP request with customer payload and stats
    validateRequest(event, compression);
    debugListener.expect(2, 0, 0, 1, 0);
}

TEST_F(AISendTests, receiveServerRejected)
{
    AITestDebugEventListener debugListener;
    bool compression = false;

    CleanStorage();
    Initialize(debugListener, "/400/", compression);

    EventProperties event("ai_event");
    event.SetProperty("property", "value");
    logger->LogEvent(event);
    waitForEvents(5, 1, compression);
    waitForResponse(5, 2, debugListener);

    FlushAndTeardown(debugListener);
    EXPECT_GE(receivedRequests.size(), (size_t)1);  // at least 1 HTTP request with customer payload and stats
    validateRequest(event, compression);
    debugListener.expect(1, 0, 0, 0, 1);
}

TEST_F(AISendTests, sendCompressedEvent)
{
    AITestDebugEventListener debugListener;
    bool compression = true;

    CleanStorage();
    Initialize(debugListener, "", compression);

    EventProperties event("ai_event");
    event.SetProperty("property", "value");
    logger->LogEvent(event);
    waitForEvents(5, 1, compression);
    waitForResponse(5, 2, debugListener);
    FlushAndTeardown(debugListener);
    EXPECT_GE(receivedRequests.size(), (size_t)1);  // at least 1 HTTP request with customer payload and stats
    validateRequest(event, compression);
    debugListener.expect(1, 1, 0, 0, 0);
}

#endif  // HAVE_MAT_AI
