// Copyright (c) Microsoft. All rights reserved .

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
