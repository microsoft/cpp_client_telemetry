// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"

#include "HttpDeflateCompression.hpp"
#include "utils/Utils.hpp"
#ifdef HAVE_MAT_ZLIB
#define ZLIB_CONST
#include <zlib.h>
#endif

namespace ARIASDK_NS_BEGIN {
    #define windowBits
    #define GZIP_ENCODING 16

    HttpDeflateCompression::HttpDeflateCompression(IRuntimeConfig& runtimeConfig)
        : m_config(runtimeConfig)
    {
    }

    HttpDeflateCompression::~HttpDeflateCompression()
    {
    }

    bool HttpDeflateCompression::handleCompress(EventsUploadContextPtr const& ctx)
    {
        UNREFERENCED_PARAMETER(ctx);
#ifdef HAVE_MAT_ZLIB
        if (!m_config.IsHttpRequestCompressionEnabled()) {
            return true;
        }

        // Using a slightly adapted in-place compression technique as suggested
        // by Mark Adler himself: http://stackoverflow.com/a/12412863/3543211

        z_stream stream;
        memset(&stream, 0, sizeof(stream));

        // All values are defaults as would be used by a plain deflate(), except
        // for the negative -MAX_WBITS argument which makes zlib use "raw deflate"
        // without zlib header, as required by IIS.
        int windowsBits = -MAX_WBITS;
        if (m_config.IsHttpRequestCompressionGzip()) {
            windowsBits = MAX_WBITS | GZIP_ENCODING;
        }

        int result = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowsBits, 8 /*DEF_MEM_LEVEL*/, Z_DEFAULT_STRATEGY);
        if (result != Z_OK) {
            LOG_WARN("HTTP request compressing failed, error=%u/%u (%s)", 1, result, stream.msg);
            compressionFailed(ctx);
            return false;
        }

        stream.avail_in = static_cast<uInt>(ctx->body.size());
        ctx->body.resize(deflateBound(&stream, stream.avail_in));
        stream.next_in = ctx->body.data();

        std::vector<uint8_t> temp(32);
        stream.next_out = temp.data();
        stream.avail_out = static_cast<uInt>(temp.size());

        result = deflate(&stream, Z_FINISH);
        if (result == Z_OK || result == Z_STREAM_END) {
            std::copy(temp.cbegin(), temp.cbegin() + stream.total_out, ctx->body.begin());
            stream.next_out = ctx->body.data() + stream.total_out;
            stream.avail_out = static_cast<uInt>(ctx->body.size()) - static_cast<uInt>(stream.total_out);
        }
        if (result == Z_OK) {
            result = deflate(&stream, Z_FINISH);
            if (result == Z_OK) {
                // The output shall never catch up with the input. Mark says in the
                // Stack Overflow reference above that this should happen only for
                // incompressible data longer than 20 MB, i.e. virtually never for
                // telemetry events in Bond. But better make it 100% safe here.
                temp.assign(ctx->body.cbegin() + stream.total_in, ctx->body.cbegin() + stream.total_in + stream.avail_in);
                stream.next_in = temp.data();
                stream.avail_in = static_cast<uInt>(temp.size());
                result = deflate(&stream, Z_FINISH);
            }
        }

        deflateEnd(&stream);

        if (result != Z_STREAM_END) {
            LOG_WARN("HTTP request compressing failed, error=%u/%u (%s)", 2, result, stream.msg);
            compressionFailed(ctx);
            return false;
        }

        ctx->body.resize(stream.total_out);
        ctx->compressed = true;
#endif
        return true;
    }


} ARIASDK_NS_END
