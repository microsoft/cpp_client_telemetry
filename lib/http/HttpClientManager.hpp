// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IHttpClient.hpp"
#include "pal/PAL.hpp"
#include "system/Contexts.hpp"
#include "system/Route.hpp"
#include "ILogManager.hpp"

#include <list>
#include <mutex>

namespace ARIASDK_NS_BEGIN
{

class HttpClientManager
{

    public:

        HttpClientManager(
                ILogManager& logManager,
                IHttpClient& httpClient,
                ITaskDispatcher& taskDispatcher);

        virtual ~HttpClientManager() noexcept;

        void cancelAllRequests();

        size_t requestCount() const
        {
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
        bool cancelAllRequestsAsync();

        ILogManager&              m_logManager;
        IHttpClient&              m_httpClient;
        ITaskDispatcher&          m_taskDispatcher;
        std::mutex                m_httpCallbacksMtx;
        std::list<HttpCallback*>  m_httpCallbacks;
};

} ARIASDK_NS_END
