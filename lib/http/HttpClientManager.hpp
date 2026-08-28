//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include "IHttpClient.hpp"
#include "pal/PAL.hpp"
#include "system/Contexts.hpp"
#include "system/Route.hpp"
#include "ILogManager.hpp"

#include <list>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <map>
#include <thread>

namespace MAT_NS_BEGIN
{

class HttpClientManager
{

    public:

        HttpClientManager(
                ILogManager& logManager,
                IHttpClient& httpClient,
                ITaskDispatcher& taskDispatcher);

        virtual ~HttpClientManager() noexcept;

        // Cancel in-flight requests. Shutdown drains fully; pause uses a bounded,
        // best-effort drain because it may run under the LogManager lock.
        void cancelAllRequests(bool bestEffort = false);

        size_t requestCount() const
        {
            // Access to m_httpCallbacks must be serialized via m_httpCallbacksMtx.
            // Without the lock this is a std::list data race vs onHttpResponse,
            // handleSendRequest, and cancelAllRequests (same UB class as the
            // empty()-check bug fixed in cancelAllRequests). The mutex is
            // declared mutable below so a const observer can take it.
            LOCKGUARD(m_httpCallbacksMtx);
            return m_httpCallbacks.size();
        }

        RouteSource<EventsUploadContextPtr const&> requestDone;
        RouteSource<EventsUploadContextPtr const&> requestFailed;
        RouteSource<EventsUploadContextPtr const&> requestFailureComplete;

        RouteSink<HttpClientManager, EventsUploadContextPtr const&> sendRequest
        {
            this, &HttpClientManager::handleSendRequest
        };

    protected:
        class HttpCallback;

        void handleSendRequest(EventsUploadContextPtr const& ctx);
        virtual void scheduleOnHttpResponse(HttpCallback* callback);
        void onHttpResponse(HttpCallback* callback);
        void notifyRequestFailure(EventsUploadContextPtr const& ctx) noexcept;
        void cancelAllRequestsAsync(std::chrono::milliseconds bestEffortTimeout = std::chrono::milliseconds::zero());
        void cancelTrackedRequestsAsync();

        ILogManager&              m_logManager;
        IHttpClient&              m_httpClient;
        ITaskDispatcher&          m_taskDispatcher;
        mutable std::mutex           m_httpCallbacksMtx;
        std::list<HttpCallback*>  m_httpCallbacks;
        std::map<HttpCallback*, std::thread::id> m_activeHttpCallbacks;
        // Signaled from onHttpResponse when a callback is removed, so cancelAllRequests
        // can drain via a condition variable instead of a poll loop.
        std::condition_variable      m_httpCallbacksCV;
        // Configured soft cap on the best-effort pause drain. One native handle
        // close already in progress may finish after it. Non-reentrant full
        // shutdown remains a lifetime barrier and waits for every accepted
        // request's terminal callback.
        std::chrono::milliseconds m_cancelDrainTimeout{std::chrono::milliseconds::zero()};
};

} MAT_NS_END
