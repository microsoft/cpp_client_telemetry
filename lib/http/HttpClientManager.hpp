// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <IHttpClient.hpp>
#include "pal/PAL.hpp"
#include "system/Contexts.hpp"
#include "system/Route.hpp"
#include <list>

namespace ARIASDK_NS_BEGIN {


class HttpClientManager {
  public:
    HttpClientManager(IHttpClient& httpClient);
    virtual ~HttpClientManager();

  protected:
    class HttpCallback;

  protected:
    void handleSendRequest(EventsUploadContextPtr const& ctx);
    virtual void scheduleOnHttpResponse(HttpCallback* callback);
    void onHttpResponse(HttpCallback* callback);
    bool handleCancelAllRequestsAsync();

  protected:
    IHttpClient&             m_httpClient;
    std::list<HttpCallback*> m_httpCallbacks;

  public:
    RouteSource<EventsUploadContextPtr const&>                  requestDone;
    RouteSink<HttpClientManager, EventsUploadContextPtr const&> sendRequest{this, &HttpClientManager::handleSendRequest};

    RoutePassThrough<HttpClientManager>                         cancelAllRequestsAsync{this, &HttpClientManager::handleCancelAllRequestsAsync};
};


} ARIASDK_NS_END
