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
#include <atomic>
#include <chrono>
#include <exception>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <utility>
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
        struct CompletionState
        {
            explicit CompletionState(std::string id)
                : requestId(std::move(id))
            {
            }

            bool TryStartTerminal() noexcept
            {
                bool expected = false;
                return terminalStarted.compare_exchange_strong(expected, true);
            }

            std::atomic<bool> terminalStarted{false};
            std::string const requestId;
        };

        HttpCallback(HttpClientManager& hcm, EventsUploadContextPtr const& ctx)
            : m_hcm(hcm),
            m_ctx(ctx),
            m_startTime(PAL::getMonotonicTimeMs()),
            m_completion(std::make_shared<CompletionState>(
                !ctx->httpRequestId.empty()
                    ? ctx->httpRequestId
                    : (ctx->httpRequest != nullptr
                        ? ctx->httpRequest->GetId()
                        : std::string())))
        {
        }

        virtual void OnHttpResponse(IHttpResponse* response) override
        {
            std::unique_ptr<IHttpResponse> ownedResponse(response);
            if (!m_completion->TryStartTerminal())
            {
                LOG_ERROR("Ignoring duplicate terminal HTTP callback for request %s",
                    m_completion->requestId.c_str());
                return;
            }
            CompleteClaimed(ownedResponse.release());
        }

        void CompleteClaimed(IHttpResponse* response)
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
        std::shared_ptr<CompletionState> m_completion;
    };

    //---

    HttpClientManager::HttpClientManager(ILogManager& logManager, IHttpClient& httpClient, ITaskDispatcher& taskDispatcher) :
        m_logManager(logManager),
        m_httpClient(httpClient),
        m_taskDispatcher(taskDispatcher)
    {
        int64_t configuredSeconds =
            logManager.GetLogConfiguration()[CFG_INT_MAX_TEARDOWN_TIME];
        if (configuredSeconds > 0)
        {
            int64_t const maxSeconds =
                std::chrono::milliseconds::max().count() / 1000;
            m_cancelDrainTimeout = std::chrono::seconds(
                std::min(configuredSeconds, maxSeconds));
        }
    }

    HttpClientManager::~HttpClientManager() noexcept
    {
        // HttpCallback and scheduled response tasks retain a reference to this
        // manager, so non-reentrant destruction must be a full callback lifetime
        // barrier. Reentrant destruction is unsupported because the active
        // callback itself must still unwind through this object.
#ifndef NDEBUG
        {
            std::lock_guard<std::mutex> lock(m_httpCallbacksMtx);
            for (auto const& active : m_activeHttpCallbacks)
            {
                assert(active.second != std::this_thread::get_id());
            }
        }
#endif
        cancelAllRequests();
    }

    void HttpClientManager::handleSendRequest(EventsUploadContextPtr const& ctx)
    {
        HttpCallback *callback = new HttpCallback(*this, ctx);
        auto completion = callback->m_completion;
        {
            LOCKGUARD(m_httpCallbacksMtx);
            m_httpCallbacks.push_back(callback);
        }

        LOG_INFO("Uploading %u event(s) of priority %d (%s) for %u tenant(s) in HTTP request %s (approx. %u bytes)...",
            static_cast<unsigned>(ctx->recordIdsAndTenantIds.size()), ctx->latency, latencyToStr(ctx->latency), static_cast<unsigned>(ctx->packageIds.size()),
            ctx->httpRequest->GetId().c_str(), static_cast<unsigned>(ctx->httpRequest->GetSizeEstimate()));

#if HAVE_EXCEPTIONS
        try
        {
            m_httpClient.SendRequestAsync(ctx->httpRequest, callback);
        }
        catch (const std::exception& ex)
        {
            LOG_ERROR("HTTP client rejected request %s with an exception: %s",
                completion->requestId.c_str(), ex.what());
            if (completion->TryStartTerminal())
            {
                callback->CompleteClaimed(
                    new SimpleHttpResponse(completion->requestId));
            }
        }
        catch (...)
        {
            LOG_ERROR("HTTP client rejected request %s with a non-standard exception",
                completion->requestId.c_str());
            if (completion->TryStartTerminal())
            {
                callback->CompleteClaimed(
                    new SimpleHttpResponse(completion->requestId));
            }
        }
#else
        m_httpClient.SendRequestAsync(ctx->httpRequest, callback);
#endif
    }

    void HttpClientManager::scheduleOnHttpResponse(HttpCallback* callback)
    {
        PAL::scheduleTask(&m_taskDispatcher, 0, this, &HttpClientManager::onHttpResponse, callback);
    }

    /* This method may get executed synchronously on Windows from handleSendRequest in case of connection failure */
    void HttpClientManager::onHttpResponse(HttpCallback* callback)
    {
        {
            std::lock_guard<std::mutex> lock(m_httpCallbacksMtx);
            auto z = std::find(m_httpCallbacks.cbegin(), m_httpCallbacks.cend(), callback);
            if (z == m_httpCallbacks.end()) {
                LOG_ERROR("Ignoring untracked HTTP callback=%p", callback);
                return;
            }
            m_activeHttpCallbacks[callback] = std::this_thread::get_id();
            m_httpCallbacksCV.notify_all();
        }

        EventsUploadContextPtr &ctx = callback->m_ctx;

#if !defined(NDEBUG) && defined(HAVE_MAT_LOGGING)
        // Response may be null if request got aborted
        if (ctx->httpResponse != nullptr)
        {
            IHttpResponse const& response = (*ctx->httpResponse);
            LOG_TRACE("HTTP response %s: result=%u, status=%u, body=%u bytes",
                response.GetId().c_str(), response.GetResult(), response.GetStatusCode(), static_cast<unsigned>(response.GetBody().size()));
        }
#endif

        // Never hold m_httpCallbacksMtx while calling the transport or
        // dispatching requestDone(): either path may synchronously re-enter this
        // manager. Reentrant cancellation recognizes this callback as active
        // and does not wait for its own stack to unwind.
#if HAVE_EXCEPTIONS
        try
        {
            requestDone(ctx);
        }
        catch (const std::exception& ex)
        {
            LOG_ERROR("Unhandled exception in HTTP response callback: %s", ex.what());
            notifyRequestFailure(ctx);
        }
        catch (...)
        {
            LOG_ERROR("Unhandled non-standard exception in HTTP response callback");
            notifyRequestFailure(ctx);
        }
#else
        requestDone(ctx);
#endif
        // request done should be handled by now

        {
            std::lock_guard<std::mutex> lock(m_httpCallbacksMtx);
            LOG_TRACE("HTTP remove callback=%p", callback);
            m_httpCallbacks.remove(callback);
            m_activeHttpCallbacks.erase(callback);
            // Wake cancelAllRequests() waiting for the list to drain while the
            // condition variable is still guaranteed to be alive.
            m_httpCallbacksCV.notify_all();
        }

        delete callback;
    }

    void HttpClientManager::notifyRequestFailure(EventsUploadContextPtr const& ctx) noexcept
    {
#if HAVE_EXCEPTIONS
        try
        {
            requestFailed(ctx);
        }
        catch (const std::exception& ex)
        {
            LOG_ERROR("Unhandled exception while releasing failed HTTP request: %s", ex.what());
        }
        catch (...)
        {
            LOG_ERROR("Unhandled non-standard exception while releasing failed HTTP request");
        }

        try
        {
            requestFailureComplete(ctx);
        }
        catch (const std::exception& ex)
        {
            LOG_ERROR("Unhandled exception while completing failed HTTP request: %s", ex.what());
        }
        catch (...)
        {
            LOG_ERROR("Unhandled non-standard exception while completing failed HTTP request");
        }
#else
        requestFailed(ctx);
        requestFailureComplete(ctx);
#endif
    }

    void HttpClientManager::cancelAllRequestsAsync(std::chrono::milliseconds bestEffortTimeout)
    {
        if (bestEffortTimeout > std::chrono::milliseconds::zero())
        {
#if defined(_CPPRTTI) || defined(__GXX_RTTI)
            auto boundedCancel = dynamic_cast<IBoundedHttpClientCancel*>(&m_httpClient);
            if (boundedCancel != nullptr)
            {
#if HAVE_EXCEPTIONS
                try
                {
                    boundedCancel->CancelAllRequests(bestEffortTimeout);
                    return;
                }
                catch (const std::exception& ex)
                {
                    LOG_ERROR("HTTP client bounded cancellation failed: %s", ex.what());
                }
                catch (...)
                {
                    LOG_ERROR("HTTP client bounded cancellation failed with a non-standard exception");
                }
#else
                boundedCancel->CancelAllRequests(bestEffortTimeout);
                return;
#endif
            }
#endif

            cancelTrackedRequestsAsync();
            return;
        }

#if HAVE_EXCEPTIONS
        try
        {
            m_httpClient.CancelAllRequests();
        }
        catch (const std::exception& ex)
        {
            LOG_ERROR("HTTP client cancellation failed: %s", ex.what());
            cancelTrackedRequestsAsync();
        }
        catch (...)
        {
            LOG_ERROR("HTTP client cancellation failed with a non-standard exception");
            cancelTrackedRequestsAsync();
        }
#else
        m_httpClient.CancelAllRequests();
#endif
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
#if HAVE_EXCEPTIONS
            try
            {
                m_httpClient.CancelRequestAsync(id);
            }
            catch (const std::exception& ex)
            {
                LOG_ERROR("HTTP client failed to cancel request %s: %s",
                    id.c_str(), ex.what());
            }
            catch (...)
            {
                LOG_ERROR("HTTP client failed to cancel request %s with a non-standard exception",
                    id.c_str());
            }
#else
            m_httpClient.CancelRequestAsync(id);
#endif
        }
    }

    void HttpClientManager::cancelAllRequests(bool bestEffort)
    {
        if (bestEffort &&
            m_cancelDrainTimeout <= std::chrono::milliseconds::zero())
        {
            // A zero budget means "do not wait", not "leave requests running".
            // Snapshot IDs and initiate asynchronous cancellation before returning.
            cancelTrackedRequestsAsync();
            return;
        }
        // Quiesce the transport before taking m_httpCallbacksMtx. Moving this
        // call under the mutex deadlocks when a synchronous transport completion
        // re-enters onHttpResponse().
        const auto cancelStart = std::chrono::steady_clock::now();
        cancelAllRequestsAsync(bestEffort ? m_cancelDrainTimeout : std::chrono::milliseconds::zero());

        // Drain callbacks through the condition variable signaled by onHttpResponse.
        std::unique_lock<std::mutex> lock(m_httpCallbacksMtx);
        std::thread::id const callerThread = std::this_thread::get_id();
        auto callbacksDrainedForCaller = [this, callerThread] {
            for (auto const& active : m_activeHttpCallbacks)
            {
                if (active.second == callerThread)
                {
                    // A completion running on a single-thread dispatcher cannot
                    // wait for peer completions queued behind itself. Returning
                    // from reentrant cancellation lets this callback unwind and
                    // the dispatcher drain the remaining work.
                    return true;
                }
            }
            return m_httpCallbacks.empty();
        };
        if (bestEffort)
        {
            // Keep pause within the configured soft cap, including time spent
            // in transport cancellation. A synchronous native handle close
            // already in progress can finish after the deadline.
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - cancelStart);
            const auto remaining = (elapsed < m_cancelDrainTimeout)
                ? (m_cancelDrainTimeout - elapsed) : std::chrono::milliseconds::zero();
            if (!m_httpCallbacksCV.wait_for(
                    lock, remaining, callbacksDrainedForCaller))
            {
                LOG_WARN("cancelAllRequests: %zu callback(s) still draining after %lld ms (best-effort)",
                         m_httpCallbacks.size(), static_cast<long long>(m_cancelDrainTimeout.count()));
            }
        }
        else
        {
            // Non-reentrant shutdown/cleanup is the lifetime barrier for callback
            // state. A callback re-entering cancellation must return so its own
            // stack can unwind; destroying the manager from that stack is unsupported.
            m_httpCallbacksCV.wait(lock, callbacksDrainedForCaller);
        }
    }

    // start async cancellation

} MAT_NS_END
