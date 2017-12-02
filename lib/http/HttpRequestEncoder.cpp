// Copyright (c) Microsoft. All rights reserved.

#include "HttpRequestEncoder.hpp"
#include "utils/Utils.hpp"
#include "pal/PAL.hpp"
#include <memory>
#include <string>

#include "utils/Utils.hpp"
#include <bond_lite/All.hpp>
#include "bond/generated/AriaProtocol_writers.hpp"
#include "bond/generated/AriaProtocol_readers.hpp"

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
    ctx->httpRequest->GetHeaders().set("Client-Version",  PAL::getSdkVersion());
    ctx->httpRequest->GetHeaders().set("Client-Id",    "NO_AUTH");
    ctx->httpRequest->GetHeaders().set("Content-Type", "application/bond-compact-binary");
    ctx->httpRequest->GetHeaders().set("Upload-Time", toString(PAL::getUtcSystemTimeMs()));

 /*   if (!deviceTicketToken.empty())
    {
        // If we have the ticket add it in a header
        AddRequestHeader(headers, L"AuthMsaDeviceTicket", deviceTicketToken);
    }

    if (!xtokenAuthHeader.empty())
    {
        // If we an XToken auth header, add it.
        AddRequestHeader(headers, L"AuthXToken", xtokenAuthHeader);
    }

    if (!ticketHeader.empty())
    {
        // If we have an XToken / MSA User id mapping header, add it.
        AddRequestHeader(headers, L"Tickets", ticketHeader);
    }

    if (!aadTicketHeader.empty())
    {
        // If we have a base 64 encoded AAD device ticket, add it.
        AddRequestHeader(headers, L"Aad-Token", aadTicketHeader);
    }
 */ 
    std::string tenantTokens;
    tenantTokens.reserve(ctx->packageIds.size() * 75); // Tenants tokens are usually 74 chars long.
    for (auto const& item : ctx->packageIds) {
        if (!tenantTokens.empty()) {
            tenantTokens.push_back(',');
        }
        tenantTokens.append(item.first);
    }
    ctx->httpRequest->GetHeaders().set("APIKey", tenantTokens);

    if (ctx->compressed) {
        ctx->httpRequest->GetHeaders().add("Content-Encoding", "deflate");
    }


  //  AriaProtocol::CsEvent result;
 //   bond_lite::CompactBinaryProtocolReader reader(ctx->body);
 //   bond_lite::Deserialize(reader, result);

    //std::string str(ctx->body.begin(), ctx->body.end());

    ctx->httpRequest->SetBody(ctx->body);
    // IHttpRequest::SetBody() is free to swap the real body out, but better clear it anyway.
    ctx->body.clear();

    ctx->httpRequest->SetLatency(ctx->latency);

    return true;
}


} ARIASDK_NS_END
