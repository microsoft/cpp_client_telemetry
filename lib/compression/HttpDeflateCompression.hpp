// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <IRuntimeConfig.hpp>
#include "Route.hpp"
#include "Contexts.hpp"

namespace ARIASDK_NS_BEGIN {


class HttpDeflateCompression {
  public:
    HttpDeflateCompression(IRuntimeConfig& runtimeConfig);
    ~HttpDeflateCompression();

  protected:
    bool handleCompress(EventsUploadContextPtr const& ctx);

  protected:
    IRuntimeConfig& m_runtimeConfig;

  public:
    RouteSource<EventsUploadContextPtr const&>                              compressionFailed;
    RoutePassThrough<HttpDeflateCompression, EventsUploadContextPtr const&> compress{this, &HttpDeflateCompression::handleCompress};
};


} ARIASDK_NS_END
