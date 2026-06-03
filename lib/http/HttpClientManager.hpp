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

        void cancelAllRequests();

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
        bool cancelAllRequestsAsync();

        ILogManager&              m_logManager;
        IHttpClient&              m_httpClient;
        ITaskDispatcher&          m_taskDispatcher;
        mutable std::recursive_mutex m_httpCallbacksMtx;
        std::list<HttpCallback*>  m_httpCallbacks;
};

} MAT_NS_END

