// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"
#ifdef HAVE_MAT_AI
#include "AIHttpRequestEncoder.hpp"
#include "utils/Utils.hpp"
#include "pal/PAL.hpp"
#include "json.hpp"
#ifdef HAVE_MAT_ZLIB
#include <zlib.h>
#endif

#include <memory>
#include <string>

using json = nlohmann::json;

namespace ARIASDK_NS_BEGIN {
    #define GZIP_ENCODING 16

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

    bool InflateVector(const std::vector<uint8_t>& in, std::vector<uint8_t>& out)
    {
        bool result = true;

        z_stream zs;
        memset(&zs, 0, sizeof(zs));

        // [MG]: must call inflateInit2 with -9 because otherwise
        // it'd be searching for non-existing gzip header...
        if (inflateInit2(&zs,  MAX_WBITS | GZIP_ENCODING) != Z_OK)
        {
            return false;
        }

        zs.next_in = (Bytef *)in.data();
        zs.avail_in = (uInt)in.size();
        int ret;
        // The problem with 32K is that it's too small and causes corruption
        // in zlib inflate. 128KB seems to be fine.
        // Allocate a buffer enough to hold an output with Zlib max compression
        // ratio 5:1 in case it is larger than 128KB.
        uInt outbufferSize = std::max((uInt)131072, zs.avail_in * 5);

        char* outbuffer = new char[outbufferSize];
        do
        {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = outbufferSize;
            ret = inflate(&zs, Z_NO_FLUSH);
            out.insert(out.end(), outbuffer, outbuffer + zs.total_out);
        } while (ret == Z_OK);
        if (ret != Z_STREAM_END)
        {
            result = false;
        }
        inflateEnd(&zs);
        delete[] outbuffer;
        return result;
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

        ctx->httpRequest->SetUrl(m_config.GetCollectorUrl());

        // "Content-Type": "application/x-json-stream"
        ctx->httpRequest->GetHeaders().set("Content-Type", "application/json");

        if (ctx->compressed) {
            const char* contentEncoding = m_config.IsHttpRequestCompressionGzip() ? "gzip" : "deflate";
            ctx->httpRequest->GetHeaders().add("Content-Encoding", contentEncoding);
        }

#if 0
        // XXX: [MG] - debug only
        std::string str;
        if (ctx->compressed) {
            std::vector<uint8_t> buffer;
            if (InflateVector(ctx->body, buffer)) {
                str.assign(buffer.begin(), buffer.end());
            }
        } else {
            str.assign(ctx->body.begin(), ctx->body.end());
        }
        LOG_INFO("Sending body (compressed '%s'): %s", ctx->compressed ? "Yes" : "No", str.c_str());
#endif

        ctx->httpRequest->SetBody(ctx->body);
        // IHttpRequest::SetBody() is free to swap the real body out, but better clear it anyway.
        ctx->body.clear();

        ctx->httpRequest->SetLatency(ctx->latency);

        DispatchDataViewerEvent(ctx->httpRequest->GetBody());

        return true;
    }

} ARIASDK_NS_END
#endif
