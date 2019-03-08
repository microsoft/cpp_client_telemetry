// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IHttpClient.hpp"
#include "pal/PAL.hpp"
#include "system/Contexts.hpp"
#include "system/Route.hpp"

#include <list>
#include <mutex>

namespace ARIASDK_NS_BEGIN {


class HttpClientManager {

  public:
    HttpClientManager(IHttpClient& httpClient);
    virtual ~HttpClientManager();
    void cancelAllRequests();

    size_t requestCount() const
    {
        return m_httpCallbacks.size();
    }

  protected:
    class HttpCallback;

  protected:
    void handleSendRequest(EventsUploadContextPtr const& ctx);
    virtual void scheduleOnHttpResponse(HttpCallback* callback);
    void onHttpResponse(HttpCallback* callback);
    bool cancelAllRequestsAsync();

  protected:
    IHttpClient&             m_httpClient;

    std::mutex               m_httpCallbacksMtx;
    std::list<HttpCallback*> m_httpCallbacks;

  public:
    RouteSource<EventsUploadContextPtr const&>                  requestDone;
    RouteSink<HttpClientManager, EventsUploadContextPtr const&> sendRequest{this, &HttpClientManager::handleSendRequest};
};


} ARIASDK_NS_END
