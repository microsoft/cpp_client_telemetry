//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

#include "HttpDeflateCompression.hpp"
#include "utils/Utils.hpp"
#ifdef HAVE_MAT_ZLIB
#define ZLIB_CONST
#include <zlib.h>
#endif

namespace MAT_NS_BEGIN {

    HttpDeflateCompression::HttpDeflateCompression(IRuntimeConfig& runtimeConfig)
        : m_config(runtimeConfig)
    {
        // Plain "deflate": negative -MAX_WBITS argument which makes zlib use "raw deflate"
        // without zlib header, as required by IIS.
        // "gzip": Add 16 to windowBits to write a simple gzip header
#ifdef HAVE_MAT_ZLIB
        m_windowBits = m_config.GetHttpRequestContentEncoding() == "gzip" ? (MAX_WBITS | 16) : -MAX_WBITS;
#endif
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

        int result = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, m_windowBits, 8 /*DEF_MEM_LEVEL*/, Z_DEFAULT_STRATEGY);
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


} MAT_NS_END

