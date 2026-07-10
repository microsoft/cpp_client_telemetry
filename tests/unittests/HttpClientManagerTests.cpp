// Copyright (c) Microsoft Corporation. All rights reserved.

#include "common/Common.hpp"
#include "common/MockIHttpClient.hpp"
#include "http/HttpClientManager.hpp"

#include "NullObjects.hpp"
#include "ILogManager.hpp"

using namespace testing;
using namespace MAT;

static NullLogManager dummyLogManager;

class HttpClientManager4Test : public HttpClientManager {
  public:
    HttpClientManager4Test(IHttpClient& httpClient)
      : HttpClientManager(dummyLogManager, httpClient, *PAL::getDefaultTaskDispatcher())
    {
    }

    virtual void scheduleOnHttpResponse(HttpCallback* callback) override
    {
        onHttpResponse(callback);
    }

    void setCancelDrainTimeout(std::chrono::milliseconds t)
    {
        m_cancelDrainTimeout = t;
    }
};

class HttpClientManagerTests : public StrictMock<Test> {
  protected:
    MockIHttpClient        httpClientMock;
    HttpClientManager4Test hcm;

    RouteSink<HttpClientManagerTests, EventsUploadContextPtr const&> requestDone{this, &HttpClientManagerTests::resultRequestDone};

  protected:
    HttpClientManagerTests()
      : hcm(httpClientMock)
    {
        hcm.requestDone >> requestDone;
    }

    MOCK_METHOD1(resultRequestDone, void(EventsUploadContextPtr const &));
};


TEST_F(HttpClientManagerTests, HandlesRequestFlow)
{
    SimpleHttpRequest* req = new SimpleHttpRequest("HttpClientManagerTests");

    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->httpRequestId = req->GetId();
    ctx->httpRequest = req;
    ctx->recordIdsAndTenantIds["r1"] = "t1"; ctx->recordIdsAndTenantIds["r2"] = "t1";
    ctx->latency = EventLatency_Normal;
    ctx->packageIds["tenant1-token"] = 0;

    IHttpResponseCallback* callback = nullptr;
    EXPECT_CALL(httpClientMock, SendRequestAsync(ctx->httpRequest, _))
        .WillOnce(SaveArg<1>(&callback));
    hcm.sendRequest(ctx);
    ASSERT_THAT(callback, NotNull());

    PAL::sleep(200);

    std::unique_ptr<SimpleHttpResponse> rsp(new SimpleHttpResponse("HttpClientManagerTests"));
    rsp->m_result = HttpResult_OK;
    rsp->m_statusCode = 200;

    EXPECT_CALL(*this, resultRequestDone(ctx))
        .WillOnce(Return());
    IHttpResponse* rspRef = rsp.get();
    callback->OnHttpResponse(rsp.release());

    EXPECT_THAT(ctx->httpResponse, rspRef);
    EXPECT_THAT(ctx->durationMs, Gt(199));
}

// Regression test: cancelAllRequests() must not spin/hang forever
// when an in-flight callback never drains (e.g. the dispatcher or HTTP stack is
// stalled). It waits for the drain via a condition variable, bounded by a timeout.
TEST_F(HttpClientManagerTests, CancelAllRequests_TimesOutInsteadOfHanging)
{
    hcm.setCancelDrainTimeout(std::chrono::milliseconds(150));

    SimpleHttpRequest* req = new SimpleHttpRequest("stall");
    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->httpRequestId = req->GetId();
    ctx->httpRequest = req;
    ctx->recordIdsAndTenantIds["r1"] = "t1";
    ctx->latency = EventLatency_Normal;
    ctx->packageIds["tenant1-token"] = 0;

    IHttpResponseCallback* callback = nullptr;
    EXPECT_CALL(httpClientMock, SendRequestAsync(ctx->httpRequest, _))
        .WillOnce(SaveArg<1>(&callback));
    hcm.sendRequest(ctx);
    ASSERT_THAT(callback, NotNull());

    // The response never arrives, so the callback never drains from m_httpCallbacks.
    // The best-effort (pause) drain must still return, bounded by the drain timeout,
    // rather than block forever (MockIHttpClient does not mock CancelAllRequests, so
    // the base no-op runs and nothing completes the request).
    auto start = std::chrono::steady_clock::now();
    hcm.cancelAllRequests(/* bestEffort */ true);
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    EXPECT_THAT(elapsedMs, Ge(100));    // waited a meaningful fraction of the 150ms timeout, not an immediate return
    EXPECT_THAT(elapsedMs, Lt(5000));   // but did not hang

    // Drain the still-outstanding callback so nothing leaks, and confirm it is still
    // safe to complete after cancelAllRequests abandoned the drain.
    EXPECT_CALL(*this, resultRequestDone(ctx)).WillOnce(Return());
    callback->OnHttpResponse(new SimpleHttpResponse("stall"));
}
