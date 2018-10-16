#include "AriaDecoderV3.hpp"

// TODO: move Compress and Expand into separate utilities module
#define LOG_ERROR       printf

#include <Windows.h>

// XXX: sometimes Windows.h defines max as a macro in some Windows SDKs
#ifdef max
#undef max
#undef min
#endif

#include <algorithm>
#include <chrono>

// Definitions needed for Bond protocol decoder
#include <bondlite/generated/DataPackage_types.hpp>
#include <bondlite/generated/DataPackage_readers.hpp>
#include <bondlite/generated/DataPackage_writers.hpp>
#include <bondlite/include/bond_lite/CompactBinaryProtocolReader.hpp>

#define MAX_GUID_LEN    36  /* maximum GUIDv4 length with hyphens, but without curly braces */
#define SNPRINTF        snprintf

#include <json.hpp>

using nlohmann::json;

namespace clienttelemetry {
    namespace data {
        namespace v3 {

            void to_json(json& j, const DataPackage& p);

            void to_json(json& j, const struct ClientToCollectorRequest& r) {
                j = json {
                    { "RequestRetryCount", r.RequestRetryCount}, // 2: optional int32 RequestRetryCount
                };

                // 1: optional vector<DataPackage> DataPackages
                json dataPackages;
                for (auto &v : r.DataPackages)
                {
                    json jdp;
                    to_json(jdp, v);
                    dataPackages.push_back(jdp);
                }
                j["DataPackages"] = dataPackages;

                // 3: optional map<string, vector<DataPackage>> TokenToDataPackagesMap
                json tokenToDataPackagesMap;
#if 0
                for (auto &kv : r.TokenToDataPackagesMap)
                {
                    for (auto &dp : kv.second)
                    {
                        json jdp;
                        to_json(jdp, dp);
                        tokenToDataPackagesMap[kv.first].push_back(jdp);
                    }
                }
#endif
                j["TokenToDataPackagesMap"] = tokenToDataPackagesMap;
            }

            void to_json(json& j, const Record& r);

            void to_json(json& j, const DataPackage& p) {
                j = json {
                    { "Type",    p.Type },                       // 1: optional string Type
                    { "Source",  p.Source },                     // 2: optional string Source
                    { "Version", p.Version },                    // 3: optional string Version
                    { "Ids",     p.Ids },                        // 4: optional map<string, string> Ids
                    { "DataPackageId", p.DataPackageId },        // 5: optional string DataPackageId
                    { "Timestamp", p.Timestamp },                // 6: optional int64 Timestamp
                    { "SchemaVersion", p.SchemaVersion },        // 7: optional int32 SchemaVersion
                };

                json records = {};
                for (auto &r : p.Records)
                {
                    json j2;
                    to_json(j2, r);
                    records.push_back(j);
                }
                j["Records"] = records;
            }

            void to_json(json& j, const Record& r) {
                j = json {
                    { "Id",         r.Id },                      // 1: optional string Id
                    { "Timestamp",  r.Timestamp },               // 3: optional int64 Timestamp
                    { "ConfigurationIds", r.ConfigurationIds },  // 4: optional map<string, string> ConfigurationIds
                    { "Type",       r.Type },                    // 5: optional string Type
                    { "EventType",  r.EventType },               // 6: optional string EventType
                    { "Extension",  r.Extension },               // 13: optional map<string, string> Extension
                    { "ContextIds", r.ContextIds },              // 19: optional map<string, string> ContextIds
                    { "RecordType", (unsigned)(r.RecordType) },  // 24: optional RecordType RecordType
                };
                // 30: optional map<string, PII> PIIExtensions
                json piiExtensions;
                for (auto &kv : r.PIIExtensions)
                {
                    json pii;
                    pii["ScrubType"]  = kv.second.ScrubType;
                    pii["Kind"]       = kv.second.Kind;
                    pii["RawContent"] = kv.second.RawContent;
                    piiExtensions[kv.first] = pii;
                }
                j["PIIExtensions"]  = piiExtensions;

                j["Extension"]      = r.Extension;
                j["ExtensionInt64"] = r.ExtensionInt64;
            }

        }
    }
}

namespace common
{
    /// <summary>
    /// Get timestamp in milliseconds
    /// </summary>
    /// <returns></returns>
    uint64_t GetCurrentTimeStampMs()
    {
        return std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
    }

    /**
    * Convert Windows GUID type to string
    */
    std::string GuidToString(GUID *guid)
    {
        char buff[MAX_GUID_LEN + 1] = { 0 };
        int rc = SNPRINTF(buff, sizeof(buff),
            "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
            guid->Data1, guid->Data2, guid->Data3,
            guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
            guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
        (rc); // If something is not right, then the string is going to be empty.
        return std::string((const char*)(buff));
    }

    /**
    * Much faster Windows-specific implementation of random GUID generator...
    * Make GUID great again!
    */
    std::string CreateGUIDv4() {
        GUID guid;
        if (CoCreateGuid(&guid) == S_OK)
            return common::GuidToString(&guid);
        // If CoCreateGuid RPC call to UuidCreate fails (which NEVER happens),
        // then return GUID_NULL in lieu of any better failsafe option here.
        return "00000000-0000-0000-0000-000000000000";
    }

    std::string CreateGUID()
    {
        return CreateGUIDv4();
    }
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
                    LOG_ERROR("Decompression failed, error=%d, len=%u, destLen=%u", res, len, (unsigned int)destLen);
                    delete[] decompBody;
                    return false;
                }
                *dest = decompBody;
                destLen = len;
                return true;
            }
        }
        catch (std::bad_alloc& ex) {
            (ex);
            LOG_ERROR("Decompression failed (out of memory): destLen=%zu", destLen);
            dest = NULL;
            destLen = 0;
        }
    }

    // OOM
    return false;
}


void AriaDecoderV3::ExpandVector(std::vector<uint8_t> &in, std::vector<uint8_t> &out)
{
    size_t destLen = out.size();
    std::cout << "size=" << destLen << std::endl;
    char *buffer = nullptr;
    Expand((const char*)(in.data()), in.size(), &buffer, destLen, false); // TODO: Check if true
    out = std::vector<uint8_t>(buffer, buffer + destLen);
    if (buffer)
        delete[] buffer;

}

void AriaDecoderV3::InflateVector(std::vector<uint8_t> &in, std::vector<uint8_t> &out)
{
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    // [MG]: must call inflateInit2 with -9 because otherwise
    // it'd be searching for non-existing gzip header...
    if (inflateInit2(&zs, -9) != Z_OK)
        return;

    zs.next_in = (Bytef *)in.data();
    zs.avail_in = (uInt)in.size();
    int ret;
    // FIXME: [MG] - ideally we should allocate the decompression buffer on heap,
    // not on stack. The problem with 32K is that it's too small and causes corruption
    // in zlib inflate. 128KB seems to be fine.
    char outbuffer[131072] = { 0 };
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);
        ret = inflate(&zs, Z_NO_FLUSH);
        out.insert(out.end(), outbuffer, outbuffer + zs.total_out);
    } while (ret == Z_OK);
    if (ret != Z_STREAM_END)
    {
        /* TODO: return error if buffer is corrupt */;
        LOG_ERROR("Corrupt buffer");
    }
    inflateEnd(&zs);
}

using namespace clienttelemetry::data::v3;

void AriaDecoderV3::decode(std::vector<uint8_t> &in, std::vector<uint8_t> &out, bool compressed)
{
    if (compressed)
    {
        InflateVector(in, out);
    }
    else
    {
        std::copy(in.begin(), in.end(), std::back_inserter(out));
    }

    // TODO: read record-by-record using '3' and '.' as delimiter
#if 0
    ClientToCollectorRequest record;
    {
        CompactBinaryProtocolReader reader(out);
        bool result = Deserialize(reader, record, false);
        if (!result)
            LOG_ERROR("Deserialization failed!");
        json j;
        to_json(j, record);
        printf("%s\n", j.dump(2).c_str());
    }
#endif

}
