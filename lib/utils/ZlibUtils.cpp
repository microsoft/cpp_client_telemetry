//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"
#include "ZlibUtils.hpp"
#include "pal/PAL.hpp"

#ifdef HAVE_MAT_ZLIB
#define ZLIB_CONST
#include <zlib.h>
#endif

namespace MAT_NS_BEGIN
{
    bool ZlibUtils::InflateVector(const std::vector<uint8_t>& in, std::vector<uint8_t>& out, bool isGzip)
    {
#ifdef HAVE_MAT_ZLIB
        bool result = true;

        z_stream zs;
        memset(&zs, 0, sizeof(zs));

        // "deflate": negative -MAX_WBITS argument which makes zlib use "raw deflate" format,
        // "gzip": Add 16 to windowBits to decode a simple gzip header
        int windowBits = isGzip ? (MAX_WBITS | 16) : -MAX_WBITS;
        if (inflateInit2(&zs, windowBits) != Z_OK)
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
            out.insert(out.end(), outbuffer, outbuffer + (outbufferSize - zs.avail_out));
        } while (ret == Z_OK);
        if (ret != Z_STREAM_END)
        {
            LOG_WARN("Inflate failed, error=%u/%u (%s)", 2, ret, zs.msg);
            result = false;
        }
        inflateEnd(&zs);
        delete[] outbuffer;
        return result;
#else
        UNREFERENCED_PARAMETER(in);
        UNREFERENCED_PARAMETER(out);
        UNREFERENCED_PARAMETER(isGzip);
        return false;
#endif
    }

} MAT_NS_END
