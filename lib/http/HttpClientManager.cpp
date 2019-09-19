// Copyright (c) Microsoft. All rights reserved.

#include "HttpClientManager.hpp"
#include "utils/Utils.hpp"
#include "pal/TaskDispatcher.hpp"

#include <assert.h>
#include <algorithm>

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

namespace ARIASDK_NS_BEGIN {


    class HttpClientManager::HttpCallback : public IHttpResponseCallback {
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

        virtual ~HttpCallback()
        {
            LOG_TRACE("destroy HTTP callback=%p ctx=%p", this, m_ctx);
        }

    public:
        HttpClientManager & m_hcm;
        EventsUploadContextPtr m_ctx;
        int64_t                m_startTime;
    };

    //---

    HttpClientManager::HttpClientManager(IHttpClient& httpClient, ITaskDispatcher& taskDispatcher) :
        m_httpClient(httpClient),
        m_taskDispatcher(taskDispatcher)
    {
    }

    HttpClientManager::~HttpClientManager()
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

    void HttpClientManager::onHttpResponse(HttpCallback* callback)
    {
        EventsUploadContextPtr &ctx = callback->m_ctx;

        {
            LOCKGUARD(m_httpCallbacksMtx);
            assert(std::find(m_httpCallbacks.cbegin(), m_httpCallbacks.cend(), callback) != m_httpCallbacks.end());

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
        }

        delete callback;
    }

    bool HttpClientManager::cancelAllRequestsAsync()
    {
        m_httpClient.CancelAllRequests();
        return true;
    }

    void HttpClientManager::cancelAllRequests()
    {
        cancelAllRequestsAsync();
        while (!m_httpCallbacks.empty())
            std::this_thread::yield();
    }

    // start async cancellation

} ARIASDK_NS_END
