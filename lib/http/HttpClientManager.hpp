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

        // Cancel in-flight requests and drain their callbacks. bestEffort=false (the
        // default, used by shutdown/cleanup) drains fully -- it is the lifetime
        // barrier before state the callbacks reference is destroyed. bestEffort=true
        // (used by pause) caps the wait so it cannot block a caller that holds the
        // LogManager lock; the manager is not being destroyed, so outstanding
        // callbacks stay valid and drain later.
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

        RouteSink<HttpClientManager, EventsUploadContextPtr const&> sendRequest
        {
            this, &HttpClientManager::handleSendRequest
        };

    protected:
        class HttpCallback;
        friend class HttpCallback;

        void handleSendRequest(EventsUploadContextPtr const& ctx);
        virtual void scheduleOnHttpResponse(HttpCallback* callback);
        void onHttpResponse(HttpCallback* callback);
        bool cancelAllRequestsAsync(std::chrono::milliseconds bestEffortTimeout = std::chrono::milliseconds::zero());

        ILogManager&              m_logManager;
        IHttpClient&              m_httpClient;
        ITaskDispatcher&          m_taskDispatcher;
        mutable std::recursive_mutex m_httpCallbacksMtx;
        std::list<HttpCallback*>  m_httpCallbacks;
        // Signaled from onHttpResponse when a callback is removed, so cancelAllRequests
        // can drain via a condition variable instead of a poll loop.
        std::condition_variable_any m_httpCallbacksCV;
        // Upper bound on how long cancelAllRequests waits for callbacks to drain. A
        // last-resort safety valve so a stalled dispatcher/HTTP stack can never make
        // the drain spin or block forever. Adjustable so tests can
        // exercise the timeout path without a long wait.
        std::chrono::milliseconds m_cancelDrainTimeout{std::chrono::seconds(30)};
};

} MAT_NS_END

