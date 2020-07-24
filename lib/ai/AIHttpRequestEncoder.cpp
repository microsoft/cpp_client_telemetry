// Copyright (c) Microsoft. All rights reserved.

#include "AIHttpRequestEncoder.hpp"
#include "utils/Utils.hpp"
#include "pal/PAL.hpp"
#include "json.hpp"

#include <memory>
#include <string>

using json = nlohmann::json;

namespace ARIASDK_NS_BEGIN {

    AIHttpRequestEncoder::AIHttpRequestEncoder(ITelemetrySystem& system, IHttpClient& httpClient)
        :
        m_system(system),
        m_httpClient(httpClient),
        m_config(system.getConfig())
    {
    }

    AIHttpRequestEncoder::~AIHttpRequestEncoder()
    {
    }

    void AIHttpRequestEncoder::DispatchDataViewerEvent(const StorageBlob& dataPacket)
    {
        m_system.getLogManager().GetDataViewerCollection().DispatchDataViewerEvent(dataPacket);
    }

    bool AIHttpRequestEncoder::handleEncode(EventsUploadContextPtr const& ctx)
    {
        ctx->httpRequest = m_httpClient.CreateRequest();
        ctx->httpRequestId = ctx->httpRequest->GetId();

        ctx->httpRequest->SetMethod("POST");

        ctx->httpRequest->SetUrl("https://dc.services.visualstudio.com/v2/track");

        // "Content-Type": "application/x-json-stream"
        ctx->httpRequest->GetHeaders().set("Content-Type", "application/json");

//        // "gzip"
////        if (ctx->compressed) {
////            ctx->httpRequest->GetHeaders().add("Content-Encoding",  "deflate");
////        }
//
#if 1
        // XXX: [MG] - debug only
        std::string str(ctx->body.begin(), ctx->body.end());
#endif

        ctx->httpRequest->SetBody(ctx->body);
        // IHttpRequest::SetBody() is free to swap the real body out, but better clear it anyway.
        ctx->body.clear();

        ctx->httpRequest->SetLatency(ctx->latency);

        DispatchDataViewerEvent(ctx->httpRequest->GetBody());

        return true;
    }

} ARIASDK_NS_END