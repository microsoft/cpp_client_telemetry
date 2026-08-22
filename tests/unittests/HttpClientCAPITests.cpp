//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "common/Common.hpp"

#include "http/HttpClient_CAPI.hpp"
#include "mat.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

using namespace testing;
using namespace MAT;
using std::string;

namespace
{
    class TestHttpResponseCallback : public IHttpResponseCallback
    {
    public:
        void SetResponseValidation(std::function<void(IHttpResponse*)> fn) { m_validateFn = fn; }

        virtual void OnHttpResponse(IHttpResponse* response) override
        {
            std::unique_ptr<IHttpResponse> ownedResponse(response);
            if (m_validateFn)
                m_validateFn(ownedResponse.get());
        }

    private:
        std::function<void(IHttpResponse*)> m_validateFn;
    };

    class TestHelper {
    public:
        void SetShouldSend(bool shouldSend) { m_shouldSend = shouldSend; }
        bool ShouldSend() { return m_shouldSend; }
        void SetSendValidation(std::function<void(http_request_t*)> fn) { m_validateSendFn = fn; }
        void SetCancelValidation(std::function<void(const char*)> fn) { m_validateCancelFn = fn; }

        void OnSend(http_request_t* request, http_complete_fn_t callback)
        {
            m_requestId = request->id;
            m_completeFn = callback;
            if (m_validateSendFn)
                m_validateSendFn(request);
        }

        void OnCancel(const char* requestId)
        {
            if (m_validateCancelFn)
                m_validateCancelFn(requestId);
        }

        void Complete(http_result_t result, http_response_t* response = nullptr)
        {
            if (m_completeFn != nullptr)
            {
                m_completeFn(m_requestId.c_str(), result, response);
            }
        }

    private:
        std::function<void(http_request_t*)> m_validateSendFn;
        std::function<void(const char*)> m_validateCancelFn;
        bool m_shouldSend = false;
        std::string m_requestId;
        http_complete_fn_t m_completeFn = nullptr;
    };

    static std::unique_ptr<TestHelper> s_testHelper;

    // RAII helper that automatically uninstalls static TestHelper instance upon destruction
    class AutoTestHelper {
    public:
        AutoTestHelper()
        {
            s_testHelper.reset(new TestHelper());
        }

        ~AutoTestHelper()
        {
            s_testHelper = nullptr;
        }
        
        TestHelper* operator->()
        {
            return s_testHelper.get();
        }
    };
} // namespace

void EVTSDK_LIBABI_CDECL OnHttpSend(http_request_t* request, http_complete_fn_t callback)
{
    s_testHelper->OnSend(request, callback);

    if (s_testHelper->ShouldSend())
    {
        // Construct simple test response
        uint8_t body[] = {'y', 'e', 's'};
        http_header_t header;
        header.name = "response_key1";
        header.value = "response_value1";
        http_response_t response;
        response.statusCode = 200;
        response.body = body;
        response.bodySize = 3;
        response.headers = &header;
        response.headersCount = 1;

        callback(request->id, HTTP_RESULT_OK, &response);
    }
}

void EVTSDK_LIBABI_CDECL OnHttpSendThrow(
    http_request_t* request,
    http_complete_fn_t callback)
{
    s_testHelper->OnSend(request, callback);
    throw std::runtime_error("send hook failed");
}

void EVTSDK_LIBABI_CDECL OnHttpSendCompleteThenThrow(
    http_request_t* request,
    http_complete_fn_t callback)
{
    s_testHelper->OnSend(request, callback);
    callback(request->id, HTTP_RESULT_OK, nullptr);
    throw std::runtime_error("send hook failed after completion");
}

void EVTSDK_LIBABI_CDECL OnHttpCancel(const char* requestId)
{
    s_testHelper->OnCancel(requestId);
}

TEST(HttpClientCAPITests, SendAsync)
{
    HttpClient_CAPI httpClient(&OnHttpSend, &OnHttpCancel);

    // Build request
    std::vector<uint8_t> body = {'a', 'b', 'c'};
    auto request = httpClient.CreateRequest();
    request->SetUrl("https://www.microsoft.com");
    request->SetBody(body);
    request->SetMethod("POST");
    request->GetHeaders().add("key1", "value1");
    request->GetHeaders().add("key2", "value2");

    AutoTestHelper testHelper;
    testHelper->SetShouldSend(true);

    // Validate C++ -> C transformation of request
    bool wasSent = false;
    testHelper->SetSendValidation([&wasSent](http_request_t* request) {
        wasSent = true;
        EXPECT_EQ(request->type, HTTP_REQUEST_TYPE_POST);
        EXPECT_EQ(string(request->url), string("https://www.microsoft.com"));
        EXPECT_EQ(request->bodySize, 3);
        EXPECT_EQ(request->body[0], 'a');
        EXPECT_EQ(request->body[1], 'b');
        EXPECT_EQ(request->body[2], 'c');
        EXPECT_EQ(request->headersCount, 2);
        EXPECT_EQ(string(request->headers[0].name), string("key1"));
        EXPECT_EQ(string(request->headers[0].value), string("value1"));
        EXPECT_EQ(string(request->headers[1].name), string("key2"));
        EXPECT_EQ(string(request->headers[1].value), string("value2"));
    });

    // Validate C -> C++ transformation of response
    bool wasReceived = false;
    TestHttpResponseCallback responseCallback;
    responseCallback.SetResponseValidation([&wasReceived](IHttpResponse* response) {
        wasReceived = true;
        EXPECT_EQ(response->GetResult(), HttpResult_OK);
        EXPECT_EQ(response->GetBody().size(), size_t { 3 });
        EXPECT_EQ(response->GetBody()[0], 'y');
        EXPECT_EQ(response->GetBody()[1], 'e');
        EXPECT_EQ(response->GetBody()[2], 's');
        EXPECT_EQ(response->GetHeaders().size(), size_t { 1 });
        EXPECT_EQ(response->GetHeaders().has("response_key1"), true);
        EXPECT_EQ(response->GetHeaders().get("response_key1"), string("response_value1"));
    });

    httpClient.SendRequestAsync(request, &responseCallback);

    EXPECT_EQ(wasSent, true);
    EXPECT_EQ(wasReceived, true);
}

TEST(HttpClientCAPITests, Cancel)
{
    HttpClient_CAPI httpClient(&OnHttpSend, &OnHttpCancel);

    // Build request
    auto request = httpClient.CreateRequest();
    request->SetUrl("https://www.microsoft.com");
    request->SetMethod("GET");

    AutoTestHelper testHelper;
    testHelper->SetShouldSend(false);

    string cancelledId;
    testHelper->SetCancelValidation([&cancelledId](const char* requestId) {
        cancelledId = requestId;
    });

    size_t responses = 0;
    TestHttpResponseCallback responseCallback;
    responseCallback.SetResponseValidation([&responses](IHttpResponse* response) {
        ++responses;
        EXPECT_EQ(response->GetResult(), HttpResult_Aborted);
    });

    httpClient.SendRequestAsync(request, &responseCallback);
    httpClient.CancelRequestAsync(request->GetId());

    EXPECT_EQ(cancelledId, request->GetId());
    EXPECT_EQ(responses, 1u);

    // A late external completion is ignored because cancellation already
    // retired and terminally completed the operation.
    testHelper->Complete(HTTP_RESULT_OK);
    EXPECT_EQ(responses, 1u);
}

TEST(HttpClientCAPITests, ThrowingSendRejectsLateCompletion)
{
    HttpClient_CAPI httpClient(&OnHttpSendThrow, &OnHttpCancel);
    auto request = httpClient.CreateRequest();
    request->SetUrl("https://www.microsoft.com");
    request->SetMethod("GET");

    AutoTestHelper testHelper;
    size_t responses = 0;
    TestHttpResponseCallback responseCallback;
    responseCallback.SetResponseValidation([&responses](IHttpResponse*) {
        ++responses;
    });

    EXPECT_THROW(
        httpClient.SendRequestAsync(request, &responseCallback),
        std::runtime_error);
    testHelper->Complete(HTTP_RESULT_OK);
    EXPECT_EQ(responses, 0u);
}

TEST(HttpClientCAPITests, CallbackThenThrowCompletesWithoutExposingException)
{
    HttpClient_CAPI httpClient(&OnHttpSendCompleteThenThrow, &OnHttpCancel);
    auto request = httpClient.CreateRequest();
    request->SetUrl("https://www.microsoft.com");
    request->SetMethod("GET");

    AutoTestHelper testHelper;
    size_t responses = 0;
    TestHttpResponseCallback responseCallback;
    responseCallback.SetResponseValidation([&responses](IHttpResponse* response) {
        ++responses;
        EXPECT_EQ(response->GetResult(), HttpResult_OK);
    });

    EXPECT_NO_THROW(httpClient.SendRequestAsync(request, &responseCallback));
    EXPECT_EQ(responses, 1u);
}

TEST(HttpClientCAPITests, CancelAllCompletesEveryPendingRequest)
{
    HttpClient_CAPI httpClient(&OnHttpSend, &OnHttpCancel);
    AutoTestHelper testHelper;
    testHelper->SetShouldSend(false);

    std::vector<std::unique_ptr<IHttpRequest>> requests;
    std::vector<std::unique_ptr<TestHttpResponseCallback>> callbacks;
    size_t responses = 0;
    for (int i = 0; i < 2; ++i)
    {
        requests.emplace_back(httpClient.CreateRequest());
        requests.back()->SetUrl("https://www.microsoft.com");
        requests.back()->SetMethod("GET");
        callbacks.emplace_back(new TestHttpResponseCallback());
        callbacks.back()->SetResponseValidation(
            [&responses](IHttpResponse* response) {
                ++responses;
                EXPECT_EQ(response->GetResult(), HttpResult_Aborted);
            });
        httpClient.SendRequestAsync(requests.back().get(), callbacks.back().get());
    }

    httpClient.CancelAllRequests();
    EXPECT_EQ(responses, 2u);

    testHelper->Complete(HTTP_RESULT_OK);
    EXPECT_EQ(responses, 2u);
}

TEST(HttpClientCAPITests, CancelWaitsForSendHookToReleaseRequestBuffers)
{
    HttpClient_CAPI httpClient(&OnHttpSend, &OnHttpCancel);
    auto request = std::unique_ptr<IHttpRequest>(httpClient.CreateRequest());
    request->SetUrl("https://www.microsoft.com");
    request->SetMethod("GET");

    AutoTestHelper testHelper;
    testHelper->SetShouldSend(false);
    std::mutex gateMutex;
    std::condition_variable gateCV;
    bool sendEntered = false;
    bool releaseSend = false;
    testHelper->SetSendValidation(
        [&](http_request_t* capiRequest) {
            std::unique_lock<std::mutex> lock(gateMutex);
            EXPECT_STREQ(capiRequest->id, request->GetId().c_str());
            sendEntered = true;
            gateCV.notify_all();
            gateCV.wait(lock, [&] { return releaseSend; });
        });

    std::atomic<size_t> responses{0};
    TestHttpResponseCallback responseCallback;
    responseCallback.SetResponseValidation(
        [&](IHttpResponse* response) {
            EXPECT_EQ(response->GetResult(), HttpResult_Aborted);
            ++responses;
        });

    std::thread sender([&] {
        httpClient.SendRequestAsync(request.get(), &responseCallback);
    });
    {
        std::unique_lock<std::mutex> lock(gateMutex);
        ASSERT_TRUE(gateCV.wait_for(
            lock, std::chrono::seconds(5), [&] { return sendEntered; }));
    }

    std::thread canceller([&] {
        httpClient.CancelRequestAsync(request->GetId());
    });
    PAL::sleep(50);
    EXPECT_EQ(responses.load(), 0u);

    {
        std::lock_guard<std::mutex> lock(gateMutex);
        releaseSend = true;
    }
    gateCV.notify_all();
    sender.join();
    canceller.join();
    EXPECT_EQ(responses.load(), 1u);
}

TEST(HttpClientCAPITests, CancelAllOnlyCompletesOwningClient)
{
    HttpClient_CAPI firstClient(&OnHttpSend, &OnHttpCancel);
    HttpClient_CAPI secondClient(&OnHttpSend, &OnHttpCancel);
    AutoTestHelper testHelper;
    testHelper->SetShouldSend(false);

    auto firstRequest = std::unique_ptr<IHttpRequest>(firstClient.CreateRequest());
    auto secondRequest = std::unique_ptr<IHttpRequest>(secondClient.CreateRequest());
    firstRequest->SetUrl("https://www.microsoft.com");
    secondRequest->SetUrl("https://www.microsoft.com");

    size_t firstResponses = 0;
    size_t secondResponses = 0;
    TestHttpResponseCallback firstCallback;
    TestHttpResponseCallback secondCallback;
    firstCallback.SetResponseValidation([&](IHttpResponse* response) {
        ++firstResponses;
        EXPECT_EQ(response->GetResult(), HttpResult_Aborted);
    });
    secondCallback.SetResponseValidation([&](IHttpResponse* response) {
        ++secondResponses;
        EXPECT_EQ(response->GetResult(), HttpResult_Aborted);
    });

    firstClient.SendRequestAsync(firstRequest.get(), &firstCallback);
    secondClient.SendRequestAsync(secondRequest.get(), &secondCallback);

    firstClient.CancelAllRequests();
    EXPECT_EQ(firstResponses, 1u);
    EXPECT_EQ(secondResponses, 0u);

    secondClient.CancelAllRequests();
    EXPECT_EQ(secondResponses, 1u);
}

TEST(HttpClientCAPITests, DestructorCompletesPendingRequestAndIgnoresLateResponse)
{
    AutoTestHelper testHelper;
    testHelper->SetShouldSend(false);

    size_t responses = 0;
    TestHttpResponseCallback callback;
    callback.SetResponseValidation([&](IHttpResponse* response) {
        ++responses;
        EXPECT_EQ(response->GetResult(), HttpResult_Aborted);
    });

    {
        HttpClient_CAPI httpClient(&OnHttpSend, &OnHttpCancel);
        auto request = std::unique_ptr<IHttpRequest>(httpClient.CreateRequest());
        request->SetUrl("https://www.microsoft.com");
        request->SetMethod("GET");
        httpClient.SendRequestAsync(request.get(), &callback);
    }

    EXPECT_EQ(responses, 1u);
    testHelper->Complete(HTTP_RESULT_OK);
    EXPECT_EQ(responses, 1u);
}

TEST(HttpClientCAPITests, SynchronousCallbackCanDestroyClient)
{
    AutoTestHelper testHelper;
    testHelper->SetShouldSend(true);

    auto httpClient = std::unique_ptr<HttpClient_CAPI>(
        new HttpClient_CAPI(&OnHttpSend, &OnHttpCancel));
    auto request = std::unique_ptr<IHttpRequest>(httpClient->CreateRequest());
    request->SetUrl("https://www.microsoft.com");
    request->SetMethod("GET");

    TestHttpResponseCallback callback;
    callback.SetResponseValidation([&](IHttpResponse* response) {
        EXPECT_EQ(response->GetResult(), HttpResult_OK);
        httpClient.reset();
    });

    EXPECT_NO_THROW(httpClient->SendRequestAsync(request.get(), &callback));
    EXPECT_EQ(httpClient, nullptr);
}

TEST(HttpClientCAPITests, CancelAllThenSend)
{
    HttpClient_CAPI httpClient(&OnHttpSend, &OnHttpCancel);

    AutoTestHelper testHelper;
    testHelper->SetShouldSend(true);

    // Cancel all requests (none pending)
    httpClient.CancelAllRequests();

    // Build request
    auto request = httpClient.CreateRequest();
    request->SetUrl("https://www.microsoft.com");
    request->SetMethod("GET");
    request->GetHeaders().add("key1", "value1");

    // Validate C++ -> C transformation of request
    bool wasSent = false;
    testHelper->SetSendValidation([&wasSent](http_request_t* request) {
        wasSent = true;
        EXPECT_EQ(request->type, HTTP_REQUEST_TYPE_GET);
        EXPECT_EQ(request->url, string("https://www.microsoft.com"));
        EXPECT_EQ(request->bodySize, 0);
        EXPECT_EQ(request->headersCount, 1);
        EXPECT_EQ(string(request->headers[0].name), string("key1"));
        EXPECT_EQ(string(request->headers[0].value), string("value1"));
    });

    // Validate C -> C++ transformation of response
    bool wasReceived = false;
    TestHttpResponseCallback responseCallback;
    responseCallback.SetResponseValidation([&wasReceived](IHttpResponse* response) {
        wasReceived = true;
        EXPECT_EQ(response->GetResult(), HttpResult_OK);
        EXPECT_EQ(response->GetBody().size(), size_t { 3 });
        EXPECT_EQ(response->GetBody()[0], 'y');
        EXPECT_EQ(response->GetBody()[1], 'e');
        EXPECT_EQ(response->GetBody()[2], 's');
        EXPECT_EQ(response->GetHeaders().size(), size_t { 1 });
        EXPECT_EQ(response->GetHeaders().has("response_key1"), true);
        EXPECT_EQ(response->GetHeaders().get("response_key1"), string("response_value1"));
    });

    httpClient.SendRequestAsync(request, &responseCallback);

    EXPECT_EQ(wasSent, true);
    EXPECT_EQ(wasReceived, true);
}
