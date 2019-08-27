// Copyright (c) Microsoft. All rights reserved.
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
        void ValidateResponse(std::function<void(IHttpResponse*)> fn) { mValidateFn = fn; }

        virtual void OnHttpResponse(IHttpResponse* response) override
        {
            if (mValidateFn)
                mValidateFn(response);
        }

    private:
        std::function<void(IHttpResponse*)> mValidateFn;
    };

    class TestHelper {
    public:
        void SetShouldSend(bool shouldSend) { mShouldSend = shouldSend; }
        bool ShouldSend() { return mShouldSend; }
        void ValidateSend(std::function<void(http_request_t*)> fn) { mValidateSendFn = fn; }
        void ValidateCancel(std::function<void(const char*)> fn) { mValidateCancelFn = fn; }

        void OnSend(http_request_t* request)
        {
            if (mValidateSendFn)
                mValidateSendFn(request);
        }

        void OnCancel(const char* requestId)
        {
            if (mValidateCancelFn)
                mValidateCancelFn(requestId);
        }

    private:
        std::function<void(http_request_t*)> mValidateSendFn;
        std::function<void(const char*)> mValidateCancelFn;
        bool mShouldSend = false;
    };

    static std::unique_ptr<TestHelper> kTestHelper;

    // RAII helper that automatically uninstalls static TestHelper instance upon destruction
    class AutoTestHelper {
    public:
        AutoTestHelper()
        {
            kTestHelper.reset(new TestHelper());
        }

        ~AutoTestHelper()
        {
            kTestHelper = nullptr;
        }
        
        TestHelper* operator->()
        {
            return kTestHelper.get();
        }
    };
} // namespace

void EVTSDK_LIBABI_CDECL OnHttpSend(http_request_t* request, http_complete_fn_t callback)
{
    kTestHelper->OnSend(request);

    if (kTestHelper->ShouldSend())
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

void EVTSDK_LIBABI_CDECL OnHttpCancel(const char* requestId)
{
    kTestHelper->OnCancel(requestId);
}

TEST(HttpClientCAPITests, SendAsync)
{
    auto httpClient = std::make_shared<HttpClient_CAPI>(&OnHttpSend, &OnHttpCancel);
    
    // Build request
    std::vector<uint8_t> body = {'a', 'b', 'c'};
    auto request = httpClient->CreateRequest();
    request->SetUrl("https://www.microsoft.com");
    request->SetBody(body);
    request->SetMethod("POST");
    request->GetHeaders().add("key1", "value1");
    request->GetHeaders().add("key2", "value2");

    AutoTestHelper testHelper;
    testHelper->SetShouldSend(true);

    // Validate C++ -> C transformation of request
    bool wasSent = false;
    testHelper->ValidateSend([&wasSent](http_request_t* request) {
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
    auto responseCallback = std::make_shared<TestHttpResponseCallback>();
    responseCallback->ValidateResponse([&wasReceived](IHttpResponse* response) {
        wasReceived = true;
        EXPECT_EQ(response->GetResult(), HttpResult_OK);
        EXPECT_EQ(response->GetBody().size(), 3);
        EXPECT_EQ(response->GetBody()[0], 'y');
        EXPECT_EQ(response->GetBody()[1], 'e');
        EXPECT_EQ(response->GetBody()[2], 's');
        EXPECT_EQ(response->GetHeaders().size(), 1);
        EXPECT_EQ(response->GetHeaders().has("response_key1"), true);
        EXPECT_EQ(response->GetHeaders().get("response_key1"), string("response_value1"));
    });

    httpClient->SendRequestAsync(request, responseCallback.get());

    EXPECT_EQ(wasSent, true);
    EXPECT_EQ(wasReceived, true);
}

TEST(HttpClientCAPITests, Cancel)
{
    auto httpClient = std::make_shared<HttpClient_CAPI>(&OnHttpSend, &OnHttpCancel);
    
    // Build request
    auto request = httpClient->CreateRequest();
    request->SetUrl("https://www.microsoft.com");
    request->SetMethod("GET");

    AutoTestHelper testHelper;
    testHelper->SetShouldSend(false);

    string cancelledId;
    testHelper->ValidateCancel([&cancelledId](const char* requestId) {
        cancelledId = requestId;
    });

    auto responseCallback = std::make_shared<TestHttpResponseCallback>();
    responseCallback->ValidateResponse([](IHttpResponse* /*response*/) {
        FAIL() << "No response should have been received";
    });

    httpClient->SendRequestAsync(request, responseCallback.get());
    httpClient->CancelRequestAsync(request->GetId());

    EXPECT_EQ(cancelledId, request->GetId());
}

TEST(HttpClientCAPITests, CancelAllThenSend)
{
    auto httpClient = std::make_shared<HttpClient_CAPI>(&OnHttpSend, &OnHttpCancel);

    AutoTestHelper testHelper;
    testHelper->SetShouldSend(true);

    // Cancel all requests (none pending)
    httpClient->CancelAllRequests();

    // Build request
    auto request = httpClient->CreateRequest();
    request->SetUrl("https://www.microsoft.com");
    request->SetMethod("GET");
    request->GetHeaders().add("key1", "value1");

    // Validate C++ -> C transformation of request
    bool wasSent = false;
    testHelper->ValidateSend([&wasSent](http_request_t* request) {
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
    auto responseCallback = std::make_shared<TestHttpResponseCallback>();
    responseCallback->ValidateResponse([&wasReceived](IHttpResponse* response) {
        wasReceived = true;
        EXPECT_EQ(response->GetResult(), HttpResult_OK);
        EXPECT_EQ(response->GetBody().size(), 3);
        EXPECT_EQ(response->GetBody()[0], 'y');
        EXPECT_EQ(response->GetBody()[1], 'e');
        EXPECT_EQ(response->GetBody()[2], 's');
        EXPECT_EQ(response->GetHeaders().size(), 1);
        EXPECT_EQ(response->GetHeaders().has("response_key1"), true);
        EXPECT_EQ(response->GetHeaders().get("response_key1"), string("response_value1"));
    });

    httpClient->SendRequestAsync(request, responseCallback.get());

    EXPECT_EQ(wasSent, true);
    EXPECT_EQ(wasReceived, true);
}