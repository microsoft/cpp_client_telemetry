// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "Version.hpp"
#include "api/IRuntimeConfig.hpp"
#include "system/Route.hpp"
#include "system/Contexts.hpp"

namespace ARIASDK_NS_BEGIN {


    class HttpDeflateCompression {
    public:
        HttpDeflateCompression(IRuntimeConfig& runtimeConfig);
        ~HttpDeflateCompression();

    protected:
        bool handleCompress(EventsUploadContextPtr const& ctx);

    protected:
        IRuntimeConfig& m_config;

    public:
        RouteSource<EventsUploadContextPtr const&>                              compressionFailed;
        RoutePassThrough<HttpDeflateCompression, EventsUploadContextPtr const&> compress{ this, &HttpDeflateCompression::handleCompress };
    };

} ARIASDK_NS_END
