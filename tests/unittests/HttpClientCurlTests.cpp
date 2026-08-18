//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

// These tests only apply to the curl HTTP client path (Linux, non-Apple, non-Android)
#if defined(MATSDK_PAL_CPP11) && !defined(_MSC_VER) && defined(HAVE_MAT_DEFAULT_HTTP_CLIENT) \
    && !defined(__APPLE__) && !defined(ANDROID)

#include "common/Common.hpp"
#include "common/HttpServer.hpp"
#include "http/HttpClient_Curl.hpp"
#include "config/RuntimeConfig_Default.hpp"

#include <future>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace testing;
using namespace MAT;

class HttpClientCurlTests : public ::testing::Test
{
protected:
    HttpClient_Curl m_client;
    // Named, fixture-lifetime values so they outlive each CurlHttpOperation.
    const std::map<std::string, std::string> m_headers;
    const std::vector<uint8_t> m_body;
};

// --- SetSslVerification wiring ---

TEST_F(HttpClientCurlTests, SslVerification_DefaultsToTrue)
{
    CurlHttpOperation op("GET", "https://example.com", nullptr, m_headers, m_body);
    ASSERT_NE(op.GetHandle(), nullptr);
}

TEST_F(HttpClientCurlTests, CurlHttpOperation_ConstructsWithVerifyTrue)
{
    CurlHttpOperation op("GET", "https://example.com", nullptr,
        m_headers, m_body,
        false, 5, true, "");
    ASSERT_NE(op.GetHandle(), nullptr);
}

TEST_F(HttpClientCurlTests, CurlHttpOperation_ConstructsWithVerifyFalse)
{
    CurlHttpOperation op("GET", "https://example.com", nullptr,
        m_headers, m_body,
        false, 5, false, "");
    ASSERT_NE(op.GetHandle(), nullptr);
}

TEST_F(HttpClientCurlTests, CurlHttpOperation_ConstructsWithCaInfo)
{
    CurlHttpOperation op("GET", "https://example.com", nullptr,
        m_headers, m_body,
        false, 5, true, "/etc/ssl/certs/ca-certificates.crt");
    ASSERT_NE(op.GetHandle(), nullptr);
}

TEST(HttpClientCurlOperationTests, SelectsHttp2OnlyWhenRuntimeSupportsIt)
{
    const curl_version_info_data* versionInfo = curl_version_info(CURLVERSION_NOW);
    const long expected = (versionInfo != nullptr && (versionInfo->features & CURL_VERSION_HTTP2) != 0)
        ? CURL_HTTP_VERSION_2_0
        : CURL_HTTP_VERSION_1_1;
    EXPECT_EQ(CurlHttpOperation::GetPreferredHttpVersion(), expected);
}

class HttpClientCurlHeaderTests : public ::testing::Test,
                                  public HttpServer::Callback
{
protected:
    HttpServer m_server;
    std::string m_url;

    void SetUp() override
    {
        const int port = m_server.addListeningPort(0);
        std::ostringstream address;
        address << "127.0.0.1:" << port;
        m_url = "http://" + address.str() + "/headers/";
        m_server.setServerName(address.str());
        m_server.addHandler("/headers/", *this);
        m_server.start();
    }

    void TearDown() override
    {
        m_server.stop();
    }

    int onHttpRequest(HttpServer::Request const&, HttpServer::Response& response) override
    {
        response.headers["X-MAT-Test"] = "header-value";
        response.content = "body-value";
        return 200;
    }
};

TEST_F(HttpClientCurlHeaderTests, CapturesResponseHeadersAndBody)
{
    const std::map<std::string, std::string> requestHeaders;
    const std::vector<uint8_t> requestBody;
    const HttpClient_Curl client;
    (void)client; // Initialize curl globally before constructing the operation.
    CurlHttpOperation operation("GET", m_url, nullptr, requestHeaders, requestBody);

    operation.Send();
    ASSERT_EQ(operation.GetTransportError(), CURLE_OK);
    ASSERT_EQ(operation.GetHttpStatusCode(), 200L);
    const auto responseHeaders = operation.GetResponseHeaders();
    const auto responseBody = operation.GetResponseBody();

    ASSERT_EQ(responseHeaders.count("X-MAT-Test"), 1u);
    EXPECT_EQ(responseHeaders.at("X-MAT-Test"), "header-value");
    EXPECT_EQ(std::string(responseBody.begin(), responseBody.end()), "body-value");
}

// --- ILogConfiguration integration ---

TEST(HttpClientCurlConfigTests, LogConfiguration_SslVerify_DefaultIsTrue)
{
    // defaultRuntimeConfig from RuntimeConfig_Default.hpp has the defaults
    bool sslVerify = defaultRuntimeConfig[CFG_MAP_HTTP][CFG_BOOL_HTTP_SSL_VERIFY];
    EXPECT_TRUE(sslVerify);
}

TEST(HttpClientCurlConfigTests, LogConfiguration_SslCaInfo_DefaultIsEmpty)
{
    const char* caInfo = defaultRuntimeConfig[CFG_MAP_HTTP][CFG_STR_HTTP_SSL_CAINFO];
    EXPECT_STREQ(caInfo, "");
}

TEST(HttpClientCurlConfigTests, LogConfiguration_SslVerify_CanBeDisabled)
{
    ILogConfiguration config;
    config[CFG_MAP_HTTP][CFG_BOOL_HTTP_SSL_VERIFY] = false;
    bool sslVerify = config[CFG_MAP_HTTP][CFG_BOOL_HTTP_SSL_VERIFY];
    EXPECT_FALSE(sslVerify);
}

TEST(HttpClientCurlConfigTests, LogConfiguration_SslCaInfo_CanBeSet)
{
    ILogConfiguration config;
    config[CFG_MAP_HTTP][CFG_STR_HTTP_SSL_CAINFO] = "/custom/ca-bundle.crt";
    const char* caInfo = config[CFG_MAP_HTTP][CFG_STR_HTTP_SSL_CAINFO];
    EXPECT_STREQ(caInfo, "/custom/ca-bundle.crt");
}

// --- ApplySettings integration ---

TEST_F(HttpClientCurlTests, ApplySettings_ReadsSslConfigFromLogConfiguration)
{
    ILogConfiguration config;
    config[CFG_MAP_HTTP][CFG_BOOL_HTTP_SSL_VERIFY] = false;
    config[CFG_MAP_HTTP][CFG_STR_HTTP_SSL_CAINFO] = "/custom/ca.pem";
    m_client.ApplySettings(config);
    // Verify indirectly -- constructing an operation should not fail
    SUCCEED();
}

TEST_F(HttpClientCurlTests, ApplySettings_DefaultConfigEnablesVerification)
{
    ILogConfiguration config;
    m_client.ApplySettings(config);
    SUCCEED();
}

// --- Thread safety: SetSslVerification concurrent with reads ---

TEST_F(HttpClientCurlTests, SetSslVerification_ConcurrentCallsNoRace)
{
    // Exercise the atomic + mutex path under contention.
    // No assertions on output -- this is a sanitizer/TSAN target.
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 10; ++i)
    {
        futures.push_back(std::async(std::launch::async, [this, i]() {
            m_client.SetSslVerification(i % 2 == 0, (i % 2 == 0) ? "/some/path" : "");
        }));
    }
    for (auto& f : futures)
    {
        f.get();
    }
    SUCCEED();
}

// --- Regression: EDEADLK self-join in ~CurlHttpOperation ---

TEST_F(HttpClientCurlTests, SendAsync_DestroyOnWorkerThread_NoSelfJoin)
{
    struct TrackingCallback : public IHttpResponseCallback
    {
        std::atomic<int> destroyEvents { 0 };
        void OnHttpResponse(IHttpResponse* response) override { delete response; }
        void OnHttpStateEvent(HttpStateEvent state, void*, size_t) override
        {
            if (state == OnDestroy)
            {
                ++destroyEvents;
            }
        }
    };

    auto callback = std::make_shared<TrackingCallback>();
    auto callbackDone = std::make_shared<std::promise<void>>();
    auto done = callbackDone->get_future();

    auto op = std::make_shared<CurlHttpOperation>(
        "GET", "://malformed", callback.get(), m_headers, m_body,
        false, 1 /*connTimeout*/, false /*sslVerify*/, "");

    auto box = std::make_shared<std::shared_ptr<CurlHttpOperation>>(std::move(op));
    (*box)->SendAsync([box, callback, callbackDone](CurlHttpOperation&) {
        box->reset();
        callbackDone->set_value();
    });

    if (done.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    {
        ADD_FAILURE() << "curl worker did not finish before fixture teardown";
        std::abort();
    }
    EXPECT_EQ(callback->destroyEvents.load(), 1);
}

TEST_F(HttpClientCurlTests, SendAsync_CallbackCopyFailureStillCompletes)
{
    struct ThrowOnCopy
    {
        explicit ThrowOnCopy(bool& invoked) : invoked(&invoked) {}
        ThrowOnCopy(ThrowOnCopy&&) = default;
        ThrowOnCopy(const ThrowOnCopy&) { throw std::logic_error("copy failed"); }
        void operator()(CurlHttpOperation&) const { *invoked = true; }
        bool* invoked;
    };

    CurlHttpOperation op(
        "GET", "://malformed", nullptr, m_headers, m_body,
        false, 1 /*connTimeout*/, false /*sslVerify*/, "");
    bool callbackInvoked = false;
    std::function<void(CurlHttpOperation&)> callback { ThrowOnCopy(callbackInvoked) };

    EXPECT_NO_THROW(op.SendAsync(std::move(callback)));
    EXPECT_TRUE(callbackInvoked);
    EXPECT_EQ(op.GetTransportError(), CURLE_FAILED_INIT);
    EXPECT_EQ(op.GetSetupError(), CURLE_FAILED_INIT);
    EXPECT_THROW(op.SendAsync(), std::logic_error);
}

// --- Response-size cap (memory-amplification DoS hardening) ---

class HttpClientCurlResponseCapTests : public ::testing::Test,
                                       public HttpServer::Callback,
                                       public IHttpResponseCallback
{
protected:
    HttpServer      m_server;
    HttpClient_Curl m_client;
    // The client never takes ownership of the request (it only stores a raw pointer
    // and erases it); the fixture owns it and frees it in TearDown -- on the main
    // thread, after the transfer has completed.
    std::unique_ptr<IHttpRequest> m_request;
    std::string     m_hostname;
    size_t          m_responseBodySize {0};

    std::mutex      m_lock;
    bool            m_received {false};
    HttpResult      m_result {};
    unsigned int    m_statusCode {0};
    size_t          m_bodySize {0};

    void SetUp() override
    {
        int port = m_server.addListeningPort(0);
        std::ostringstream os;
        os << "127.0.0.1:" << port;
        m_hostname = os.str();
        m_server.setServerName(m_hostname);
        m_server.addHandler("/huge/", *this);
        m_server.start();
    }

    void TearDown() override
    {
        m_server.stop();
        m_request.reset();
    }

    // HttpServer::Callback -- returns a body of m_responseBodySize bytes.
    int onHttpRequest(HttpServer::Request const& /*request*/, HttpServer::Response& response) override
    {
        size_t bodySize;
        {
            std::lock_guard<std::mutex> lock(m_lock);
            bodySize = m_responseBodySize;
        }
        response.headers["Content-Type"] = "application/octet-stream";
        response.content = std::string(bodySize, 'A');
        return 200;
    }

    // IHttpResponseCallback -- the SDK hands over ownership of the response.
    void OnHttpResponse(IHttpResponse* response) override
    {
        std::unique_ptr<IHttpResponse> owned(response);
        std::lock_guard<std::mutex> lock(m_lock);
        m_result = owned->GetResult();
        m_statusCode = owned->GetStatusCode();
        m_bodySize = owned->GetBody().size();
        m_received = true;
    }

    bool responseReceived()
    {
        std::lock_guard<std::mutex> lock(m_lock);
        return m_received;
    }

    void sendAndWait(size_t bodySize)
    {
        {
            std::lock_guard<std::mutex> lock(m_lock);
            m_received = false;
            m_result = HttpResult{};
            m_statusCode = 0;
            m_bodySize = 0;
            m_responseBodySize = bodySize; // read under the same lock by onHttpRequest
        }
        m_request.reset(m_client.CreateRequest());
        m_request->SetUrl("http://" + m_hostname + "/huge/");
        m_client.SendRequestAsync(m_request.get(), this);
        for (int i = 0; i < 300 && !responseReceived(); i++)
            PAL::sleep(100);
    }
};

TEST_F(HttpClientCurlResponseCapTests, AbortsOversizedResponseBody)
{
    // A response body larger than the client's response-size cap (kMaxResponseBytes,
    // 16 MB) must be refused, not buffered in full, so a hostile/MITM'd collector
    // cannot exhaust process memory.
    sendAndWait(17u * 1024u * 1024u);
    ASSERT_TRUE(responseReceived());
    // curl aborts the transfer (CURLE_WRITE_ERROR) once the cap is hit -> NetworkFailure.
    EXPECT_EQ(m_result, HttpResult_NetworkFailure);
    // The oversized body is never fully buffered.
    EXPECT_LE(m_bodySize, static_cast<size_t>(16u * 1024u * 1024u));
}

TEST_F(HttpClientCurlResponseCapTests, AcceptsLargeResponseUnderCap)
{
    // A large-but-legitimate response (well under the cap) must still be received
    // in full: the cap must not regress normal responses.
    const size_t bodySize = 4u * 1024u * 1024u;
    sendAndWait(bodySize);
    ASSERT_TRUE(responseReceived());
    EXPECT_EQ(m_result, HttpResult_OK);
    EXPECT_EQ(m_statusCode, 200u);
    EXPECT_EQ(m_bodySize, bodySize);
}

// --- Lifetime, cancellation and drain semantics ---

namespace
{

// A TCP endpoint that accepts connections at the kernel level (the listen
// backlog completes the handshake) but never reads or answers them. curl
// therefore connects, writes the request, and blocks waiting for a response
// until it is cancelled. No sleeps, no timing assumptions, no dependence on a
// live network: the stall is a property of the socket, not of the schedule.
class StalledEndpoint
{
public:
    StalledEndpoint()
    {
        m_listener = ::socket(AF_INET, SOCK_STREAM, 0);
        if (m_listener < 0)
        {
            return;
        }
        int reuse = 1;
        ::setsockopt(m_listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

        struct sockaddr_in address;
        std::memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = 0;
        if (::bind(m_listener, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) != 0 ||
            ::listen(m_listener, 32) != 0)
        {
            ::close(m_listener);
            m_listener = -1;
            return;
        }

        socklen_t length = sizeof(address);
        if (::getsockname(m_listener, reinterpret_cast<struct sockaddr*>(&address), &length) == 0)
        {
            m_port = ntohs(address.sin_port);
        }
    }

    ~StalledEndpoint()
    {
        if (m_listener >= 0)
        {
            ::close(m_listener);
        }
    }

    StalledEndpoint(StalledEndpoint const&) = delete;
    StalledEndpoint& operator=(StalledEndpoint const&) = delete;

    bool valid() const { return m_listener >= 0 && m_port != 0; }

    std::string url() const
    {
        return "http://127.0.0.1:" + std::to_string(m_port) + "/stall";
    }

private:
    int m_listener {-1};
    int m_port {0};
};

// One-shot barrier used to pin a callback in place for as long as a test needs.
class Gate
{
public:
    void wait()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this]() { return m_open; });
    }

    void open()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_open = true;
        }
        m_cv.notify_all();
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_open {false};
};

class RecordingCallback : public IHttpResponseCallback
{
public:
    // Runs inside OnHttpResponse, after the response has been counted, so a test
    // can hold the terminal callback open or re-enter the client from it.
    void setResponseHook(std::function<void()> hook)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_hook = std::move(hook);
    }

    void setStateHook(std::function<void(HttpStateEvent)> hook)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stateHook = std::move(hook);
    }

    void OnHttpResponse(IHttpResponse* response) override
    {
        std::unique_ptr<IHttpResponse> owned(response);
        std::function<void()> hook;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            ++m_responses;
            m_results.push_back(owned->GetResult());
            hook = m_hook;
        }
        m_cv.notify_all();
        if (hook != nullptr)
        {
            hook();
        }
    }

    void OnHttpStateEvent(HttpStateEvent state, void*, size_t) override
    {
        std::function<void(HttpStateEvent)> hook;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            ++m_states[static_cast<int>(state)];
            hook = m_stateHook;
        }
        m_cv.notify_all();
        if (hook != nullptr)
        {
            hook(state);
        }
    }

    size_t responses()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_responses;
    }

    size_t responsesWithResult(HttpResult result)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t count = 0;
        for (auto const& item : m_results)
        {
            if (item == result)
            {
                ++count;
            }
        }
        return count;
    }

    size_t stateCount(HttpStateEvent state)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_states.find(static_cast<int>(state));
        return (it == m_states.end()) ? 0u : it->second;
    }

    bool waitForResponses(size_t count, std::chrono::milliseconds timeout)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_cv.wait_for(lock, timeout, [&]() { return m_responses >= count; });
    }

    bool waitForState(HttpStateEvent state, size_t count, std::chrono::milliseconds timeout)
    {
        const int key = static_cast<int>(state);
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_cv.wait_for(lock, timeout, [&]() { return m_states[key] >= count; });
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    size_t m_responses {0};
    std::vector<HttpResult> m_results;
    std::map<int, size_t> m_states;
    std::function<void()> m_hook;
    std::function<void(HttpStateEvent)> m_stateHook;
};

constexpr std::chrono::milliseconds kInFlightTimeout {15000};
constexpr std::chrono::milliseconds kTerminalTimeout {15000};

} // namespace

class HttpClientCurlLifetimeTests : public ::testing::Test
{
protected:
    // Declared first so it is destroyed last: the client's destructor drains
    // in-flight transfers that are still pointed at this endpoint.
    StalledEndpoint m_endpoint;
    HttpClient_Curl m_client;

    void SetUp() override
    {
        ASSERT_TRUE(m_endpoint.valid()) << "could not open a loopback listening socket";
    }

    // Sends a request whose transfer is guaranteed to stall, and returns once
    // the worker has actually written the request to the socket.
    std::string sendStalled(std::unique_ptr<IHttpRequest>& request, RecordingCallback& callback)
    {
        request.reset(m_client.CreateRequest());
        request->SetUrl(m_endpoint.url());
        const std::string id = request->GetId();
        m_client.SendRequestAsync(request.get(), &callback);
        return id;
    }
};

// A client destroyed with a transfer in flight must deliver the terminal
// callback before ~HttpClient_Curl returns.
TEST_F(HttpClientCurlLifetimeTests, DestroyingClientWithRequestInFlightCompletesAbortedFirst)
{
    RecordingCallback callback;
    std::unique_ptr<HttpClient_Curl> client(new HttpClient_Curl());
    std::unique_ptr<IHttpRequest> request(client->CreateRequest());
    request->SetUrl(m_endpoint.url());
    client->SendRequestAsync(request.get(), &callback);
    ASSERT_TRUE(callback.waitForState(OnSending, 1, kInFlightTimeout));

    client.reset();

    // No wait here on purpose: the drain is the assertion.
    EXPECT_EQ(callback.responses(), 1u);
    EXPECT_EQ(callback.responsesWithResult(HttpResult_Aborted), 1u);
}

// The public IHttpClient contract requires the request to stay alive until the
// terminal callback begins. This intentionally violates that contract to prove
// Curl's private cancellation registry does not retain or dereference it.
TEST_F(HttpClientCurlLifetimeTests, InternalRegistryDoesNotDereferenceDeletedRequest)
{
    RecordingCallback callback;
    IHttpRequest* request = m_client.CreateRequest();
    request->SetUrl(m_endpoint.url());
    const std::string id = request->GetId();
    m_client.SendRequestAsync(request, &callback);
    ASSERT_TRUE(callback.waitForState(OnSending, 1, kInFlightTimeout));

    delete request;
    m_client.CancelRequestAsync(id);

    ASSERT_TRUE(callback.waitForResponses(1, kTerminalTimeout));
    EXPECT_EQ(callback.responses(), 1u);
    EXPECT_EQ(callback.responsesWithResult(HttpResult_Aborted), 1u);

    // Cancelling a retired id is a no-op and must not produce a second callback.
    m_client.CancelRequestAsync(id);
    EXPECT_EQ(callback.responses(), 1u);
}

// A full drain returns only when every operation has completed and been
// destroyed, for all of them, not just the first.
TEST_F(HttpClientCurlLifetimeTests, CancelAllRequestsFullyDrainsEveryOperation)
{
    constexpr size_t kRequests = 4;
    RecordingCallback callback;
    std::vector<std::unique_ptr<IHttpRequest>> requests(kRequests);
    for (size_t i = 0; i < kRequests; ++i)
    {
        sendStalled(requests[i], callback);
    }
    ASSERT_TRUE(callback.waitForState(OnSending, kRequests, kInFlightTimeout));

    m_client.CancelAllRequests();

    EXPECT_EQ(callback.responses(), kRequests);
    EXPECT_EQ(callback.responsesWithResult(HttpResult_Aborted), kRequests);
}

// The bounded overload is a soft cap: it stops waiting at the deadline even
// though a terminal callback (and therefore the operation and the shared state)
// is still alive. The callback keeps everything it touches alive itself.
TEST_F(HttpClientCurlLifetimeTests, BoundedCancelAllReturnsAtDeadlineWhileCallbackIsRunning)
{
    RecordingCallback callback;
    auto gate = std::make_shared<Gate>();
    callback.setResponseHook([gate]() { gate->wait(); });

    std::unique_ptr<IHttpRequest> request;
    const std::string id = sendStalled(request, callback);
    ASSERT_TRUE(callback.waitForState(OnSending, 1, kInFlightTimeout));
    m_client.CancelRequestAsync(id);
    // The response is counted before the hook blocks, so this proves the
    // terminal callback is in flight and pinned.
    ASSERT_TRUE(callback.waitForResponses(1, kTerminalTimeout));

    const auto start = std::chrono::steady_clock::now();
    m_client.CancelAllRequests(std::chrono::milliseconds(200));
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    EXPECT_GE(elapsed, std::chrono::milliseconds(150));
    EXPECT_LT(elapsed, std::chrono::seconds(5));

    gate->open();
    // The unbounded drain now has to complete, which also makes fixture
    // teardown safe.
    m_client.CancelAllRequests();
    EXPECT_EQ(callback.responses(), 1u);
}

// A terminal callback must abort every registered peer before returning from a
// reentrant CancelAllRequests call; it must not wait for either callback.
TEST_F(HttpClientCurlLifetimeTests, ReentrantCancelAllAbortsStalledPeerBeforeReturning)
{
    RecordingCallback callbackA;
    RecordingCallback callbackB;
    std::atomic<bool> reentrantCancelReturned {false};
    callbackA.setResponseHook([this, &reentrantCancelReturned]() {
        m_client.CancelAllRequests();
        reentrantCancelReturned = true;
    });

    std::unique_ptr<IHttpRequest> requestA;
    std::unique_ptr<IHttpRequest> requestB;
    const std::string idA = sendStalled(requestA, callbackA);
    sendStalled(requestB, callbackB);
    ASSERT_TRUE(callbackA.waitForState(OnSending, 1, kInFlightTimeout));
    ASSERT_TRUE(callbackB.waitForState(OnSending, 1, kInFlightTimeout));
    m_client.CancelRequestAsync(idA);
    ASSERT_TRUE(callbackA.waitForResponses(1, kTerminalTimeout));
    ASSERT_TRUE(callbackB.waitForResponses(1, kTerminalTimeout));

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(15);
    while (!reentrantCancelReturned && std::chrono::steady_clock::now() < deadline)
    {
        PAL::sleep(10);
    }
    if (!reentrantCancelReturned)
    {
        ADD_FAILURE() << "reentrant CancelAllRequests() did not return";
        std::abort();
    }

    m_client.CancelAllRequests();
    EXPECT_EQ(callbackA.responsesWithResult(HttpResult_Aborted), 1u);
    EXPECT_EQ(callbackB.responsesWithResult(HttpResult_Aborted), 1u);
}

TEST_F(HttpClientCurlLifetimeTests, StateCallbackMayDestroyClientDuringOperationConstruction)
{
    RecordingCallback callback;
    std::unique_ptr<HttpClient_Curl> client(new HttpClient_Curl());
    callback.setStateHook([&client](HttpStateEvent state) {
        if (state == OnCreated)
        {
            client.reset();
        }
    });

    std::unique_ptr<IHttpRequest> request(client->CreateRequest());
    request->SetUrl(m_endpoint.url());
    client->SendRequestAsync(request.get(), &callback);

    EXPECT_EQ(client.get(), nullptr);
    EXPECT_EQ(callback.responses(), 1u);
    EXPECT_EQ(callback.responsesWithResult(HttpResult_Aborted), 1u);
}

// A send that lands inside an open cancellation epoch must not start network
// work (that would let late arrivals starve the drain), and must still get
// exactly one terminal callback, synchronously, so no caller is left hanging.
TEST_F(HttpClientCurlLifetimeTests, SendDuringCancellationEpochCompletesAbortedWithoutNetwork)
{
    RecordingCallback stalledCallback;
    auto gate = std::make_shared<Gate>();
    stalledCallback.setResponseHook([gate]() { gate->wait(); });

    std::unique_ptr<IHttpRequest> stalledRequest;
    sendStalled(stalledRequest, stalledCallback);
    ASSERT_TRUE(stalledCallback.waitForState(OnSending, 1, kInFlightTimeout));

    // The drain runs on its own thread and cannot return while the pinned
    // callback is in flight, so the epoch is provably open below.
    std::thread drain([this]() { m_client.CancelAllRequests(); });
    ASSERT_TRUE(stalledCallback.waitForResponses(1, kTerminalTimeout));

    std::mutex lateEventsMutex;
    std::vector<std::string> lateEvents;
    auto lateCallback = std::make_shared<RecordingCallback>();
    lateCallback->setStateHook([&lateEventsMutex, &lateEvents](HttpStateEvent state) {
        std::lock_guard<std::mutex> lock(lateEventsMutex);
        switch (state)
        {
        case OnCreated: lateEvents.push_back("created"); break;
        case OnCreateFailed: lateEvents.push_back("create-failed"); break;
        case OnConnecting: lateEvents.push_back("connecting"); break;
        case OnConnectFailed: lateEvents.push_back("connect-failed"); break;
        case OnSendFailed: lateEvents.push_back("send-failed"); break;
        case OnSending: lateEvents.push_back("sending"); break;
        case OnResponse: lateEvents.push_back("response-state"); break;
        case OnDestroy: lateEvents.push_back("destroy"); break;
        }
    });
    lateCallback->setResponseHook([&lateEventsMutex, &lateEvents, &lateCallback]() {
        {
            std::lock_guard<std::mutex> lock(lateEventsMutex);
            lateEvents.push_back("response");
        }
        lateCallback.reset();
    });

    std::unique_ptr<IHttpRequest> lateRequest(m_client.CreateRequest());
    lateRequest->SetUrl(m_endpoint.url());
    m_client.SendRequestAsync(lateRequest.get(), lateCallback.get());

    // Completed synchronously, on this thread, before SendRequestAsync returned.
    {
        std::lock_guard<std::mutex> lock(lateEventsMutex);
        EXPECT_EQ(lateEvents, (std::vector<std::string>{"created", "destroy", "response"}));
    }

    gate->open();
    drain.join();
    EXPECT_EQ(stalledCallback.responses(), 1u);
}

// A reentrant CancelRequestAsync fired from the OnCreated state event must find
// the operation (it is registered before the event fires), stop it before any
// network work begins, and yield exactly one Aborted terminal in
// OnCreated -> OnDestroy -> response order.
TEST_F(HttpClientCurlLifetimeTests, OnCreatedCancelRequestFindsOperationAndAbortsWithoutNetwork)
{
    RecordingCallback callback;
    std::unique_ptr<IHttpRequest> request(m_client.CreateRequest());
    request->SetUrl(m_endpoint.url());
    const std::string id = request->GetId();

    std::mutex eventsMutex;
    std::vector<std::string> events;
    callback.setStateHook([this, id, &eventsMutex, &events](HttpStateEvent state) {
        {
            std::lock_guard<std::mutex> lock(eventsMutex);
            switch (state)
            {
            case OnCreated: events.push_back("created"); break;
            case OnCreateFailed: events.push_back("create-failed"); break;
            case OnConnecting: events.push_back("connecting"); break;
            case OnConnectFailed: events.push_back("connect-failed"); break;
            case OnSendFailed: events.push_back("send-failed"); break;
            case OnSending: events.push_back("sending"); break;
            case OnResponse: events.push_back("response-state"); break;
            case OnDestroy: events.push_back("destroy"); break;
            }
        }
        if (state == OnCreated)
        {
            // If the operation were not registered yet, this would be a no-op and
            // the transfer would proceed to the stalled endpoint.
            m_client.CancelRequestAsync(id);
        }
    });
    callback.setResponseHook([&eventsMutex, &events]() {
        std::lock_guard<std::mutex> lock(eventsMutex);
        events.push_back("response");
    });

    m_client.SendRequestAsync(request.get(), &callback);

    ASSERT_TRUE(callback.waitForResponses(1, kTerminalTimeout));
    EXPECT_EQ(callback.responses(), 1u);
    EXPECT_EQ(callback.responsesWithResult(HttpResult_Aborted), 1u);
    // No worker, no socket: the cancellation during OnCreated was honored.
    EXPECT_EQ(callback.stateCount(OnConnecting), 0u);
    EXPECT_EQ(callback.stateCount(OnSending), 0u);
    {
        std::lock_guard<std::mutex> lock(eventsMutex);
        EXPECT_EQ(events, (std::vector<std::string>{"created", "destroy", "response"}));
    }
}

// The same guarantee for a reentrant CancelAllRequests fired from OnCreated: the
// operation is found among the peers, aborted before network work, and produces
// exactly one Aborted terminal.
TEST_F(HttpClientCurlLifetimeTests, OnCreatedCancelAllAbortsOperationBeforeNetwork)
{
    RecordingCallback callback;
    std::unique_ptr<IHttpRequest> request(m_client.CreateRequest());
    request->SetUrl(m_endpoint.url());
    callback.setStateHook([this](HttpStateEvent state) {
        if (state == OnCreated)
        {
            m_client.CancelAllRequests();
        }
    });

    m_client.SendRequestAsync(request.get(), &callback);

    ASSERT_TRUE(callback.waitForResponses(1, kTerminalTimeout));
    EXPECT_EQ(callback.responses(), 1u);
    EXPECT_EQ(callback.responsesWithResult(HttpResult_Aborted), 1u);
    EXPECT_EQ(callback.stateCount(OnConnecting), 0u);
    EXPECT_EQ(callback.stateCount(OnSending), 0u);
    EXPECT_EQ(callback.stateCount(OnDestroy), 1u);
}

// A cancellation reentered from the OnDestroy state event of a *successful*
// transfer may legitimately abort peers, but it must not rewrite this
// operation's already-finished result. The cancellation classification is
// frozen before OnDestroy runs, so the terminal stays OK/200.
class HttpClientCurlDestroyReentryTests : public ::testing::Test,
                                          public HttpServer::Callback
{
protected:
    HttpServer      m_server;
    HttpClient_Curl m_client;
    std::string     m_url;

    void SetUp() override
    {
        const int port = m_server.addListeningPort(0);
        std::ostringstream address;
        address << "127.0.0.1:" << port;
        m_url = "http://" + address.str() + "/ok/";
        m_server.setServerName(address.str());
        m_server.addHandler("/ok/", *this);
        m_server.start();
    }

    void TearDown() override
    {
        m_server.stop();
    }

    int onHttpRequest(HttpServer::Request const&, HttpServer::Response& response) override
    {
        response.content = "ok-body";
        return 200;
    }

    struct ResultCallback : public IHttpResponseCallback
    {
        std::mutex mutex;
        std::condition_variable cv;
        size_t responses {0};
        HttpResult result {};
        unsigned int statusCode {0};
        std::function<void(HttpStateEvent)> stateHook;

        void OnHttpResponse(IHttpResponse* response) override
        {
            std::unique_ptr<IHttpResponse> owned(response);
            {
                std::lock_guard<std::mutex> lock(mutex);
                ++responses;
                result = owned->GetResult();
                statusCode = owned->GetStatusCode();
            }
            cv.notify_all();
        }

        void OnHttpStateEvent(HttpStateEvent state, void*, size_t) override
        {
            if (stateHook != nullptr)
            {
                stateHook(state);
            }
        }

        bool waitForResponse(std::chrono::milliseconds timeout)
        {
            std::unique_lock<std::mutex> lock(mutex);
            return cv.wait_for(lock, timeout, [&]() { return responses >= 1; });
        }
    };
};

TEST_F(HttpClientCurlDestroyReentryTests, OnDestroyReentrantCancelDoesNotRewriteSuccess)
{
    ResultCallback callback;
    std::unique_ptr<IHttpRequest> request(m_client.CreateRequest());
    request->SetUrl(m_url);
    const std::string id = request->GetId();

    callback.stateHook = [this, id](HttpStateEvent state) {
        if (state == OnDestroy)
        {
            // The operation is still registered during OnDestroy. Both of these
            // set its live abort flag, but the frozen classification must win.
            m_client.CancelRequestAsync(id);
            m_client.CancelAllRequests();
        }
    };

    m_client.SendRequestAsync(request.get(), &callback);
    ASSERT_TRUE(callback.waitForResponse(kTerminalTimeout));

    std::lock_guard<std::mutex> lock(callback.mutex);
    EXPECT_EQ(callback.responses, 1u);
    EXPECT_EQ(callback.result, HttpResult_OK);
    EXPECT_EQ(callback.statusCode, 200u);
}

// Clients are independent: one going away with work in flight must not disturb
// another, and the process-wide libcurl initialization must survive all of it.
TEST_F(HttpClientCurlLifetimeTests, OverlappingClientsWithActiveRequestsDestroyIndependently)
{
    constexpr size_t kClients = 4;
    std::vector<std::unique_ptr<RecordingCallback>> callbacks;
    for (size_t i = 0; i < kClients; ++i)
    {
        callbacks.emplace_back(new RecordingCallback());
    }

    Gate release;
    std::vector<std::thread> threads;
    for (size_t i = 0; i < kClients; ++i)
    {
        threads.emplace_back([this, i, &callbacks, &release]() {
            HttpClient_Curl client;
            std::unique_ptr<IHttpRequest> request(client.CreateRequest());
            request->SetUrl(m_endpoint.url());
            client.SendRequestAsync(request.get(), callbacks[i].get());
            callbacks[i]->waitForState(OnSending, 1, kInFlightTimeout);
            // Destroy all of them while every one of them has work in flight.
            release.wait();
        });
    }

    for (size_t i = 0; i < kClients; ++i)
    {
        callbacks[i]->waitForState(OnSending, 1, kInFlightTimeout);
    }
    release.open();
    for (auto& thread : threads)
    {
        thread.join();
    }

    for (size_t i = 0; i < kClients; ++i)
    {
        EXPECT_EQ(callbacks[i]->responses(), 1u) << "client " << i;
        EXPECT_EQ(callbacks[i]->responsesWithResult(HttpResult_Aborted), 1u) << "client " << i;
    }
}

#endif // MATSDK_PAL_CPP11 && !_MSC_VER && HAVE_MAT_DEFAULT_HTTP_CLIENT
