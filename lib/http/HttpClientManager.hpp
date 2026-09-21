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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <map>
#include <memory>
#include <mutex>
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
            auto registry = m_callbackRegistry;
            LOCKGUARD(registry->mutex);
            return registry->callbacks.size();
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
        struct CallbackRegistry
        {
            mutable std::mutex mutex;
            std::list<HttpCallback*> callbacks;
            std::map<HttpCallback*, std::thread::id> activeCallbacks;
            std::condition_variable drained;
        };

        void handleSendRequest(EventsUploadContextPtr const& ctx);
        virtual void scheduleOnHttpResponse(HttpCallback* callback);
        void runScheduledHttpResponse(
            std::shared_ptr<std::atomic<bool>> const& started,
            HttpCallback* callback);
        void onHttpResponse(HttpCallback* callback);
        void notifyRequestFailure(EventsUploadContextPtr const& ctx) noexcept;
        void cancelAllRequestsAsync(std::chrono::milliseconds bestEffortTimeout = std::chrono::milliseconds::zero());
        void cancelTrackedRequestsAsync();

        ILogManager&              m_logManager;
        IHttpClient&              m_httpClient;
        ITaskDispatcher&          m_taskDispatcher;
        std::shared_ptr<CallbackRegistry> m_callbackRegistry {
            std::make_shared<CallbackRegistry>()};
        // Configured soft cap on the best-effort pause drain. One native handle
        // close already in progress may finish after it. Non-reentrant full
        // shutdown remains a lifetime barrier and waits for every accepted
        // request's terminal callback.
        std::chrono::milliseconds m_cancelDrainTimeout{std::chrono::milliseconds::zero()};
};

} MAT_NS_END
