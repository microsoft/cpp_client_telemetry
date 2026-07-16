//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "HttpClientManager.hpp"
#include "IBoundedHttpClientCancel.hpp"
#include "utils/StringUtils.hpp"
#include "pal/TaskDispatcher.hpp"

#include <assert.h>
#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>

#ifdef linux
#include <unistd.h>
#ifdef _POSIX_PRIORITY_SCHEDULING
#include <sched.h>
#else
#define sched_yield()
#endif
#endif

#ifdef _WIN32
#define USE_SYNC_HTTPRESPONSE_HANDLER
#else
// Linux and Mac OS X with libcurl require an async handler for now
#endif

namespace MAT_NS_BEGIN {


    class HttpClientManager::HttpCallback : public IHttpResponseCallback
    {
    public:

        HttpCallback(HttpClientManager& hcm, EventsUploadContextPtr const& ctx)
            : m_hcm(hcm),
            m_ctx(ctx),
            m_startTime(PAL::getMonotonicTimeMs())
        {
        }

        virtual void OnHttpResponse(IHttpResponse* response) override
        {
            m_ctx->durationMs = static_cast<int>(PAL::getMonotonicTimeMs() - m_startTime);
            m_ctx->httpResponse = response;
#ifdef USE_SYNC_HTTPRESPONSE_HANDLER // handle HTTP callback synchronously in context of a callback thread
            // We need to decide on pros and cons of synchronous vs. asynchronous callback
            m_hcm.onHttpResponse(this);
#else
            // Handle HTTP response asynchronously
            m_hcm.scheduleOnHttpResponse(this);
#endif
        }

        virtual void OnHttpStateEvent(HttpStateEvent state, void *data = nullptr, size_t size = 0) override
        {
            // TODO: [MG] - do we need to consider adding a return value? For example,
            // if we implement a state called OnSslVerify in future, passing down some
            // implementation-dependent struct via data ptr, then the callback can
            // indicate either success or failure.. But alternatively the callback might
            // as well pass the data back by updating the data structure.
            DebugEvent evt(EVT_HTTP_STATE, size_t(state), 0, data, size);
            m_hcm.m_logManager.DispatchEvent(evt);
        }


        virtual ~HttpCallback()
        {
            LOG_TRACE("destroy HTTP callback=%p ctx=%p", this, m_ctx.get());
        }

    public:
        HttpClientManager&      m_hcm;
        EventsUploadContextPtr  m_ctx;
        int64_t                 m_startTime;
    };

    //---

    HttpClientManager::HttpClientManager(ILogManager& logManager, IHttpClient& httpClient, ITaskDispatcher& taskDispatcher) :
        m_logManager(logManager),
        m_httpClient(httpClient),
        m_taskDispatcher(taskDispatcher)
    {
    }

    HttpClientManager::~HttpClientManager() noexcept
    {
        cancelAllRequestsAsync();
    }

    void HttpClientManager::handleSendRequest(EventsUploadContextPtr const& ctx)
    {
        HttpCallback *callback = new HttpCallback(*this, ctx);
        {
            LOCKGUARD(m_httpCallbacksMtx);
            m_httpCallbacks.push_back(callback);
        }

        LOG_INFO("Uploading %u event(s) of priority %d (%s) for %u tenant(s) in HTTP request %s (approx. %u bytes)...",
            static_cast<unsigned>(ctx->recordIdsAndTenantIds.size()), ctx->latency, latencyToStr(ctx->latency), static_cast<unsigned>(ctx->packageIds.size()),
            ctx->httpRequest->GetId().c_str(), static_cast<unsigned>(ctx->httpRequest->GetSizeEstimate()));

        m_httpClient.SendRequestAsync(ctx->httpRequest, callback);
    }

    void HttpClientManager::scheduleOnHttpResponse(HttpCallback* callback)
    {
        PAL::scheduleTask(&m_taskDispatcher, 0, this, &HttpClientManager::onHttpResponse, callback);
    }

    /* This method may get executed synchronously on Windows from handleSendRequest in case of connection failure */
    void HttpClientManager::onHttpResponse(HttpCallback* callback)
    {
        EventsUploadContextPtr &ctx = callback->m_ctx;
        {
            LOCKGUARD(m_httpCallbacksMtx);
            auto z = std::find(m_httpCallbacks.cbegin(), m_httpCallbacks.cend(), callback);
            if (z == m_httpCallbacks.end()) {
                assert(false);
            }

#if !defined(NDEBUG) && defined(HAVE_MAT_LOGGING)
            // Response may be null if request got aborted
            if (ctx->httpResponse != nullptr)
            {
                IHttpResponse const& response = (*ctx->httpResponse);
                LOG_TRACE("HTTP response %s: result=%u, status=%u, body=%u bytes",
                    response.GetId().c_str(), response.GetResult(), response.GetStatusCode(), static_cast<unsigned>(response.GetBody().size()));
            }
#endif

            requestDone(ctx);
            // request done should be handled by now

            LOG_TRACE("HTTP remove callback=%p", callback);
            m_httpCallbacks.remove(callback);
            // Wake cancelAllRequests() waiting for the list to drain.
            m_httpCallbacksCV.notify_all();
        }

        delete callback;
    }

    bool HttpClientManager::cancelAllRequestsAsync(std::chrono::milliseconds bestEffortTimeout)
    {
        if (bestEffortTimeout > std::chrono::milliseconds::zero())
        {
            auto boundedCancel = dynamic_cast<IBoundedHttpClientCancel*>(&m_httpClient);
            if (boundedCancel != nullptr)
            {
                boundedCancel->CancelAllRequests(bestEffortTimeout);
                return true;
            }

            cancelTrackedRequestsAsync();
            return false;
        }

        m_httpClient.CancelAllRequests();
        return true;
    }

    void HttpClientManager::cancelTrackedRequestsAsync()
    {
        std::vector<std::string> requestIds;
        {
            LOCKGUARD(m_httpCallbacksMtx);
            for (const auto& callback : m_httpCallbacks)
            {
                if (callback == nullptr || callback->m_ctx == nullptr)
                {
                    continue;
                }

                std::string id = callback->m_ctx->httpRequestId;
                if (id.empty() && callback->m_ctx->httpRequest != nullptr)
                {
                    id = callback->m_ctx->httpRequest->GetId();
                }
                if (!id.empty())
                {
                    requestIds.push_back(id);
                }
            }
        }

        for (const auto& id : requestIds)
        {
            m_httpClient.CancelRequestAsync(id);
        }
    }

    void HttpClientManager::cancelAllRequests(bool bestEffort)
    {
        // On clients that opt into bounded cancellation (notably synchronous-response
        // Windows transports), the transport's CancelAllRequests() is where
        // cancellation actually blocks, so pass the best-effort deadline down to
        // bound it. Older clients fall back to per-request async cancel; the
        // manager-level drain below is the bound for those async-handler paths. A
        // zero timeout means "drain fully".
        const auto cancelStart = std::chrono::steady_clock::now();
        cancelAllRequestsAsync(bestEffort ? m_cancelDrainTimeout : std::chrono::milliseconds::zero());

        // Drain in-flight callbacks via a condition variable signaled from
        // onHttpResponse -- never a busy/poll loop, which burned 100% CPU while
        // holding the LogManager lock.
        std::unique_lock<std::recursive_mutex> lock(m_httpCallbacksMtx);
        if (bestEffort)
        {
            // Pause (and similar) must not block the caller indefinitely -- it may
            // hold the LogManager lock (the observed spindump was PauseTransmission).
            // The manager is NOT being destroyed here, so outstanding callbacks remain
            // valid and drain later; cap the wait as a safety valve.
            //
            // This bounds the drain of m_httpCallbacks, which is the blocking region on
            // the async-handler and old-style-client fallback paths. On Windows
            // (USE_SYNC_HTTPRESPONSE_HANDLER), bounded-capability clients drain
            // m_httpCallbacks synchronously inside the transport cancel call above, so
            // this wait is usually already satisfied; the real blocking there is the
            // transport-level wait (WinInet condition-variable wait, WinRt poll), which
            // is bounded by the same best-effort deadline passed into the capability.
            //
            // Treat m_cancelDrainTimeout as the total budget for the pause: subtract the
            // time already spent in the transport cancel so the whole path holds the
            // LogManager lock for at most ~m_cancelDrainTimeout, not up to 2x it.
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - cancelStart);
            const auto remaining = (elapsed < m_cancelDrainTimeout)
                ? (m_cancelDrainTimeout - elapsed) : std::chrono::milliseconds::zero();
            if (!m_httpCallbacksCV.wait_for(lock, remaining,
                    [this] { return m_httpCallbacks.empty(); }))
            {
                LOG_WARN("cancelAllRequests: %zu callback(s) still draining after %lld ms (best-effort)",
                         m_httpCallbacks.size(), static_cast<long long>(m_cancelDrainTimeout.count()));
            }
        }
        else
        {
            // Shutdown/cleanup: this is the lifetime barrier before state the
            // callbacks reference is destroyed, so drain fully. The CV keeps it
            // efficient (no CPU spin) and it returns as soon as the last callback is
            // handled -- a stalled drain here indicates the caller stopped the task
            // dispatcher before cancelling, which it must not do.
            m_httpCallbacksCV.wait(lock, [this] { return m_httpCallbacks.empty(); });
        }
    }

    // start async cancellation

} MAT_NS_END
