// CopyrighIt (c) Microsoft. All rights reserved.
#ifndef AIHTTPRESPONSEDECODER_HPP
#define AIHTTPRESPONSEDECODER_HPP

#include "pal/PAL.hpp"

#include "system/Route.hpp"
#include "system/Contexts.hpp"
#include "system/ITelemetrySystem.hpp"
#include "http/HttpResponseDecoder.hpp"

namespace ARIASDK_NS_BEGIN {

    class AIHttpResponseDecoder : public DebugEventDispatcher
    {
    public:
        AIHttpResponseDecoder(ITelemetrySystem& system);
        ~AIHttpResponseDecoder();

    protected:
        ITelemetrySystem & m_system;
        void processBody(IHttpResponse const& response, HttpRequestResult & result);
        void handleDecode(EventsUploadContextPtr const& ctx);

    public:
        RouteSink<AIHttpResponseDecoder, EventsUploadContextPtr const&> decode{ this, &AIHttpResponseDecoder::handleDecode };
        RouteSource<EventsUploadContextPtr const&>                    eventsAccepted;
        RouteSource<EventsUploadContextPtr const&>                    eventsRejected;
        RouteSource<EventsUploadContextPtr const&>                    temporaryNetworkFailure;
        RouteSource<EventsUploadContextPtr const&>                    temporaryServerFailure;
        RouteSource<EventsUploadContextPtr const&>                    requestAborted;

        virtual bool DispatchEvent(DebugEvent evt) override;
    };

} ARIASDK_NS_END
#endif
