// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <aria/Version.hpp>
#include <aria/IHttpClient.hpp>
#include <aria/IRuntimeConfig.hpp>
#include "system/Route.hpp"
#include "system/Contexts.hpp"

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
