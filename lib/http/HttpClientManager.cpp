// Copyright (c) Microsoft. All rights reserved.

#include "HttpClientManager.hpp"
#include "utils/Common.hpp"
#include <assert.h>

namespace ARIASDK_NS_BEGIN {


class HttpClientManager::HttpCallback : public IHttpResponseCallback {
  public:
    HttpCallback(HttpClientManager& hcm, EventsUploadContextPtr const& ctx)
      : m_hcm(hcm),
        m_ctx(ctx),
        m_startTime(PAL::getMonotonicTimeMs())
    {
    }

    virtual void OnHttpResponse(IHttpResponse const* response) override
    {
        m_ctx->durationMs = static_cast<int>(PAL::getMonotonicTimeMs() - m_startTime);
        m_ctx->httpResponse.reset(response);
        m_hcm.scheduleOnHttpResponse(this);
    }

  public:
    HttpClientManager&     m_hcm;
    EventsUploadContextPtr m_ctx;
    int64_t                m_startTime;
};

//---

HttpClientManager::HttpClientManager(IHttpClient& httpClient)
  : m_httpClient(httpClient)
{
}

HttpClientManager::~HttpClientManager()
{
    assert(m_httpCallbacks.empty());
}

void HttpClientManager::handleSendRequest(EventsUploadContextPtr const& ctx)
{
    std::unique_ptr<HttpCallback> callback(new HttpCallback(*this, ctx));
    m_httpCallbacks.push_back(callback.get());

    ARIASDK_LOG_INFO("Uploading %u event(s) of priority %d (%s) for %u tenant(s) in HTTP request %s (approx. %u bytes)...",
        static_cast<unsigned>(ctx->recordIds.size()), ctx->priority, priorityToStr(ctx->priority), static_cast<unsigned>(ctx->packageIds.size()),
        ctx->httpRequest->GetId().c_str(), static_cast<unsigned>(ctx->httpRequest->GetSizeEstimate()));
    m_httpClient.SendRequestAsync(ctx->httpRequest.release(), callback.release());
}

void HttpClientManager::scheduleOnHttpResponse(HttpCallback* callback)
{
    PAL::executeOnWorkerThread(self(), &HttpClientManager::onHttpResponse, callback);
}

void HttpClientManager::onHttpResponse(HttpCallback* callback)
{
    EventsUploadContextPtr ctx = callback->m_ctx;

    assert(std::find(m_httpCallbacks.cbegin(), m_httpCallbacks.cend(), callback) != m_httpCallbacks.end());
    m_httpCallbacks.remove(callback);
    delete callback;

    IHttpResponse const& response = *ctx->httpResponse;
    ARIASDK_LOG_DETAIL("HTTP response %s: result=%u, status=%u, body=%u bytes",
        response.GetId().c_str(), response.GetResult(), response.GetStatusCode(), static_cast<unsigned>(response.GetBody().size()));

    requestDone(ctx);
}

bool HttpClientManager::handleCancelAllRequestsAsync()
{
    if (!m_httpCallbacks.empty()) {
        ARIASDK_LOG_DETAIL("Cancelling %u outstanding HTTP requests...",
            static_cast<unsigned>(m_httpCallbacks.size()));

        for (HttpCallback* callback : m_httpCallbacks) {
            m_httpClient.CancelRequestAsync(callback->m_ctx->httpRequestId);
        }
        // Requests will be cancelled asynchronously.
    }

    return true;
}


} ARIASDK_NS_END
