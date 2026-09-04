//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif
// Must precede the guard below: HAVE_MAT_DEFAULT_HTTP_CLIENT comes from the SDK
// configuration header, so testing it before including this silently compiles
// the whole suite away (same ordering as HttpClientCurlTests.cpp).
#include "mat/config.h"

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
#include "common/Common.hpp"
#include "common/HttpServer.hpp"
#include "http/HttpClientFactory.hpp"

// Mirror HttpClientFactory's selection of HttpClient_Apple so the Apple-specific
// tests below only compile when the factory actually hands back that transport.
// On macOS desktop without APPLE_HTTP the factory builds HttpClient_Curl instead,
// and gating merely on __APPLE__ would run these expectations against the wrong
// client.
#if defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE || defined(APPLE_HTTP)
#define MAT_TEST_APPLE_TRANSPORT 1
#endif
#endif

#include <atomic>
#include <condition_variable>
#include <future>
#include <thread>

using namespace testing;
using namespace MAT;

#define getMATSDKLogComponent  ::testing::getMATSDKLogComponent

class HttpClientTests : public ::testing::Test,
                        public HttpServer::Callback,
                        public IHttpResponseCallback
{
  protected:
    HttpServer                           _server;
    int                                  _port;
    std::string                          _hostname;
    std::shared_ptr<IHttpClient>         _client;
    std::vector<IHttpResponse*>          _responses;

    enum RequestState { Planned, Sent, Processed, Done };
    std::vector<RequestState>            _countedRequests;
    std::mutex                           _lock;
    std::condition_variable              _responseCv;
    std::condition_variable              _blockedRequestCv;
    std::mutex                           _blockedRequestLock;
    bool                                  _blockedRequestReceived {false};
    bool                                  _releaseBlockedRequest {false};
    bool                                  _cancelOnConnecting {false};
    bool                                  _blockStateEvent {false};
    HttpStateEvent                        _stateEventToBlock {OnConnecting};
    bool                                  _stateEventEntered {false};
    bool                                  _releaseConnecting {false};
    bool                                  _blockResponseCallback {false};
    bool                                  _responseCallbackEntered {false};
    bool                                  _releaseResponseCallback {false};
    std::atomic<size_t>                   _cancelAllOnResponse {0};
    std::atomic<bool>                     _synchronizeCancelAllResponses {false};
    size_t                                _cancelAllResponsesEntered {0};
    std::atomic<bool>                     _sendRequestOnResponse {false};
    bool                                  _destroyClientOnConnecting {false};
    std::string                           _lateRequestId;

  public:
    HttpClientTests()
    {
        _client = HttpClientFactory::Create();
    }

    void Clear()
    {
        for (auto &v : _responses)
            delete v;
        _responses.clear();
        _countedRequests.clear();
    }

    bool responseReceived()
    {
        return (_responses.size() > 0);
    }

    virtual void SetUp() override
    {
        _port = _server.addListeningPort(0);
        std::ostringstream os;
        os << "127.0.0.1:" << _port;
        _hostname = os.str();
        _server.setServerName(_hostname);
        _server.addHandler("/simple/", *this);
        _server.addHandler("/echo/",   *this);
        _server.addHandler("/count/",  *this);
        _server.addHandler("/block/",  *this);
        _server.addHandler("/large/",  *this);
        _server.addHandler("/redirect/", *this);
        _server.addHandler("/query", *this);
        _server.start();

        Clear();
    }

    virtual void TearDown() override
    {
        {
            std::lock_guard<std::mutex> lock(_blockedRequestLock);
            _releaseBlockedRequest = true;
            _releaseConnecting = true;
            _releaseResponseCallback = true;
        }
        _blockedRequestCv.notify_all();
        _server.stop();
        _client.reset();
        Clear();
    }

  protected:
    // Deterministic filler whose every byte depends on its offset, so a
    // truncated, duplicated or misordered chunk cannot pass unnoticed.
    static std::string LargePayload(size_t size)
    {
        std::string payload(size, '\0');
        for (size_t i = 0; i < size; ++i) {
            payload[i] = static_cast<char>('a' + (i % 26));
        }
        return payload;
    }

    virtual int onHttpRequest(HttpServer::Request const& request, HttpServer::Response& inResponse) override
    {
        if (request.uri.substr(0, 8) == "/simple/") {
            inResponse.headers["Content-Type"] = "text/plain";
            inResponse.content = "It works!";
            return atoi(request.uri.substr(8).c_str());
        }

        if (request.uri == "/echo/") {
            auto it = request.headers.find("Content-Type");
            inResponse.headers["Content-Type"] = (it != request.headers.end()) ? it->second : "application/octet-stream";
            inResponse.content = request.content;
            return 200;
        }

        if (request.uri == "/query?key=value") {
            return 200;
        }

        if (request.uri == "/block/") {
            {
                std::lock_guard<std::mutex> lock(_blockedRequestLock);
                _blockedRequestReceived = true;
            }
            _blockedRequestCv.notify_all();
            std::unique_lock<std::mutex> lock(_blockedRequestLock);
            _blockedRequestCv.wait(lock, [this]() { return _releaseBlockedRequest; });
            return 200;
        }

        if (request.uri == "/redirect/") {
            inResponse.headers["Location"] = "http://" + _hostname + "/simple/200";
            return 302;
        }

        if (request.uri.substr(0, 7) == "/large/") {
            size_t size = static_cast<size_t>(atoi(request.uri.substr(7).c_str()));
            inResponse.headers["Content-Type"] = "application/octet-stream";
            inResponse.content = LargePayload(size);
            return 200;
        }

        if (request.uri.substr(0, 7) == "/count/") {
            int id = atoi(request.uri.substr(7).c_str());
            if (id >= 0 && static_cast<size_t>(id) < _countedRequests.size()) {
                _countedRequests[id] = Processed;
            }
            inResponse.headers["Content-Type"] = "text/plain";
            inResponse.content = request.uri.substr(7);
            return 200;
        }

        return 0;
    }

    /**
     * This method temporarily copies SimpleHttpResponse to a responses buffer.
     */
    virtual SimpleHttpResponse* clone(IHttpResponse* inResponse)
    {
        SimpleHttpResponse *src = static_cast<SimpleHttpResponse*>(inResponse);
        SimpleHttpResponse *dst = new SimpleHttpResponse("");
        dst->m_id = src->m_id;
        dst->m_result = src->m_result;
        dst->m_statusCode = src->m_statusCode;
        dst->m_headers = src->m_headers;
        dst->m_body = src->m_body;
        return dst;
    }

    virtual void OnHttpResponse(IHttpResponse* inResponse) override
    {
        if (_sendRequestOnResponse.exchange(false))
        {
            std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
            request->SetUrl("http://" + _hostname + "/echo/");
            {
                std::lock_guard<std::mutex> lock(_blockedRequestLock);
                _lateRequestId = request->GetId();
            }
            _client->SendRequestAsync(request.release(), this);
        }
        bool cancelAll = false;
        size_t remaining = _cancelAllOnResponse.load();
        while (remaining != 0)
        {
            if (_cancelAllOnResponse.compare_exchange_weak(
                    remaining, remaining - 1))
            {
                cancelAll = true;
                break;
            }
        }
        if (cancelAll && _synchronizeCancelAllResponses.load())
        {
            std::unique_lock<std::mutex> lock(_blockedRequestLock);
            ++_cancelAllResponsesEntered;
            _blockedRequestCv.notify_all();
            _blockedRequestCv.wait_for(lock, std::chrono::seconds(5), [this]() {
                return _cancelAllResponsesEntered == 2;
            });
        }
        if (cancelAll)
        {
            _client->CancelAllRequests();
        }
        {
            std::unique_lock<std::mutex> lock(_blockedRequestLock);
            if (_blockResponseCallback)
            {
                _responseCallbackEntered = true;
                _blockedRequestCv.notify_all();
                _blockedRequestCv.wait(lock, [this]() {
                    return _releaseResponseCallback;
                });
            }
        }
        std::lock_guard<std::mutex> lock(_lock);
        _responses.push_back(clone(inResponse));
        _responseCv.notify_all();
    }

    virtual void OnHttpStateEvent(HttpStateEvent state, void*, size_t) override
    {
        if (_destroyClientOnConnecting && state == OnConnecting)
        {
            _destroyClientOnConnecting = false;
            _client.reset();
        }
        if (_cancelOnConnecting && state == OnConnecting)
        {
            _cancelOnConnecting = false;
            _client->CancelAllRequests();
        }
        if (_blockStateEvent && state == _stateEventToBlock)
        {
            std::unique_lock<std::mutex> lock(_blockedRequestLock);
            _stateEventEntered = true;
            _blockedRequestCv.notify_all();
            _blockedRequestCv.wait(lock, [this]() { return _releaseConnecting; });
            _blockStateEvent = false;
        }
    }
};

std::vector<uint8_t> Binary(std::string const& str)
{
    return std::vector<uint8_t>(str.data(), str.data() + str.size());
}

TEST_F(HttpClientTests, HandlesCancellationWhileResponseIsInFlight)
{
    Clear();
    {
        std::lock_guard<std::mutex> lock(_blockedRequestLock);
        _blockedRequestReceived = false;
        _releaseBlockedRequest = false;
    }

    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/block/");
    _client->SendRequestAsync(request.get(), this);

    {
        std::unique_lock<std::mutex> lock(_blockedRequestLock);
        ASSERT_TRUE(_blockedRequestCv.wait_for(lock, std::chrono::seconds(10),
            [this]() { return _blockedRequestReceived; }));
    }

    _client->CancelRequestAsync(requestId);
    {
        std::lock_guard<std::mutex> lock(_blockedRequestLock);
        _releaseBlockedRequest = true;
    }
    _blockedRequestCv.notify_all();

    std::unique_ptr<IHttpResponse> response;
    {
        std::unique_lock<std::mutex> lock(_lock);
        ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(2),
            [this]() { return !_responses.empty(); }));
        ASSERT_EQ(_responses.size(), 1u);
        response.reset(_responses[0]);
        _responses.clear();
    }

    EXPECT_THAT(response->GetId(), requestId);
    EXPECT_THAT(response->GetResult(), HttpResult_Aborted);
#if defined(MAT_TEST_APPLE_TRANSPORT)
    {
        std::unique_lock<std::mutex> lock(_lock);
        EXPECT_FALSE(_responseCv.wait_for(lock, std::chrono::milliseconds(250),
            [this]() { return !_responses.empty(); }));
    }
#endif
}

//---

#ifdef MATSDK_PAL_WIN32
TEST_F(HttpClientTests, UsesConfiguredWindowsTransport)
{
#if defined(HAVE_MAT_WININET_HTTP_CLIENT)
    EXPECT_THAT(dynamic_cast<HttpClient_WinInet*>(_client.get()), NotNull());
#elif defined(HAVE_MAT_WINHTTP_HTTP_CLIENT)
    EXPECT_THAT(dynamic_cast<HttpClient_WinHttp*>(_client.get()), NotNull());
#else
#error A Windows HTTP transport must be selected.
#endif
}

TEST_F(HttpClientTests, DisablesRedirectsWhenMicrosoftRootCheckIsEnabled)
{
#if defined(HAVE_MAT_WININET_HTTP_CLIENT)
    auto windowsClient = dynamic_cast<HttpClient_WinInet*>(_client.get());
#elif defined(HAVE_MAT_WINHTTP_HTTP_CLIENT)
    auto windowsClient = dynamic_cast<HttpClient_WinHttp*>(_client.get());
#else
#error A Windows HTTP transport must be selected.
#endif
    ASSERT_THAT(windowsClient, NotNull());
    windowsClient->SetMsRootCheck(true);

    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    request->SetUrl("http://" + _hostname + "/redirect/");
    _client->SendRequestAsync(request.release(), this);

    std::unique_lock<std::mutex> lock(_lock);
    ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(5),
        [this]() { return !_responses.empty(); }));
    ASSERT_EQ(_responses.size(), 1u);
    EXPECT_THAT(_responses[0]->GetResult(), HttpResult_OK);
    EXPECT_THAT(_responses[0]->GetStatusCode(), 302u);
}
#endif

TEST_F(HttpClientTests, HandlesSimpleRequest)
{
    Clear();
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/simple/200");
    _client->SendRequestAsync(request.release(), this);

    while (!responseReceived())
        PAL::sleep(100);

    std::unique_ptr<IHttpResponse> _response(_responses[0]);
    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_OK);
    EXPECT_THAT(_response->GetStatusCode(), 200u);
    EXPECT_THAT(_response->GetHeaders().get("Host"), _hostname);
    EXPECT_THAT(_response->GetBody(), Eq(Binary("It works!")));
    _response.release();
}

TEST_F(HttpClientTests, HandlesErrorRequest)
{
    Clear();
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/simple/404");
    _client->SendRequestAsync(request.release(), this);

    while (!responseReceived())
        PAL::sleep(100);

    std::unique_ptr<IHttpResponse> _response(_responses[0]);
    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_OK);
    EXPECT_THAT(_response->GetStatusCode(), 404u);
    EXPECT_THAT(_response->GetHeaders().get("Host"), _hostname);
    EXPECT_THAT(_response->GetBody(), Eq(Binary("It works!")));
    _response.release();

}

TEST_F(HttpClientTests, HandlesPostRequest)
{
    Clear();
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetMethod("POST");
    request->GetHeaders().set("Content-Type", "application/octet-stream");
    request->SetUrl("http://" + _hostname + "/echo/");
    auto body = Binary("Some\xBB\x11naryContent");
    request->SetBody(body);
    _client->SendRequestAsync(request.release(), this);

    while (!responseReceived())
        PAL::sleep(100);

    std::unique_ptr<IHttpResponse> _response(_responses[0]);
    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_OK);
    EXPECT_THAT(_response->GetStatusCode(), 200u);
    EXPECT_THAT(_response->GetHeaders().get("Host"), _hostname);
    EXPECT_THAT(_response->GetHeaders().get("Content-Type"), Eq("application/octet-stream"));
    EXPECT_THAT(_response->GetBody(), Eq(Binary("Some\xBB\x11naryContent")));
    _response.release();
}

TEST_F(HttpClientTests, HandlesLocalErrors)
{
    Clear();
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("://trololo!");
    _client->SendRequestAsync(request.release(), this);

    for (int i = 0; i < 200 && !responseReceived(); i++) {
        PAL::sleep(100);
    }

    std::unique_ptr<IHttpResponse> _response(_responses[0]);
    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_LocalFailure);
    _response.release();
}

#if defined(MAT_TEST_APPLE_TRANSPORT)
TEST_F(HttpClientTests, InvalidUtf8UrlCompletesExactlyOnce)
{
    // The request must outlive the whole exchange: keep ownership here (the Apple
    // transport never deletes it) and hand only a borrowed pointer to the client.
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    std::string invalidUrl("http://invalid-url/");
    invalidUrl.push_back(static_cast<char>(0xff));
    request->SetUrl(invalidUrl);
    _client->SendRequestAsync(request.get(), this);

    std::unique_lock<std::mutex> lock(_lock);
    ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(5),
        [this]() { return !_responses.empty(); }));
    ASSERT_EQ(_responses.size(), 1u);
    EXPECT_THAT(_responses[0]->GetId(), requestId);
    EXPECT_THAT(_responses[0]->GetResult(), HttpResult_LocalFailure);
    EXPECT_FALSE(_responseCv.wait_for(lock, std::chrono::milliseconds(250),
        [this]() { return _responses.size() > 1; }));
}

TEST_F(HttpClientTests, CancelBeforeSendCompletesExactlyOneAborted)
{
    // A cancel issued before SendRequestAsync must only arm the cancel flag; the
    // single Aborted has to be delivered by Send once the callback is known, and
    // never twice. The request is kept alive by this fixture for the duration.
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/simple/200");

    _client->CancelRequestAsync(requestId);
    _client->SendRequestAsync(request.get(), this);

    std::unique_lock<std::mutex> lock(_lock);
    ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(5),
        [this]() { return !_responses.empty(); }));
    ASSERT_EQ(_responses.size(), 1u);
    EXPECT_THAT(_responses[0]->GetId(), requestId);
    EXPECT_THAT(_responses[0]->GetResult(), HttpResult_Aborted);
    EXPECT_FALSE(_responseCv.wait_for(lock, std::chrono::milliseconds(250),
        [this]() { return _responses.size() > 1; }));
}

TEST_F(HttpClientTests, CancelAllReturnsWithUnsentRequest)
{
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/simple/200");

    auto cancel = std::async(std::launch::async, [this]()
                             { _client->CancelAllRequests(); });
    ASSERT_EQ(cancel.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    cancel.get();

    _client->SendRequestAsync(request.get(), this);

    std::unique_lock<std::mutex> lock(_lock);
    ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(5),
                                     [this]()
                                     { return !_responses.empty(); }));
    ASSERT_EQ(_responses.size(), 1u);
    EXPECT_THAT(_responses[0]->GetId(), requestId);
    EXPECT_THAT(_responses[0]->GetResult(), HttpResult_Aborted);
    EXPECT_FALSE(_responseCv.wait_for(lock, std::chrono::milliseconds(250),
                                      [this]()
                                      { return _responses.size() > 1; }));
}

#endif

TEST_F(HttpClientTests, HandlesDnsError)
{
    Clear();
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://domain.name.doesnt.exist");
    _client->SendRequestAsync(request.release(), this);

    for (int i = 0; i < 200 && !responseReceived(); i++) {
        PAL::sleep(100);
    }

    std::unique_ptr<IHttpResponse> _response(_responses[0]);
    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_NetworkFailure);
    _response.release();
}

TEST_F(HttpClientTests, HandlesConnectionError)
{
    Clear();
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://localhost:4");
    _client->SendRequestAsync(request.release(), this);

    for (int i = 0; i < 200 && !responseReceived(); i++) {
        PAL::sleep(100);
    }

    std::unique_ptr<IHttpResponse> _response(_responses[0]);
    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_NetworkFailure);
    _response.release();
}

TEST_F(HttpClientTests, HandlesCancellation)
{
    Clear();
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/echo/");
    _client->SendRequestAsync(request.release(), this);
    _client->CancelRequestAsync(requestId);

    for (int i = 0; i < 20 && !responseReceived(); i++) {
        PAL::sleep(100);
    }

    std::unique_ptr<IHttpResponse> _response(_responses[0]);
    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_Aborted);
    _response.release();
}

#if defined(HAVE_MAT_WINHTTP_HTTP_CLIENT) || defined(HAVE_MAT_WININET_HTTP_CLIENT)
TEST_F(HttpClientTests, HandlesCancellationFromStateEvent)
{
    Clear();
    _cancelOnConnecting = true;

    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/echo/");
    _client->SendRequestAsync(request.release(), this);

    std::unique_ptr<IHttpResponse> response;
    {
        std::unique_lock<std::mutex> lock(_lock);
        ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(2),
            [this]() { return !_responses.empty(); }));
        ASSERT_EQ(_responses.size(), 1u);
        response.reset(_responses[0]);
        _responses.clear();
    }

    EXPECT_THAT(response->GetId(), requestId);
    EXPECT_THAT(response->GetResult(), HttpResult_Aborted);
}

TEST_F(HttpClientTests, HandlesConcurrentCancellationDuringStateEvent)
{
    Clear();
    {
        std::lock_guard<std::mutex> lock(_blockedRequestLock);
        _blockStateEvent = true;
        _stateEventToBlock = OnSending;
        _stateEventEntered = false;
        _releaseConnecting = false;
    }

    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/echo/");
    IHttpRequest* requestPtr = request.release();
    std::thread sender([this, requestPtr]() {
        _client->SendRequestAsync(requestPtr, this);
    });

    {
        std::unique_lock<std::mutex> lock(_blockedRequestLock);
        ASSERT_TRUE(_blockedRequestCv.wait_for(lock, std::chrono::seconds(2),
            [this]() { return _stateEventEntered; }));
    }
    _client->CancelRequestAsync(requestId);
    {
        std::lock_guard<std::mutex> lock(_lock);
        EXPECT_TRUE(_responses.empty())
            << "Terminal response overlapped the active state callback";
    }
    {
        std::lock_guard<std::mutex> lock(_blockedRequestLock);
        _releaseConnecting = true;
    }
    _blockedRequestCv.notify_all();
    sender.join();

    std::unique_ptr<IHttpResponse> response;
    {
        std::unique_lock<std::mutex> lock(_lock);
        ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(2),
            [this]() { return !_responses.empty(); }));
        ASSERT_EQ(_responses.size(), 1u);
        response.reset(_responses[0]);
        _responses.clear();
    }

    EXPECT_THAT(response->GetId(), requestId);
    EXPECT_THAT(response->GetResult(), HttpResult_Aborted);
}
#endif

#if defined(HAVE_MAT_WINHTTP_HTTP_CLIENT) || defined(HAVE_MAT_WININET_HTTP_CLIENT)
TEST_F(HttpClientTests, CancelAllWaitsForActiveStateCallback)
{
    {
        std::lock_guard<std::mutex> lock(_blockedRequestLock);
        _blockStateEvent = true;
        _stateEventToBlock = OnSending;
    }

    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    request->SetUrl("http://" + _hostname + "/echo/");
    IHttpRequest* requestPtr = request.release();
    std::thread sender([this, requestPtr]() {
        _client->SendRequestAsync(requestPtr, this);
    });

    {
        std::unique_lock<std::mutex> lock(_blockedRequestLock);
        ASSERT_TRUE(_blockedRequestCv.wait_for(lock, std::chrono::seconds(5),
            [this]() { return _stateEventEntered; }));
    }

    std::atomic<bool> cancelReturned {false};
    std::thread canceller([this, &cancelReturned]() {
        _client->CancelAllRequests();
        cancelReturned.store(true);
    });

    PAL::sleep(100);
    EXPECT_FALSE(cancelReturned.load());
    {
        std::lock_guard<std::mutex> lock(_blockedRequestLock);
        _releaseConnecting = true;
    }
    _blockedRequestCv.notify_all();
    sender.join();
    canceller.join();
    EXPECT_TRUE(cancelReturned.load());
}

TEST_F(HttpClientTests, CancelAllWaitsForTerminalCallback)
{
    {
        std::lock_guard<std::mutex> lock(_blockedRequestLock);
        _blockResponseCallback = true;
    }

    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    request->SetUrl("http://" + _hostname + "/simple/200");
    _client->SendRequestAsync(request.release(), this);

    {
        std::unique_lock<std::mutex> lock(_blockedRequestLock);
        ASSERT_TRUE(_blockedRequestCv.wait_for(lock, std::chrono::seconds(5),
            [this]() { return _responseCallbackEntered; }));
    }

    std::atomic<bool> cancelReturned {false};
    std::thread canceller([this, &cancelReturned]() {
        _client->CancelAllRequests();
        cancelReturned.store(true);
    });

    PAL::sleep(100);
    EXPECT_FALSE(cancelReturned.load());
    {
        std::lock_guard<std::mutex> lock(_blockedRequestLock);
        _releaseResponseCallback = true;
    }
    _blockedRequestCv.notify_all();
    canceller.join();
    EXPECT_TRUE(cancelReturned.load());
}

TEST_F(HttpClientTests, TerminalCallbackCanCancelAllRequests)
{
    _cancelAllOnResponse.store(1);

    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    request->SetUrl("http://" + _hostname + "/simple/200");
    _client->SendRequestAsync(request.release(), this);

    std::unique_lock<std::mutex> lock(_lock);
    ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(5),
        [this]() { return !_responses.empty(); }));
    EXPECT_THAT(_responses[0]->GetResult(), HttpResult_OK);
}

#if defined(HAVE_MAT_WINHTTP_HTTP_CLIENT)
TEST_F(HttpClientTests, QueryStringIsPreservedWithoutFragment)
{
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    request->SetUrl("http://" + _hostname + "/query?key=value#client-only");
    _client->SendRequestAsync(request.release(), this);

    std::unique_lock<std::mutex> lock(_lock);
    ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(5),
        [this]() { return !_responses.empty(); }));
    EXPECT_THAT(_responses[0]->GetStatusCode(), 200u);
}

TEST_F(HttpClientTests, SynchronousFailureCallbackCanCancelAllRequests)
{
    _cancelAllOnResponse.store(1);

    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    request->SetUrl("://invalid-url");
    _client->SendRequestAsync(request.release(), this);

    std::unique_lock<std::mutex> lock(_lock);
    ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(5),
        [this]() { return !_responses.empty(); }));
    EXPECT_THAT(_responses[0]->GetResult(), HttpResult_LocalFailure);
}
#endif

TEST_F(HttpClientTests, ConcurrentTerminalCallbacksCanCancelAllRequests)
{
    _synchronizeCancelAllResponses.store(true);
    _cancelAllOnResponse.store(2);

    for (size_t i = 0; i < 2; ++i)
    {
        std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
        request->SetUrl("http://" + _hostname + "/simple/200");
        _client->SendRequestAsync(request.release(), this);
    }

    std::unique_lock<std::mutex> lock(_lock);
    ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(10),
        [this]() { return _responses.size() == 2; }));
    EXPECT_THAT(_cancelAllResponsesEntered, 2u);
}

TEST_F(HttpClientTests, StateCallbackCanDestroyClient)
{
    _destroyClientOnConnecting = true;

    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/simple/200");
    _client->SendRequestAsync(request.release(), this);

    EXPECT_THAT(_client, IsNull());
    std::unique_lock<std::mutex> lock(_lock);
    ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(5),
        [this]() { return !_responses.empty(); }));
    EXPECT_THAT(_responses[0]->GetId(), requestId);
    EXPECT_THAT(_responses[0]->GetResult(), HttpResult_Aborted);
}

TEST_F(HttpClientTests, CancelAllIncludesRequestRegisteredDuringDrain)
{
    {
        std::lock_guard<std::mutex> lock(_blockedRequestLock);
        _blockStateEvent = true;
        _stateEventToBlock = OnSending;
        _stateEventEntered = false;
        _releaseConnecting = false;
    }
    _sendRequestOnResponse.store(true);

    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    request->SetUrl("http://" + _hostname + "/echo/");
    IHttpRequest* requestPtr = request.release();
    std::thread sender([this, requestPtr]() {
        _client->SendRequestAsync(requestPtr, this);
    });

    {
        std::unique_lock<std::mutex> lock(_blockedRequestLock);
        ASSERT_TRUE(_blockedRequestCv.wait_for(lock, std::chrono::seconds(5),
            [this]() { return _stateEventEntered; }));
    }

    std::atomic<bool> cancelStarted {false};
    std::thread canceller([this, &cancelStarted]() {
        cancelStarted.store(true);
        _client->CancelAllRequests();
    });
    while (!cancelStarted.load())
    {
        std::this_thread::yield();
    }
    PAL::sleep(100);

    {
        std::lock_guard<std::mutex> lock(_blockedRequestLock);
        _stateEventEntered = false;
        _releaseConnecting = true;
    }
    _blockedRequestCv.notify_all();

    sender.join();
    canceller.join();

    std::unique_lock<std::mutex> lock(_lock);
    ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(5),
        [this]() { return _responses.size() == 2; }));
    auto lateResponse = std::find_if(
        _responses.begin(), _responses.end(), [this](IHttpResponse* response) {
            return response->GetId() == _lateRequestId;
        });
    ASSERT_THAT(lateResponse, Ne(_responses.end()));
    EXPECT_THAT((*lateResponse)->GetResult(), HttpResult_Aborted);
}

TEST_F(HttpClientTests, ClientRemainsReusableAfterCancelAll)
{
    _client->CancelAllRequests();

    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    request->SetUrl("http://" + _hostname + "/simple/200");
    _client->SendRequestAsync(request.release(), this);

    std::unique_lock<std::mutex> lock(_lock);
    ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(5),
        [this]() { return !_responses.empty(); }));
    EXPECT_THAT(_responses[0]->GetResult(), HttpResult_OK);
}
#endif

TEST_F(HttpClientTests, Handles100Continue)
{
    Clear();
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetMethod("POST");
    request->GetHeaders().set("Expect", "100-continue");
    request->GetHeaders().set("Content-Type", "application/octet-stream");
    request->SetUrl("http://" + _hostname + "/echo/");
    auto body = Binary("Some\xBB\x11naryContent");
    request->SetBody(body);
    _client->SendRequestAsync(request.release(), this);

    for (int i = 0; i < 20 && !responseReceived(); i++) {
        PAL::sleep(100);
    }

    std::unique_ptr<IHttpResponse> _response(_responses[0]);
    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_OK);
    EXPECT_THAT(_response->GetStatusCode(), 200u);
    EXPECT_THAT(_response->GetHeaders().get("Host"), _hostname);
    EXPECT_THAT(_response->GetHeaders().get("Content-Type"), Eq("application/octet-stream"));
    EXPECT_THAT(_response->GetBody(), Eq(Binary("Some\xBB\x11naryContent")));
    _response.release();
}

TEST_F(HttpClientTests, HandlesResponseLargerThanReadBuffer)
{
    Clear();
    // Several times the transport's fixed 8 KB read buffer, so the response can
    // only be assembled by chaining many read completions.
    const size_t responseSize = 300 * 1024;

    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/large/" + std::to_string(responseSize));
    _client->SendRequestAsync(request.release(), this);

    std::unique_ptr<IHttpResponse> response;
    {
        std::unique_lock<std::mutex> lock(_lock);
        ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(30),
            [this]() { return !_responses.empty(); }));
        ASSERT_EQ(_responses.size(), 1u);
        response.reset(_responses[0]);
        _responses.clear();
    }

    EXPECT_THAT(response->GetId(), requestId);
    EXPECT_THAT(response->GetResult(), HttpResult_OK);
    EXPECT_THAT(response->GetStatusCode(), 200u);
    ASSERT_THAT(response->GetBody().size(), responseSize);
    EXPECT_THAT(response->GetBody(), Eq(Binary(LargePayload(responseSize))));
}

TEST_F(HttpClientTests, HandlesRequestAndResponseLargerThanReadBuffer)
{
    Clear();
    // Exercises the send side too: the body is written separately from the
    // request headers, and the echoed response is then drained in chunks.
    const size_t bodySize = 200 * 1024;
    auto body = Binary(LargePayload(bodySize));

    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetMethod("POST");
    request->GetHeaders().set("Content-Type", "application/octet-stream");
    request->SetUrl("http://" + _hostname + "/echo/");
    request->SetBody(body);
    _client->SendRequestAsync(request.release(), this);

    std::unique_ptr<IHttpResponse> response;
    {
        std::unique_lock<std::mutex> lock(_lock);
        ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(30),
            [this]() { return !_responses.empty(); }));
        ASSERT_EQ(_responses.size(), 1u);
        response.reset(_responses[0]);
        _responses.clear();
    }

    EXPECT_THAT(response->GetId(), requestId);
    EXPECT_THAT(response->GetResult(), HttpResult_OK);
    EXPECT_THAT(response->GetStatusCode(), 200u);
    ASSERT_THAT(response->GetBody().size(), bodySize);
    EXPECT_THAT(response->GetBody(), Eq(Binary(LargePayload(bodySize))));
}

TEST_F(HttpClientTests, HandlesCancellationOfLargeResponse)
{
    Clear();
    // Cancel while the response is still being drained through the read buffer:
    // the request must still produce exactly one terminal response, and the
    // buffers WinHTTP was given must outlive it.
    const size_t responseSize = 4 * 1024 * 1024;

    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/large/" + std::to_string(responseSize));
    _client->SendRequestAsync(request.release(), this);
    _client->CancelRequestAsync(requestId);

    std::unique_ptr<IHttpResponse> response;
    {
        std::unique_lock<std::mutex> lock(_lock);
        ASSERT_TRUE(_responseCv.wait_for(lock, std::chrono::seconds(30),
            [this]() { return !_responses.empty(); }));
        ASSERT_EQ(_responses.size(), 1u);
        response.reset(_responses[0]);
        _responses.clear();
    }

    EXPECT_THAT(response->GetId(), requestId);
    // The race is intentional: cancellation may land before or after the
    // response has been fully read, but never both results and never neither.
    EXPECT_TRUE(response->GetResult() == HttpResult_Aborted ||
                response->GetResult() == HttpResult_OK);

    // No duplicate terminal response arrives afterwards.
    std::unique_lock<std::mutex> lock(_lock);
    EXPECT_FALSE(_responseCv.wait_for(lock, std::chrono::milliseconds(500),
        [this]() { return !_responses.empty(); }));
}

TEST_F(HttpClientTests, SurvivesManyRequests)
{
    Clear();

    size_t Count = 100;
    for (size_t i = 0; i < Count; i++) {
        IHttpRequest* request = _client->CreateRequest();
        // _requests.push_back(request);
        request->SetMethod("POST");
        request->GetHeaders().set("expect", "100-continue");
        request->GetHeaders().set("content-type", "application/octet-stream");
        std::ostringstream url;
        url << "http://" << _hostname << "/count/" << i;
        request->SetUrl(url.str());
        auto body = Binary("content");
        request->SetBody(body);
        _countedRequests.push_back(Sent);
        _client->SendRequestAsync(request, this);
        // slowdown if responses are not coming back because SDK itself
        // throttles its HTTP requests at a rate of no more than 2
        // per second
        if (_responses.size() + 2 < i )
            PAL::sleep(100);
    }

    while (_responses.size() < Count) {
        PAL::sleep(100);
    }

    // Verify the count of completed requests by the number of completed responses
    for (auto &v : _responses)
    {
        int id = atoi(std::string(reinterpret_cast<char const*>(v->GetBody().data()), v->GetBody().size()).c_str());
        _countedRequests[id] = Done;
    }

    // Count the number of requests that were not done
    auto it = std::find(_countedRequests.begin(), _countedRequests.end(), Sent);
    EXPECT_THAT(it, _countedRequests.end());

}
#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT
