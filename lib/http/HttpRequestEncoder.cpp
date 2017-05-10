// Copyright (c) Microsoft. All rights reserved.

#include "HttpRequestEncoder.hpp"
#include "utils/Common.hpp"
#include <memory>

namespace ARIASDK_NS_BEGIN {


HttpRequestEncoder::HttpRequestEncoder(IHttpClient& httpClient, IRuntimeConfig& runtimeConfig)
  : m_httpClient(httpClient),
    m_runtimeConfig(runtimeConfig)
{
}

HttpRequestEncoder::~HttpRequestEncoder()
{
}

bool HttpRequestEncoder::handleEncode(EventsUploadContextPtr const& ctx)
{
    ctx->httpRequest.reset(m_httpClient.CreateRequest());
    ctx->httpRequestId = ctx->httpRequest->GetId();

    ctx->httpRequest->SetMethod("POST");

    ctx->httpRequest->SetUrl(m_runtimeConfig.GetCollectorUrl());

    ctx->httpRequest->GetHeaders().set("Expect",       "100-continue");
    ctx->httpRequest->GetHeaders().set("SDK-Version",  PAL::getSdkVersion());
    ctx->httpRequest->GetHeaders().set("Client-Id",    "NO_AUTH");
    ctx->httpRequest->GetHeaders().set("Content-Type", "application/bond-compact-binary");

    std::string tenantTokens;
    tenantTokens.reserve(ctx->packageIds.size() * 75); // Tenants tokens are usually 74 chars long.
    for (auto const& item : ctx->packageIds) {
        if (!tenantTokens.empty()) {
            tenantTokens.push_back(',');
        }
        tenantTokens.append(item.first);
    }
    ctx->httpRequest->GetHeaders().set("X-APIKey", tenantTokens);

    if (ctx->compressed) {
        ctx->httpRequest->GetHeaders().add("Content-Encoding", "deflate");
    }

    ctx->httpRequest->SetBody(ctx->body);
    // IHttpRequest::SetBody() is free to swap the real body out, but better clear it anyway.
    ctx->body.clear();

    ctx->httpRequest->SetPriority(ctx->priority);

    return true;
}


} ARIASDK_NS_END
