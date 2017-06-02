// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <Version.hpp>
#include <IHttpClient.hpp>
#include <IRuntimeConfig.hpp>
#include "Route.hpp"
#include "Contexts.hpp"

namespace ARIASDK_NS_BEGIN {


class HttpRequestEncoder {
  public:
    HttpRequestEncoder(IHttpClient& httpClient, IRuntimeConfig& runtimeConfig);
    ~HttpRequestEncoder();

  protected:
    bool handleEncode(EventsUploadContextPtr const& ctx);

  protected:
    IHttpClient&    m_httpClient;
    IRuntimeConfig& m_runtimeConfig;

  public:
    RoutePassThrough<HttpRequestEncoder, EventsUploadContextPtr const&> encode{this, &HttpRequestEncoder::handleEncode};
};


} ARIASDK_NS_END
