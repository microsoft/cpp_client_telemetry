// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "pal/PAL.hpp"
#include "IHttpClient.hpp"

#include "api/IRuntimeConfig.hpp"
#include "system/Route.hpp"
#include "system/Contexts.hpp"
#include "system/ITelemetrySystem.hpp"

#include "IAuthTokensController.hpp"

namespace ARIASDK_NS_BEGIN {

    class HttpRequestEncoder {
    public:
        HttpRequestEncoder(ITelemetrySystem& system, IHttpClient& httpClient);
        ~HttpRequestEncoder();

        RoutePassThrough<HttpRequestEncoder, EventsUploadContextPtr const&> encode { this, &HttpRequestEncoder::handleEncode };

    protected:
        bool handleEncode(EventsUploadContextPtr const& ctx);

        ITelemetrySystem &      m_system;
        IHttpClient &           m_httpClient;
        IRuntimeConfig&         m_config;

        IAuthTokensController* GetAuthTokensController()
        {
            return m_system.getLogManager().GetAuthTokensController();
        }

        virtual void DispatchDataViewerEvent(const StorageBlob& dataPacket);
    };


} ARIASDK_NS_END
