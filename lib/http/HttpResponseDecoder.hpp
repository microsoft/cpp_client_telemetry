// Copyright (c) Microsoft. All rights reserved.
#pragma once
#include <pal/PAL.hpp>

#include "system/Route.hpp"
#include "system/Contexts.hpp"
#include "system/ITelemetrySystem.hpp"

namespace ARIASDK_NS_BEGIN {

    class HttpResponseDecoder : public DebugEventDispatcher
    {
    public:
        HttpResponseDecoder(ITelemetrySystem& system);
        ~HttpResponseDecoder();

    protected:
        ITelemetrySystem & m_system;
        void handleDecode(EventsUploadContextPtr const& ctx);

    public:
        RouteSink<HttpResponseDecoder, EventsUploadContextPtr const&> decode{ this, &HttpResponseDecoder::handleDecode };
        RouteSource<EventsUploadContextPtr const&>                    eventsAccepted;
        RouteSource<EventsUploadContextPtr const&>                    eventsRejected;
        RouteSource<EventsUploadContextPtr const&>                    temporaryNetworkFailure;
        RouteSource<EventsUploadContextPtr const&>                    temporaryServerFailure;
        RouteSource<EventsUploadContextPtr const&>                    requestAborted;

        virtual bool DispatchEvent(DebugEvent evt) override;
    };

} ARIASDK_NS_END
