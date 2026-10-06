//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"
#include "ZlibUtils.hpp"
#include "pal/PAL.hpp"
#include <array>

#ifdef HAVE_MAT_ZLIB
#define ZLIB_CONST
#include <zlib.h>
#endif

namespace MAT_NS_BEGIN
{
    constexpr std::size_t ZlibUtils::MAX_INFLATED_SIZE;

    bool ZlibUtils::InflateVector(const std::vector<uint8_t>& in, std::vector<uint8_t>& out, bool isGzip)
    {
#ifdef HAVE_MAT_ZLIB
        bool result = true;
        if (in.size() > MAX_INFLATED_SIZE || out.size() > MAX_INFLATED_SIZE)
        {
            LOG_WARN("Compressed payload exceeds decoder size limit");
            return false;
        }

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
        std::array<uint8_t, 16 * 1024> outbuffer;
        do
        {
            zs.next_out = outbuffer.data();
            zs.avail_out = static_cast<uInt>(outbuffer.size());
            ret = inflate(&zs, Z_NO_FLUSH);
            const size_t count = outbuffer.size() - zs.avail_out;
            if (count > MAX_INFLATED_SIZE - out.size())
            {
                LOG_WARN("Inflated payload exceeds %zu bytes; rejecting", MAX_INFLATED_SIZE);
                result = false;
                break;
            }
            out.insert(out.end(), outbuffer.data(), outbuffer.data() + count);
        } while (ret == Z_OK);
        if (ret != Z_STREAM_END)
        {
            LOG_WARN("Inflate failed, error=%d/%d (%s)", 2, ret, (zs.msg ? zs.msg : "(null)"));
            result = false;
        }
        inflateEnd(&zs);
        return result;
#else
        UNREFERENCED_PARAMETER(in);
        UNREFERENCED_PARAMETER(out);
        UNREFERENCED_PARAMETER(isGzip);
        return false;
#endif
    }

} MAT_NS_END
