// Copyright (c) Microsoft Corporation. All rights reserved.

#include "common/Common.hpp"
#include "common/MockIHttpClient.hpp"
#include "http/IBoundedHttpClientCancel.hpp"
#include "http/HttpClientManager.hpp"
#include "pal/TaskDispatcher.hpp"

#include "NullObjects.hpp"
#include "ILogManager.hpp"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>

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

class AsyncHttpClientManager4Test : public HttpClientManager {
  public:
    AsyncHttpClientManager4Test(IHttpClient& httpClient)
      : HttpClientManager(dummyLogManager, httpClient, *PAL::getDefaultTaskDispatcher())
    {
    }

    AsyncHttpClientManager4Test(
        IHttpClient& httpClient,
        ITaskDispatcher& taskDispatcher) :
        HttpClientManager(dummyLogManager, httpClient, taskDispatcher)
    {
    }

    void setCancelDrainTimeout(std::chrono::milliseconds timeout)
    {
        m_cancelDrainTimeout = timeout;
    }

    bool waitForRequestsToDrain(std::chrono::milliseconds timeout)
    {
        std::unique_lock<std::mutex> lock(m_httpCallbacksMtx);
        return m_httpCallbacksCV.wait_for(
            lock, timeout, [this]() { return m_httpCallbacks.empty(); });
    }
};

class ReentrantAsyncCompletionReceiver {
  public:
    void onRequestDone(EventsUploadContextPtr const& ctx)
    {
        if (ctx->httpRequestId == "async-reentrant-first")
        {
            {
                std::unique_lock<std::mutex> lock(mutex);
                firstEntered = true;
                cv.notify_all();
                cv.wait(lock, [this]() { return releaseFirst; });
            }
            auto start = std::chrono::steady_clock::now();
            manager->cancelAllRequests(/* bestEffort */ true);
            cancelDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start);
        }

        {
            std::lock_guard<std::mutex> lock(mutex);
            ++completed;
            cv.notify_all();
        }
    }

    HttpClientManager* manager {nullptr};
    std::mutex mutex;
    std::condition_variable cv;
    bool firstEntered {false};
    bool releaseFirst {false};
    size_t completed {0};
    std::chrono::milliseconds cancelDuration {0};
    RouteSink<ReentrantAsyncCompletionReceiver, EventsUploadContextPtr const&>
        sink {this, &ReentrantAsyncCompletionReceiver::onRequestDone};
};

class BlockingAsyncCompletionReceiver {
  public:
    void onRequestDone(EventsUploadContextPtr const&)
    {
        std::unique_lock<std::mutex> lock(mutex);
        entered = true;
        cv.notify_all();
        cv.wait(lock, [this]() { return released; });
    }

    std::mutex mutex;
    std::condition_variable cv;
    bool entered {false};
    bool released {false};
    RouteSink<BlockingAsyncCompletionReceiver, EventsUploadContextPtr const&>
        sink {this, &BlockingAsyncCompletionReceiver::onRequestDone};
};

class QueuedHttpResponseDelivery {
  public:
    void deliver(IHttpResponseCallback* callback, IHttpResponse* response)
    {
        callback->OnHttpResponse(response);
        {
            std::lock_guard<std::mutex> lock(mutex);
            ++completed;
        }
        cv.notify_all();
    }

    bool waitFor(size_t count)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, std::chrono::seconds(5),
            [this, count]() { return completed == count; });
    }

    std::mutex mutex;
    std::condition_variable cv;
    size_t completed {0};
};

class HttpRequestDoneReceiver
{
   public:
    MOCK_METHOD1(onRequestDone, void(EventsUploadContextPtr const&));

    RouteSink<HttpRequestDoneReceiver, EventsUploadContextPtr const&>
        sink{this, &HttpRequestDoneReceiver::onRequestDone};
};

class HttpClientManagerTests : public StrictMock<Test> {
  protected:
    MockIHttpClient        httpClientMock;
    HttpClientManager4Test hcm;

    RouteSink<HttpClientManagerTests, EventsUploadContextPtr const&> requestDone{this, &HttpClientManagerTests::resultRequestDone};
    RouteSink<HttpClientManagerTests, EventsUploadContextPtr const&> requestFailed{this, &HttpClientManagerTests::resultRequestFailed};
    RouteSink<HttpClientManagerTests, EventsUploadContextPtr const&> requestFailureComplete{this, &HttpClientManagerTests::resultRequestFailureComplete};

  protected:
    HttpClientManagerTests()
      : hcm(httpClientMock)
    {
        hcm.requestDone >> requestDone;
        hcm.requestFailed >> requestFailed;
        hcm.requestFailureComplete >> requestFailureComplete;
    }

    MOCK_METHOD1(resultRequestDone, void(EventsUploadContextPtr const &));
    MOCK_METHOD1(resultRequestFailed, void(EventsUploadContextPtr const &));
    MOCK_METHOD1(resultRequestFailureComplete, void(EventsUploadContextPtr const &));
};

class MockBoundedIHttpClient : public MockIHttpClient, public IBoundedHttpClientCancel {
  public:
    using MockIHttpClient::CancelAllRequests;
    MOCK_METHOD1(CancelAllRequests, void(std::chrono::milliseconds));
};

class ThrowingCancelAllHttpClient : public MockIHttpClient {
  public:
    void CancelAllRequests() override
    {
        throw std::runtime_error("cancel all failed");
    }
};

#ifndef _WIN32
class DroppingHttpResponseTaskDispatcher : public ITaskDispatcher
{
   public:
    void Join() override
    {
    }
    void Queue(Task* task) override
    {
        delete task;
    }
    bool Cancel(Task*, uint64_t = 0) override
    {
        return false;
    }
};

#if HAVE_EXCEPTIONS
class ThrowingHttpResponseTaskDispatcher : public ITaskDispatcher
{
   public:
    void Join() override
    {
    }
    void Queue(Task* task) override
    {
        delete task;
        throw std::runtime_error("queue failed");
    }
    bool Cancel(Task*, uint64_t = 0) override
    {
        return false;
    }
};
#endif
#endif

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

TEST_F(HttpClientManagerTests, ThrowingRequestDoneSettlesFailureAndDrainsCallback)
{
    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->httpRequest = new SimpleHttpRequest("throwing-request-done");
    ctx->httpRequestId = ctx->httpRequest->GetId();
    ctx->recordIdsAndTenantIds["r1"] = "t1";
    ctx->latency = EventLatency_Normal;
    ctx->packageIds["tenant1-token"] = 0;

    IHttpResponseCallback* callback = nullptr;
    EXPECT_CALL(httpClientMock, SendRequestAsync(ctx->httpRequest, _))
        .WillOnce(SaveArg<1>(&callback));
    hcm.sendRequest(ctx);
    ASSERT_THAT(callback, NotNull());

    EXPECT_CALL(*this, resultRequestDone(ctx))
        .WillOnce(Throw(std::runtime_error("listener failed")));
    {
        InSequence sequence;
        EXPECT_CALL(*this, resultRequestFailed(ctx));
        EXPECT_CALL(*this, resultRequestFailureComplete(ctx));
    }

    EXPECT_NO_THROW(callback->OnHttpResponse(new SimpleHttpResponse("throwing-request-done")));
    EXPECT_THAT(hcm.requestCount(), 0u);
}

TEST_F(HttpClientManagerTests, ThrowingFailureReleaseStillCompletesRequest)
{
    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->httpRequest = new SimpleHttpRequest("throwing-failure-release");
    ctx->httpRequestId = ctx->httpRequest->GetId();
    ctx->recordIdsAndTenantIds["r1"] = "t1";
    ctx->latency = EventLatency_Normal;
    ctx->packageIds["tenant1-token"] = 0;

    IHttpResponseCallback* callback = nullptr;
    EXPECT_CALL(httpClientMock, SendRequestAsync(ctx->httpRequest, _))
        .WillOnce(SaveArg<1>(&callback));
    hcm.sendRequest(ctx);
    ASSERT_THAT(callback, NotNull());

    EXPECT_CALL(*this, resultRequestDone(ctx))
        .WillOnce(Throw(std::runtime_error("listener failed")));
    EXPECT_CALL(*this, resultRequestFailed(ctx))
        .WillOnce(Throw(std::runtime_error("release failed")));
    EXPECT_CALL(*this, resultRequestFailureComplete(ctx));

    EXPECT_NO_THROW(callback->OnHttpResponse(
        new SimpleHttpResponse("throwing-failure-release")));
    EXPECT_THAT(hcm.requestCount(), 0u);
}

TEST_F(HttpClientManagerTests, ThrowingSendProducesOneTerminalFailure)
{
    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->httpRequest = new SimpleHttpRequest("throwing-send");
    ctx->httpRequestId = ctx->httpRequest->GetId();
    ctx->recordIdsAndTenantIds["r1"] = "t1";
    ctx->latency = EventLatency_Normal;
    ctx->packageIds["tenant1-token"] = 0;

    EXPECT_CALL(httpClientMock, SendRequestAsync(ctx->httpRequest, _))
        .WillOnce(Throw(std::runtime_error("send failed")));
    EXPECT_CALL(*this, resultRequestDone(ctx))
        .WillOnce(Invoke([](EventsUploadContextPtr const& completed) {
            ASSERT_THAT(completed->httpResponse, NotNull());
            EXPECT_EQ(completed->httpResponse->GetId(), "throwing-send");
            EXPECT_EQ(completed->httpResponse->GetResult(), HttpResult_LocalFailure);
        }));

    EXPECT_NO_THROW(hcm.sendRequest(ctx));
    EXPECT_THAT(hcm.requestCount(), 0u);
}

TEST_F(HttpClientManagerTests, CallbackThenThrowDoesNotCompleteTwice)
{
    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->httpRequest = new SimpleHttpRequest("callback-then-throw");
    ctx->httpRequestId = ctx->httpRequest->GetId();
    ctx->recordIdsAndTenantIds["r1"] = "t1";
    ctx->latency = EventLatency_Normal;
    ctx->packageIds["tenant1-token"] = 0;

    EXPECT_CALL(*this, resultRequestDone(ctx))
        .WillOnce(Invoke([](EventsUploadContextPtr const& completed) {
            ASSERT_THAT(completed->httpResponse, NotNull());
            EXPECT_EQ(completed->httpResponse->GetId(), "original-response");
            EXPECT_EQ(completed->httpResponse->GetResult(), HttpResult_OK);
        }));
    EXPECT_CALL(httpClientMock, SendRequestAsync(ctx->httpRequest, _))
        .WillOnce(Invoke([](IHttpRequest*, IHttpResponseCallback* callback) {
            auto response = new SimpleHttpResponse("original-response");
            response->m_result = HttpResult_OK;
            callback->OnHttpResponse(response);
            throw std::runtime_error("invalid throw after callback");
        }));

    EXPECT_NO_THROW(hcm.sendRequest(ctx));
    EXPECT_THAT(hcm.requestCount(), 0u);
}

TEST_F(HttpClientManagerTests, RequestDoneCanCancelAllRequests)
{
    SimpleHttpRequest* req = new SimpleHttpRequest("reentrant-cancel");
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

    EXPECT_CALL(*this, resultRequestDone(ctx))
        .WillOnce(Invoke([this](EventsUploadContextPtr const&) {
            hcm.cancelAllRequests();
        }));
    callback->OnHttpResponse(new SimpleHttpResponse("reentrant-cancel"));

    EXPECT_THAT(hcm.requestCount(), 0u);
}

TEST_F(HttpClientManagerTests, ConcurrentRequestDoneCallbacksCanCancelAllRequests)
{
    std::vector<IHttpResponseCallback*> callbacks;
    std::vector<EventsUploadContextPtr> contexts;
    for (size_t i = 0; i < 2; ++i)
    {
        auto ctx = std::make_shared<EventsUploadContext>();
        ctx->httpRequest = new SimpleHttpRequest(
            "concurrent-reentrant-cancel-" + std::to_string(i));
        ctx->httpRequestId = ctx->httpRequest->GetId();
        ctx->recordIdsAndTenantIds["r1"] = "t1";
        ctx->latency = EventLatency_Normal;
        ctx->packageIds["tenant1-token"] = 0;

        IHttpResponseCallback* callback = nullptr;
        EXPECT_CALL(httpClientMock, SendRequestAsync(ctx->httpRequest, _))
            .WillOnce(SaveArg<1>(&callback));
        hcm.sendRequest(ctx);
        ASSERT_THAT(callback, NotNull());
        callbacks.push_back(callback);
        contexts.push_back(std::move(ctx));
    }

    std::mutex barrierMutex;
    std::condition_variable barrierCv;
    size_t callbacksEntered = 0;
    EXPECT_CALL(*this, resultRequestDone(_))
        .Times(2)
        .WillRepeatedly(Invoke([this, &barrierMutex, &barrierCv, &callbacksEntered](
                                  EventsUploadContextPtr const&) {
            {
                std::unique_lock<std::mutex> lock(barrierMutex);
                ++callbacksEntered;
                barrierCv.notify_all();
                barrierCv.wait_for(lock, std::chrono::seconds(5),
                    [&callbacksEntered]() { return callbacksEntered == 2; });
            }
            hcm.cancelAllRequests();
        }));

    std::thread first([&callbacks]() {
        callbacks[0]->OnHttpResponse(
            new SimpleHttpResponse("concurrent-reentrant-cancel-0"));
    });
    std::thread second([&callbacks]() {
        callbacks[1]->OnHttpResponse(
            new SimpleHttpResponse("concurrent-reentrant-cancel-1"));
    });
    first.join();
    second.join();

    EXPECT_THAT(callbacksEntered, 2u);
    EXPECT_THAT(hcm.requestCount(), 0u);
}

TEST(HttpClientManagerAsyncTests, ReentrantCancelDoesNotBlockQueuedCallbacks)
{
    MockIHttpClient httpClient;
    AsyncHttpClientManager4Test manager(httpClient);
    manager.setCancelDrainTimeout(std::chrono::seconds(1));
    ReentrantAsyncCompletionReceiver receiver;
    receiver.manager = &manager;
    manager.requestDone >> receiver.sink;

    std::vector<IHttpResponseCallback*> callbacks;
    for (const char* id : {"async-reentrant-first", "async-reentrant-second"})
    {
        auto ctx = std::make_shared<EventsUploadContext>();
        ctx->httpRequest = new SimpleHttpRequest(id);
        ctx->httpRequestId = id;
        ctx->recordIdsAndTenantIds["r1"] = "t1";
        ctx->latency = EventLatency_Normal;
        ctx->packageIds["tenant1-token"] = 0;

        IHttpResponseCallback* callback = nullptr;
        EXPECT_CALL(httpClient, SendRequestAsync(ctx->httpRequest, _))
            .WillOnce(SaveArg<1>(&callback));
        manager.sendRequest(ctx);
        ASSERT_THAT(callback, NotNull());
        callbacks.push_back(callback);
    }

    EXPECT_CALL(httpClient, CancelRequestAsync("async-reentrant-first"));
    EXPECT_CALL(httpClient, CancelRequestAsync("async-reentrant-second"));

    QueuedHttpResponseDelivery delivery;
    auto dispatcher = PAL::getDefaultTaskDispatcher();
    PAL::scheduleTask(
        dispatcher.get(), 0, &delivery, &QueuedHttpResponseDelivery::deliver,
        callbacks[0], new SimpleHttpResponse("async-reentrant-first"));
    {
        std::unique_lock<std::mutex> lock(receiver.mutex);
        ASSERT_TRUE(receiver.cv.wait_for(lock, std::chrono::seconds(5),
            [&receiver]() { return receiver.firstEntered; }));
    }

    // This completion is now queued behind the first one on PAL's default
    // single-thread dispatcher.
    PAL::scheduleTask(
        dispatcher.get(), 0, &delivery, &QueuedHttpResponseDelivery::deliver,
        callbacks[1], new SimpleHttpResponse("async-reentrant-second"));
    {
        std::lock_guard<std::mutex> lock(receiver.mutex);
        receiver.releaseFirst = true;
    }
    receiver.cv.notify_all();

    {
        std::unique_lock<std::mutex> lock(receiver.mutex);
        ASSERT_TRUE(receiver.cv.wait_for(lock, std::chrono::seconds(5),
            [&receiver]() { return receiver.completed == 2; }));
    }
    EXPECT_THAT(receiver.cancelDuration, Lt(std::chrono::milliseconds(500)));
    ASSERT_TRUE(manager.waitForRequestsToDrain(std::chrono::seconds(5)));
    EXPECT_THAT(manager.requestCount(), 0u);
    EXPECT_TRUE(delivery.waitFor(2));
}

TEST(HttpClientManagerAsyncTests, DestructorWaitsForActiveCallback)
{
    MockIHttpClient httpClient;
    auto manager = std::make_unique<AsyncHttpClientManager4Test>(httpClient);
    BlockingAsyncCompletionReceiver receiver;
    manager->requestDone >> receiver.sink;

    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->httpRequest = new SimpleHttpRequest("async-destructor");
    ctx->httpRequestId = ctx->httpRequest->GetId();
    ctx->recordIdsAndTenantIds["r1"] = "t1";
    ctx->latency = EventLatency_Normal;
    ctx->packageIds["tenant1-token"] = 0;

    IHttpResponseCallback* callback = nullptr;
    EXPECT_CALL(httpClient, SendRequestAsync(ctx->httpRequest, _))
        .WillOnce(SaveArg<1>(&callback));
    manager->sendRequest(ctx);
    ASSERT_THAT(callback, NotNull());
    QueuedHttpResponseDelivery delivery;
    auto dispatcher = PAL::getDefaultTaskDispatcher();
    PAL::scheduleTask(
        dispatcher.get(), 0, &delivery, &QueuedHttpResponseDelivery::deliver,
        callback, new SimpleHttpResponse("async-destructor"));

    {
        std::unique_lock<std::mutex> lock(receiver.mutex);
        ASSERT_TRUE(receiver.cv.wait_for(lock, std::chrono::seconds(5),
            [&receiver]() { return receiver.entered; }));
    }

    std::atomic<bool> destructorReturned {false};
    std::thread destroyer([&manager, &destructorReturned]() {
        manager.reset();
        destructorReturned.store(true);
    });

    PAL::sleep(100);
    EXPECT_FALSE(destructorReturned.load());
    {
        std::lock_guard<std::mutex> lock(receiver.mutex);
        receiver.released = true;
    }
    receiver.cv.notify_all();
    destroyer.join();
    EXPECT_TRUE(destructorReturned.load());
    EXPECT_TRUE(delivery.waitFor(1));
}

#ifndef _WIN32
TEST(HttpClientManagerAsyncTests, DroppedResponseTaskCompletesInline)
{
    MockIHttpClient httpClient;
    DroppingHttpResponseTaskDispatcher dispatcher;
    AsyncHttpClientManager4Test manager(httpClient, dispatcher);
    HttpRequestDoneReceiver receiver;
    manager.requestDone >> receiver.sink;

    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->httpRequest = new SimpleHttpRequest("dropped-response-task");
    ctx->httpRequestId = ctx->httpRequest->GetId();
    IHttpResponseCallback* callback = nullptr;
    EXPECT_CALL(httpClient, SendRequestAsync(ctx->httpRequest, _))
        .WillOnce(SaveArg<1>(&callback));
    manager.sendRequest(ctx);

    EXPECT_CALL(receiver, onRequestDone(ctx));
    callback->OnHttpResponse(new SimpleHttpResponse(ctx->httpRequestId));

    EXPECT_THAT(manager.requestCount(), 0u);
}

#if HAVE_EXCEPTIONS
TEST(HttpClientManagerAsyncTests, ThrowingResponseQueueCompletesInline)
{
    MockIHttpClient httpClient;
    ThrowingHttpResponseTaskDispatcher dispatcher;
    AsyncHttpClientManager4Test manager(httpClient, dispatcher);
    HttpRequestDoneReceiver receiver;
    manager.requestDone >> receiver.sink;

    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->httpRequest = new SimpleHttpRequest("throwing-response-queue");
    ctx->httpRequestId = ctx->httpRequest->GetId();
    IHttpResponseCallback* callback = nullptr;
    EXPECT_CALL(httpClient, SendRequestAsync(ctx->httpRequest, _))
        .WillOnce(SaveArg<1>(&callback));
    manager.sendRequest(ctx);

    EXPECT_CALL(receiver, onRequestDone(ctx));
    callback->OnHttpResponse(new SimpleHttpResponse(ctx->httpRequestId));

    EXPECT_THAT(manager.requestCount(), 0u);
}
#endif
#endif

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
    // rather than block forever. MockIHttpClient does not implement the bounded
    // cancel capability, so HttpClientManager falls back to per-request async cancel
    // and then abandons the drain when the callback remains outstanding.
    EXPECT_CALL(httpClientMock, CancelRequestAsync(ctx->httpRequestId)).WillOnce(Return());
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

TEST_F(HttpClientManagerTests, CancelAllRequests_UsesBoundedCancelCapability)
{
    MockBoundedIHttpClient boundedClient;
    HttpClientManager4Test boundedHcm(boundedClient);
    boundedHcm.setCancelDrainTimeout(std::chrono::milliseconds(150));
    boundedHcm.requestDone >> requestDone;

    SimpleHttpRequest* req = new SimpleHttpRequest("bounded");
    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->httpRequestId = req->GetId();
    ctx->httpRequest = req;
    ctx->recordIdsAndTenantIds["r1"] = "t1";
    ctx->latency = EventLatency_Normal;
    ctx->packageIds["tenant1-token"] = 0;

    IHttpResponseCallback* callback = nullptr;
    EXPECT_CALL(boundedClient, SendRequestAsync(ctx->httpRequest, _))
        .WillOnce(SaveArg<1>(&callback));
    boundedHcm.sendRequest(ctx);
    ASSERT_THAT(callback, NotNull());

    EXPECT_CALL(boundedClient, CancelAllRequests(std::chrono::milliseconds(150))).WillOnce(Return());
    EXPECT_CALL(boundedClient, CancelRequestAsync(_)).Times(0);

    boundedHcm.cancelAllRequests(/* bestEffort */ true);

    EXPECT_CALL(*this, resultRequestDone(ctx)).WillOnce(Return());
    callback->OnHttpResponse(new SimpleHttpResponse("bounded"));
}

TEST_F(HttpClientManagerTests, ZeroBudgetPauseCancelsWithoutWaiting)
{
    hcm.setCancelDrainTimeout(std::chrono::milliseconds::zero());

    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->httpRequest = new SimpleHttpRequest("zero-budget");
    ctx->httpRequestId = ctx->httpRequest->GetId();
    ctx->recordIdsAndTenantIds["r1"] = "t1";
    ctx->latency = EventLatency_Normal;
    ctx->packageIds["tenant1-token"] = 0;

    IHttpResponseCallback* callback = nullptr;
    EXPECT_CALL(httpClientMock, SendRequestAsync(ctx->httpRequest, _))
        .WillOnce(SaveArg<1>(&callback));
    hcm.sendRequest(ctx);
    ASSERT_THAT(callback, NotNull());

    EXPECT_CALL(httpClientMock, CancelRequestAsync(ctx->httpRequestId));
    EXPECT_NO_THROW(hcm.cancelAllRequests(/* bestEffort */ true));
    EXPECT_THAT(hcm.requestCount(), 1u);

    EXPECT_CALL(*this, resultRequestDone(ctx)).WillOnce(Return());
    callback->OnHttpResponse(new SimpleHttpResponse("zero-budget"));
    EXPECT_THAT(hcm.requestCount(), 0u);
}

TEST_F(HttpClientManagerTests, ZeroBudgetPauseContinuesAfterCancelThrows)
{
    hcm.setCancelDrainTimeout(std::chrono::milliseconds::zero());

    std::vector<IHttpResponseCallback*> callbacks;
    std::vector<EventsUploadContextPtr> contexts;
    for (const char* id : {"cancel-throws", "cancel-continues"})
    {
        auto ctx = std::make_shared<EventsUploadContext>();
        ctx->httpRequest = new SimpleHttpRequest(id);
        ctx->httpRequestId = id;
        ctx->recordIdsAndTenantIds["r1"] = "t1";
        ctx->latency = EventLatency_Normal;
        ctx->packageIds["tenant1-token"] = 0;

        IHttpResponseCallback* callback = nullptr;
        EXPECT_CALL(httpClientMock, SendRequestAsync(ctx->httpRequest, _))
            .WillOnce(SaveArg<1>(&callback));
        hcm.sendRequest(ctx);
        ASSERT_THAT(callback, NotNull());
        callbacks.push_back(callback);
        contexts.push_back(std::move(ctx));
    }

    {
        InSequence sequence;
        EXPECT_CALL(httpClientMock, CancelRequestAsync("cancel-throws"))
            .WillOnce(Throw(std::runtime_error("cancel failed")));
        EXPECT_CALL(httpClientMock, CancelRequestAsync("cancel-continues"));
    }
    EXPECT_NO_THROW(hcm.cancelAllRequests(/* bestEffort */ true));

    for (size_t i = 0; i < callbacks.size(); ++i)
    {
        EXPECT_CALL(*this, resultRequestDone(contexts[i])).WillOnce(Return());
        callbacks[i]->OnHttpResponse(
            new SimpleHttpResponse(contexts[i]->httpRequestId));
    }
    EXPECT_THAT(hcm.requestCount(), 0u);
}

TEST(HttpClientManagerExceptionTests, FullCancellationContainsClientException)
{
    ThrowingCancelAllHttpClient httpClient;
    HttpClientManager4Test manager(httpClient);

    EXPECT_NO_THROW(manager.cancelAllRequests());
}

TEST(HttpClientManagerExceptionTests, BoundedCancellationFallsBackAfterException)
{
    MockBoundedIHttpClient httpClient;
    HttpClientManager4Test manager(httpClient);
    manager.setCancelDrainTimeout(std::chrono::milliseconds(50));

    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->httpRequest = new SimpleHttpRequest("bounded-throws");
    ctx->httpRequestId = ctx->httpRequest->GetId();
    ctx->recordIdsAndTenantIds["r1"] = "t1";
    ctx->latency = EventLatency_Normal;
    ctx->packageIds["tenant1-token"] = 0;

    IHttpResponseCallback* callback = nullptr;
    EXPECT_CALL(httpClient, SendRequestAsync(ctx->httpRequest, _))
        .WillOnce(SaveArg<1>(&callback));
    manager.sendRequest(ctx);
    ASSERT_THAT(callback, NotNull());

    EXPECT_CALL(httpClient, CancelAllRequests(std::chrono::milliseconds(50)))
        .WillOnce(Throw(std::runtime_error("bounded cancel failed")));
    EXPECT_CALL(httpClient, CancelRequestAsync(ctx->httpRequestId));
    manager.cancelAllRequests(/* bestEffort */ true);

    callback->OnHttpResponse(new SimpleHttpResponse("bounded-throws"));
}
