// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <Version.hpp>
#include "Route.hpp"
#include "Contexts.hpp"

namespace ARIASDK_NS_BEGIN {


class HttpResponseDecoder {
  public:
    HttpResponseDecoder();
    ~HttpResponseDecoder();

  protected:
    void handleDecode(EventsUploadContextPtr const& ctx);

  public:
    RouteSink<HttpResponseDecoder, EventsUploadContextPtr const&> decode{this, &HttpResponseDecoder::handleDecode};
    RouteSource<EventsUploadContextPtr const&>                    eventsAccepted;
    RouteSource<EventsUploadContextPtr const&>                    eventsRejected;
    RouteSource<EventsUploadContextPtr const&>                    temporaryNetworkFailure;
    RouteSource<EventsUploadContextPtr const&>                    temporaryServerFailure;
    RouteSource<EventsUploadContextPtr const&>                    requestAborted;
};


} ARIASDK_NS_END
