// Copyright (c) Microsoft. All rights reserved.

#include "Common.hpp"
#include "zlib.h"

namespace testing {


    MATSDK_LOG_INST_COMPONENT_NS("Testing", "Unit testing helpers");

    CsProtocol::Value toCsProtocolValue(const std::string& val)
    {
        CsProtocol::Value temp;
        temp.stringValue = val;
        return temp;
    }

    CsProtocol::Value toCsProtocolValue(bool val)
    {
        CsProtocol::Value temp;
        temp.type = CsProtocol::ValueBool;
        temp.longValue = val;
        return temp;
    }


    CsProtocol::Value toCsProtocolValue(double val)
    {
        CsProtocol::Value temp;
        temp.type = CsProtocol::ValueDouble;
        temp.doubleValue = val;
        return temp;
    }


    CsProtocol::Value toCsProtocolValue(int64_t val)
    {
        CsProtocol::Value temp;
        temp.type = CsProtocol::ValueInt64;
        temp.longValue = val;
        return temp;
    }

    CsProtocol::Value toCsProtocolValue(uint64_t val)
    {
        CsProtocol::Value temp;
        temp.type = CsProtocol::ValueArrayUInt64;
        temp.longValue = val;
        return temp;
    }


    CsProtocol::Value toCsProtocolValue(MAT::EventLatency val)
    {
        CsProtocol::Value temp;
        temp.type = CsProtocol::ValueArrayInt32;
        temp.longValue = (int)val;
        return temp;
    }

    /// <summary>
    /// Compress buffer from source to dest.
    /// </summary>
    /// <param name="source"></param>
    /// <param name="sourceLen"></param>
    /// <param name="dest"></param>
    /// <param name="destLen"></param>
    /// <param name="prependSize"></param>
    /// <returns></returns>
    bool Compress(const char* source, size_t sourceLen, char** dest, size_t& destLen, bool prependSize)
    {
        if ((!source) || (!sourceLen))
            return false;

        *dest = NULL;
        destLen = 0;

        // Compressing variables
        uLong compSize = compressBound((uLong)sourceLen);

        // Allocate memory for the new compressed buffer
        size_t reserved = ((unsigned)prependSize * sizeof(uint32_t));
        char* compBody = new char[std::max(compSize, ((uLong)sourceLen)) + reserved];
        if (compBody != NULL)
        {
            if (prependSize)
            {
                // Remember source uncompressed size if requested
                uint32_t *s = (uint32_t*)(compBody);
                (*s) = (uint32_t)sourceLen; // truncate this to 32-bit, we do not support 3+ TB blobs
            }
            // Deflate
            int res = compress2((Bytef *)(compBody + reserved), &compSize, (Bytef *)source, (uLong)sourceLen, Z_BEST_SPEED);
            if (res != Z_OK)
            {
                LOG_ERROR("Compression failed, error=%u", res);
                delete[] compBody;
                compBody = NULL;
                return false;
            }
            else
            {
                *dest = compBody;
                destLen = compSize + reserved;
                return true;
            }
        }
        // OOM
        return false;
    }

    /// <summary>
    /// Expand buffer from source to dest.
    /// </summary>
    /// <param name="source"></param>
    /// <param name="sourceLen"></param>
    /// <param name="dest"></param>
    /// <param name="destLen"></param>
    /// <param name="sizeAtZeroIndex"></param>
    /// <returns></returns>
    bool Expand(const char* source, size_t sourceLen, char** dest, size_t& destLen, bool sizeAtZeroIndex)
    {
        if (!(source) || !(sourceLen))
            return false;

        *dest = NULL;

        unsigned reserved = (unsigned)sizeAtZeroIndex * sizeof(uint32_t);
        // Get uncompressed size at zero offset.
        if (sizeAtZeroIndex)
        {
            uint32_t s32 = *((uint32_t*)(source));
            uint64_t s64 = (sourceLen >= sizeof(uint64_t)) ? *((uint64_t*)(source)) : 0;
            // If we are reading 64-bit generated legacy DB, step 32-bit forward to
            // skip zero-padding in most-significant DWORD on Intel architecture
            if ((s64 - s32) == 0)
                reserved += sizeof(uint32_t);
            destLen = s32;
        }

        // Allocate memory for the new uncompressed buffer
        if (destLen > 0)
        {
            try {
                char* decompBody = new char[destLen];
                if (source != NULL)
                {
                    // Inflate
                    uLongf len = (uLongf)destLen;
                    int res = uncompress((Bytef *)decompBody, &len, (const Bytef *)(source + reserved), (uLong)(sourceLen - reserved));
                    if ((res != Z_OK) || (len != destLen))
                    {
                        LOG_ERROR("Decompression failed, error=%d, len=%z, destLen=%z", res, len, (unsigned int)destLen);
                        delete[] decompBody;
                        return false;
                    }
                    *dest = decompBody;
                    destLen = len;
                    return true;
                }
            }
            catch (std::bad_alloc&) {
                LOG_ERROR("Decompression failed (out of memory): destLen=%u", destLen);
                dest = NULL;
                destLen = 0;
            }
        }

        // OOM
        return false;
    }

    void InflateVector(std::vector<uint8_t> &in, std::vector<uint8_t> &out, bool isGzip)
    {
        z_stream zs;
        memset(&zs, 0, sizeof(zs));
        // -MAX_WBITS for no header
        int windowBits = isGzip ? (MAX_WBITS | 16) : -MAX_WBITS;
        EXPECT_EQ(inflateInit2(&zs, windowBits), Z_OK);
        zs.next_in = (Bytef *)in.data();
        zs.avail_in = (uInt)in.size();
        int ret;
        char outbuffer[32768] = { 0 };
        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);
            ret = inflate(&zs, Z_NO_FLUSH);
            out.insert(out.end(), outbuffer, outbuffer + zs.total_out);
        } while (ret == Z_OK);
        EXPECT_EQ(ret, Z_STREAM_END);
        inflateEnd(&zs);
    }

} // namespace testing
