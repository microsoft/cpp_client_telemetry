//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "common/Common.hpp"

#include "http/HttpClient_CAPI.hpp"
#include "mat.h"

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

        void OnSend(http_request_t* request, http_complete_fn_t completeFn)
        {
            m_requestId = request->id;
            m_completeFn = completeFn;
            if (m_validateSendFn)
                m_validateSendFn(request);
        }

        void OnCancel(const char* requestId)
        {
            if (m_validateCancelFn)
                m_validateCancelFn(requestId);
            Complete(HTTP_RESULT_CANCELLED, nullptr);
        }

        void Complete(http_result_t result, http_response_t* response)
        {
            auto completeFn = m_completeFn;
            m_completeFn = nullptr;
            if (completeFn != nullptr)
                completeFn(m_requestId.c_str(), result, response);
        }

    private:
        std::function<void(http_request_t*)> m_validateSendFn;
        std::function<void(const char*)> m_validateCancelFn;
        std::string m_requestId;
        http_complete_fn_t m_completeFn = nullptr;
        bool m_shouldSend = false;
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

        s_testHelper->Complete(HTTP_RESULT_OK, &response);
    }
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
    std::unique_ptr<IHttpRequest> request(httpClient.CreateRequest());
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

    httpClient.SendRequestAsync(request.get(), &responseCallback);

    EXPECT_EQ(wasSent, true);
    EXPECT_EQ(wasReceived, true);
}

TEST(HttpClientCAPITests, Cancel)
{
    HttpClient_CAPI httpClient(&OnHttpSend, &OnHttpCancel);

    // Build request
    std::unique_ptr<IHttpRequest> request(httpClient.CreateRequest());
    request->SetUrl("https://www.microsoft.com");
    request->SetMethod("GET");
    const auto requestId = request->GetId();

    AutoTestHelper testHelper;
    testHelper->SetShouldSend(false);

    string cancelledId;
    testHelper->SetCancelValidation([&cancelledId](const char* requestId) {
        cancelledId = requestId;
    });

    TestHttpResponseCallback responseCallback;
    bool wasReceived = false;
    responseCallback.SetResponseValidation([&wasReceived](IHttpResponse* response) {
        wasReceived = true;
        EXPECT_EQ(response->GetResult(), HttpResult_Aborted);
    });

    httpClient.SendRequestAsync(request.get(), &responseCallback);
    httpClient.CancelRequestAsync(requestId);

    EXPECT_EQ(cancelledId, requestId);
    EXPECT_TRUE(wasReceived);
}

TEST(HttpClientCAPITests, CancelAllThenSend)
{
    HttpClient_CAPI httpClient(&OnHttpSend, &OnHttpCancel);

    AutoTestHelper testHelper;
    testHelper->SetShouldSend(true);

    // Cancel all requests (none pending)
    httpClient.CancelAllRequests();

    // Build request
    std::unique_ptr<IHttpRequest> request(httpClient.CreateRequest());
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

    httpClient.SendRequestAsync(request.get(), &responseCallback);

    EXPECT_EQ(wasSent, true);
    EXPECT_EQ(wasReceived, true);
}
