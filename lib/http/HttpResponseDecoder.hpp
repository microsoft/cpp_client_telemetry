//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef HTTPRESPONSEDECODER_HPP
#define HTTPRESPONSEDECODER_HPP

#include "pal/PAL.hpp"

#include "system/Route.hpp"
#include "system/Contexts.hpp"
#include "system/ITelemetrySystem.hpp"

namespace MAT_NS_BEGIN {

    typedef enum
    {
        Accepted,
        Rejected,
        RetryServer,
        RetryNetwork,
        Abort
    } HttpRequestResult;

    class HttpResponseDecoder : public DebugEventDispatcher
    {
    public:
        HttpResponseDecoder(ITelemetrySystem& system);
        ~HttpResponseDecoder();

    protected:
        ITelemetrySystem & m_system;
        void processBody(IHttpResponse const& response, HttpRequestResult & result);
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

} MAT_NS_END
#endif

