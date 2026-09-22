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
        class ManagerUse
        {
        public:
            ManagerUse(
                std::shared_ptr<CallbackRegistry> registry,
                HttpCallback* callback)
                : m_registry(std::move(registry))
            {
                std::lock_guard<std::mutex> lock(m_registry->mutex);
                auto found = std::find(
                    m_registry->callbacks.cbegin(),
                    m_registry->callbacks.cend(),
                    callback);
                if (found != m_registry->callbacks.end() &&
                    m_registry->manager != nullptr)
                {
                    m_manager = m_registry->manager;
                    ++m_registry->activeCalls[std::this_thread::get_id()];
                }
            }

            ~ManagerUse()
            {
                if (m_manager == nullptr)
                {
                    return;
                }
                std::lock_guard<std::mutex> lock(m_registry->mutex);
                auto active = m_registry->activeCalls.find(
                    std::this_thread::get_id());
                assert(active != m_registry->activeCalls.end());
                if (active != m_registry->activeCalls.end() &&
                    --active->second == 0)
                {
                    m_registry->activeCalls.erase(active);
                }
                m_registry->drained.notify_all();
            }

            HttpClientManager* Get() const noexcept
            {
                return m_manager;
            }

            bool IsAttached() const
            {
                std::lock_guard<std::mutex> lock(m_registry->mutex);
                return m_registry->manager == m_manager;
            }

        private:
            std::shared_ptr<CallbackRegistry> m_registry;
            HttpClientManager* m_manager {nullptr};
        };

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

        HttpCallback(
            EventsUploadContextPtr const& ctx,
            std::shared_ptr<CallbackRegistry> registry)
            : m_ctx(ctx),
            m_startTime(PAL::getMonotonicTimeMs()),
            m_completion(std::make_shared<CompletionState>(
                !ctx->httpRequestId.empty()
                    ? ctx->httpRequestId
                    : (ctx->httpRequest != nullptr
                        ? ctx->httpRequest->GetId()
                        : std::string()))),
            m_registry(std::move(registry))
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
            ProcessResponse(this);
#else
            // Handle HTTP response asynchronously
            ScheduleResponse();
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
            ManagerUse managerUse(m_registry, this);
            HttpClientManager* manager = managerUse.Get();
            if (manager != nullptr)
            {
                manager->m_logManager.DispatchEvent(evt);
            }
        }

        void ScheduleResponse()
        {
            auto started = std::make_shared<std::atomic<bool>>(false);
            auto registry = m_registry;
#if HAVE_EXCEPTIONS
            try
            {
#endif
                auto task = PAL::scheduleTask(
                    registry->taskDispatcher, 0,
                    [started, registry, callback = this]()
                    {
                        started->store(true, std::memory_order_release);
                        ProcessResponse(callback);
                    });
                if (task.GetTask() != nullptr ||
                    started->load(std::memory_order_acquire))
                {
                    return;
                }
#if HAVE_EXCEPTIONS
            }
            catch (const std::exception& ex)
            {
                (void)ex;
                LOG_ERROR("Failed to schedule HTTP response callback: %s", ex.what());
                if (started->load(std::memory_order_acquire))
                {
                    return;
                }
            }
            catch (...)
            {
                LOG_ERROR("Failed to schedule HTTP response callback with a non-standard exception");
                if (started->load(std::memory_order_acquire))
                {
                    return;
                }
            }
#endif
            // Some supported dispatchers synchronously destroy tasks they cannot
            // accept. Complete inline so the claimed callback cannot remain tracked.
            ProcessResponse(this);
        }

        static void ProcessResponse(HttpCallback* callback)
        {
            auto registry = callback->m_registry;
            ManagerUse managerUse(registry, callback);
            HttpClientManager* manager = managerUse.Get();
            if (manager == nullptr)
            {
                RemoveAndDelete(callback, registry);
                return;
            }

            EventsUploadContextPtr ctx = callback->m_ctx;

#if !defined(NDEBUG) && defined(HAVE_MAT_LOGGING)
            if (ctx->httpResponse != nullptr)
            {
                IHttpResponse const& response = (*ctx->httpResponse);
                LOG_TRACE("HTTP response %s: result=%u, status=%u, body=%u bytes",
                    response.GetId().c_str(), response.GetResult(), response.GetStatusCode(), static_cast<unsigned>(response.GetBody().size()));
            }
#endif

#if HAVE_EXCEPTIONS
            try
            {
                manager->requestDone(ctx);
            }
            catch (const std::exception& ex)
            {
                (void)ex;
                LOG_ERROR("Unhandled exception in HTTP response callback: %s", ex.what());
                if (managerUse.IsAttached())
                {
                    NotifyRequestFailure(manager, managerUse, ctx);
                }
            }
            catch (...)
            {
                LOG_ERROR("Unhandled non-standard exception in HTTP response callback");
                if (managerUse.IsAttached())
                {
                    NotifyRequestFailure(manager, managerUse, ctx);
                }
            }
#else
            manager->requestDone(ctx);
#endif

            RemoveAndDelete(callback, registry);
        }

        static void NotifyRequestFailure(
            HttpClientManager* manager,
            ManagerUse const& managerUse,
            EventsUploadContextPtr const& ctx) noexcept
        {
#if HAVE_EXCEPTIONS
            try
            {
                manager->requestFailed(ctx);
            }
            catch (const std::exception& ex)
            {
                (void)ex;
                LOG_ERROR("Unhandled exception while releasing failed HTTP request: %s", ex.what());
            }
            catch (...)
            {
                LOG_ERROR("Unhandled non-standard exception while releasing failed HTTP request");
            }

            if (!managerUse.IsAttached())
            {
                return;
            }

            try
            {
                manager->requestFailureComplete(ctx);
            }
            catch (const std::exception& ex)
            {
                (void)ex;
                LOG_ERROR("Unhandled exception while completing failed HTTP request: %s", ex.what());
            }
            catch (...)
            {
                LOG_ERROR("Unhandled non-standard exception while completing failed HTTP request");
            }
#else
            manager->requestFailed(ctx);
            if (managerUse.IsAttached())
            {
                manager->requestFailureComplete(ctx);
            }
#endif
        }

        virtual ~HttpCallback()
        {
            LOG_TRACE("destroy HTTP callback=%p ctx=%p", this, m_ctx.get());
        }

    private:
        static void RemoveAndDelete(
            HttpCallback* callback,
            std::shared_ptr<CallbackRegistry> const& registry)
        {
            {
                std::lock_guard<std::mutex> lock(registry->mutex);
                LOG_TRACE("HTTP remove callback=%p", callback);
                registry->callbacks.remove(callback);
                registry->drained.notify_all();
            }
            delete callback;
        }

    public:
        EventsUploadContextPtr  m_ctx;
        int64_t                 m_startTime;
        std::shared_ptr<CompletionState> m_completion;
        std::shared_ptr<CallbackRegistry> m_registry;
    };

    //---

    HttpClientManager::HttpClientManager(ILogManager& logManager, IHttpClient& httpClient, ITaskDispatcher& taskDispatcher) :
        m_logManager(logManager),
        m_httpClient(httpClient),
        m_taskDispatcher(taskDispatcher)
    {
        m_callbackRegistry->manager = this;
        m_callbackRegistry->taskDispatcher = &taskDispatcher;
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
        cancelAllRequests();
        detachCallbacks();
    }

    void HttpClientManager::handleSendRequest(EventsUploadContextPtr const& ctx)
    {
        auto registry = m_callbackRegistry;
        HttpCallback *callback = new HttpCallback(ctx, registry);
        auto completion = callback->m_completion;
        {
            LOCKGUARD(registry->mutex);
            registry->callbacks.push_back(callback);
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
            (void)ex;
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
                    (void)ex;
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
            (void)ex;
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
        auto registry = m_callbackRegistry;
        {
            LOCKGUARD(registry->mutex);
            for (const auto& callback : registry->callbacks)
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
                (void)ex;
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
        // Quiesce the transport before taking the callback-registry mutex. Moving this
        // call under the mutex deadlocks when a synchronous transport completion
        // re-enters onHttpResponse().
        const auto cancelStart = std::chrono::steady_clock::now();
        cancelAllRequestsAsync(bestEffort ? m_cancelDrainTimeout : std::chrono::milliseconds::zero());

        // Drain callbacks through the condition variable signaled by onHttpResponse.
        auto registry = m_callbackRegistry;
        std::unique_lock<std::mutex> lock(registry->mutex);
        std::thread::id const callerThread = std::this_thread::get_id();
        auto callbacksDrainedForCaller = [registry, callerThread] {
            return registry->activeCalls.find(callerThread) !=
                       registry->activeCalls.end() ||
                   registry->callbacks.empty();
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
            if (!registry->drained.wait_for(
                    lock, remaining, callbacksDrainedForCaller))
            {
                LOG_WARN("cancelAllRequests: %zu callback(s) still draining after %lld ms (best-effort)",
                         registry->callbacks.size(), static_cast<long long>(m_cancelDrainTimeout.count()));
            }
        }
        else
        {
            // Non-reentrant shutdown/cleanup is the lifetime barrier for callback
            // state. A callback re-entering cancellation must return so its own
            // stack can unwind; destroying the manager from that stack is unsupported.
            registry->drained.wait(lock, callbacksDrainedForCaller);
        }
    }

    void HttpClientManager::detachCallbacks()
    {
        auto registry = m_callbackRegistry;
        std::unique_lock<std::mutex> lock(registry->mutex);
        std::thread::id const callerThread = std::this_thread::get_id();
        registry->drained.wait(lock, [registry, callerThread] {
            for (auto const& active : registry->activeCalls)
            {
                if (active.first != callerThread)
                {
                    return false;
                }
            }
            return true;
        });
        registry->manager = nullptr;
        registry->drained.notify_all();
    }

    // start async cancellation

} MAT_NS_END
