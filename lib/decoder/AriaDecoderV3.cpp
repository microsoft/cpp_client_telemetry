#include "AriaDecoderV3Internal.hpp"

// TODO: move Compress and Expand into separate utilities module

#include <algorithm>
#include <chrono>
#include <fstream>

#ifndef TEST_LOG_ERROR
#define TEST_LOG_ERROR(arg0, ...)     fprintf(stderr, arg0 "\n", ##__VA_ARGS__)
#endif

#ifdef HAVE_MAT_JSONHPP
using namespace CsProtocol;
using json = nlohmann::json;

namespace clienttelemetry {
    namespace data {
        namespace v3 {

            void to_json(json& j, const Record& r);

            std::vector<Record> decodeRequest(const std::vector<uint8_t>& request)
            {
                std::vector<Record> v;
                size_t i = 0;
                size_t length = 0;
                while (i < request.size())
                {
                    Record result;
                    length = request.size() - i;
                    std::vector<uint8_t> test(request.data() + i, request.data() + i + length);
                    size_t j = 3;
                    bool found = false;
                    while (j < length)
                    {
                        while (j < length && test[j] != '\x3')
                        {
                            j++;
                        }
                        if (j < length)
                        {
                            if (j + 2 < length)
                            {
                                if (test[j + 1] == '3' && test[j + 2] == '.')
                                {
                                    found = true;
                                    break;
                                }
                            }
                            j++;
                        }
                    }
                    if (!found)
                    {
                        j += 1;
                    }
                    std::vector<uint8_t> input(request.data() + i, request.data() + i + j - 1);
                    bond_lite::CompactBinaryProtocolReader reader(input);

                    if (!Deserialize(reader, result, false))
                    {
                        TEST_LOG_ERROR("Deserialization failed!");
                        goto fail;
                    }
                    i += j - 1;
                    v.push_back(result);
                }
            fail:
                return v;
            }

            void to_json(json& out, const std::vector<uint8_t>& request) {
                auto records = decodeRequest(request);
                for (const auto &r : records)
                {
                    json j;
                    to_json(j, r);
                    out.push_back(j);
                }
            }

            void to_json(json& j, const Data& d)
            {
                for (const auto &kv : d.properties)
                {
                    const auto k = kv.first;
                    const auto v = kv.second;

                    if (kv.second.attributes.size())
                    {
                        /* C# bond decoder uses more complex notation:
                        ...
                        "piiKind.DistinguishedName",
                        {
                            "attributes": [{
                                "pii": [{
                                    "Kind": 1
                                }]
                            }],
                            "stringValue": "/CN=Jack Frost,OU=ARIA,DC=REDMOND,DC=COM"
                        }
                        ...

                        While C++ decoder uses simple compact notation as follows:
                        ...
                        "piiKind.DistinguishedName",
                        {
                            "pii": 1,
                            "stringValue": "/CN=Jack Frost,OU=ARIA,DC=REDMOND,DC=COM"
                        }
                        */

                        auto kind = kv.second.attributes[0].pii[0].Kind;
                        j[k] = {
                            { "stringValue", v.stringValue },
                            { "pii", (unsigned)kind }
                        };
                        continue;
                    };

                    switch (kv.second.type)
                    {
                    case ::CsProtocol::ValueKind::ValueInt64:
                        j[k] = (int64_t)v.longValue;
                        break;
                    case ::CsProtocol::ValueKind::ValueUInt64:
                        j[k] = (uint64_t)v.longValue;
                        break;
                    case ::CsProtocol::ValueKind::ValueInt32:
                        j[k] = (int32_t)v.longValue;
                        break;
                    case ::CsProtocol::ValueKind::ValueUInt32:
                        j[k] = (uint32_t)v.longValue;
                        break;
                    case ::CsProtocol::ValueKind::ValueDouble:
                        j[k] = v.doubleValue;
                        break;
                    case ::CsProtocol::ValueKind::ValueString:
                        j[k] = v.stringValue;
                        break;
                    case ::CsProtocol::ValueKind::ValueBool:
                        j[k] = (v.longValue > 0) ? true : false;
                        break;
                    case ::CsProtocol::ValueKind::ValueDateTime:
                        j[k] = (int64_t)kv.second.longValue;
                        break;
                    case ::CsProtocol::ValueKind::ValueGuid:
                        j[k] = kv.second.guidValue;
                        break;
                    case ::CsProtocol::ValueKind::ValueArrayInt64:
                        j[k] = kv.second.longArray;
                        break;
                    case ::CsProtocol::ValueKind::ValueArrayUInt64:
                        j[k] = kv.second.longArray;
                        break;
                    case ::CsProtocol::ValueKind::ValueArrayInt32:
                        j[k] = kv.second.longArray;
                        break;
                    case ::CsProtocol::ValueKind::ValueArrayUInt32:
                        j[k] = kv.second.longArray;
                        break;
                    case ::CsProtocol::ValueKind::ValueArrayDouble:
                        j[k] = kv.second.doubleArray;
                        break;
                    case ::CsProtocol::ValueKind::ValueArrayString:
                        j[k] = kv.second.stringArray;
                        break;
                    case ::CsProtocol::ValueKind::ValueArrayBool:
                        j[k] = kv.second.longArray;
                        break;
                    case ::CsProtocol::ValueKind::ValueArrayDateTime:
                        j[k] = kv.second.longArray;
                        break;
                    case ::CsProtocol::ValueKind::ValueArrayGuid:
                        j[k] = kv.second.guidArray;
                        break;
                    default:
                        break;
                    }
                }
            }

            void to_json(json& j, const Record& r) {

                j = json{
                    { "ver",        r.ver },        // 1: required string ver
                    { "name",       r.name },       // 2: required string name
                    { "time",       r.time },       // 3: required int64 time
                    { "popSample",  r.popSample },  // 4: optional double popSample
                    { "iKey",       r.iKey },       // 5: optional string iKey
                    { "flags",      r.flags },      // 6: optional int64 flags
                    { "cV",         r.cV },         // 7: optional string cV
                    { "ext", {
                    /*
                                                { "ingest", r.extIngest },                           // 20: optional vector<Ingest> extIngest
                     */
                                                { "protocol",                                        // 21: optional vector<Protocol> extProtocol
                                                    {
                                                        { "metadataCrc", r.extProtocol[0].metadataCrc },
                                                        { "ticketKeys",  r.extProtocol[0].ticketKeys },
                                                        { "devMake",     r.extProtocol[0].devMake },
                                                        { "devModel",    r.extProtocol[0].devModel }
                                                    }
                                                },
                                                { "user" ,                                           // 22: optional vector<User> extUser
                                                    {
                                                        { "id",      r.extUser[0].id },
                                                        { "localId", r.extUser[0].localId },
                                                        { "authId",  r.extUser[0].authId },
                                                        { "locale",  r.extUser[0].locale }
                                                    }
                                                },
                                                { "device",                                          // 23: optional vector<Device> extDevice
                                                     {
                                                        { "authId",      r.extDevice[0].authId },
                                                        { "authSecId",   r.extDevice[0].authSecId },
                                                        { "deviceClass", r.extDevice[0].deviceClass },
                                                        { "id",          r.extDevice[0].id },
                                                        { "localId",     r.extDevice[0].localId },
                                                        { "make",        r.extDevice[0].make },
                                                        { "model",       r.extDevice[0].model },
                                                     }
                                                },
                                                { "os",                                              // 24: optional vector<Os> extOs
                                                     {
                                                        { "bootId",  r.extOs[0].bootId },
                                                        { "expId",   r.extOs[0].expId },
                                                        { "locale",  r.extOs[0].locale },
                                                        { "name",    r.extOs[0].name },
                                                        { "ver",     r.extOs[0].ver }
                                                     }
                                                },
                                                { "app",                                             // 25: optional vector<App> extApp
                                                     {
                                                        { "expId",   r.extApp[0].expId },
                                                        { "userId",  r.extApp[0].userId },
                                                        { "env",     r.extApp[0].env },
                                                        { "asId",    r.extApp[0].asId },
                                                        { "id",      r.extApp[0].id },
                                                        { "ver",     r.extApp[0].ver },
                                                        { "locale",  r.extApp[0].locale },
                                                        { "name",    r.extApp[0].name }
                                                     }
                                                },
                    /*
                                                { "utc",      r.extUtc },       // 26: optional vector<Utc> extUtc
                                                { "xbl",      r.extXbl },       // 27: optional vector<Xbl> extXbl
                                                { "js",       r.extJavascript },// 28: optional vector<Javascript> extJavascript
                                                { "receipts", r.extReceipts },  // 29: optional vector<Receipts> extReceipts
                     */
                                                { "net",
                                                     {
                                                        { "cost",     r.extNet[0].cost },
                                                        { "provider", r.extNet[0].provider },
                                                        { "type",     r.extNet[0].type }
                                                     }
                                                },
                                                { "sdk",
                                                     {
                                                        { "epoch",     r.extSdk[0].epoch },
                                                        { "installId", r.extSdk[0].installId },
                                                        { "libVer",    r.extSdk[0].libVer}
                                                     }
                                                }
                    /*
                                                { "loc",      r.extLoc },       // 33: optional vector<Loc> extLoc
                                                { "cloud",    r.extCloud }      // 34: optional vector<Cloud> extCloud
                     */
                                            }
                                        }
                };

                if (r.ext.size())
                {
                    j["extData"] = json({});
                    to_json(j["extData"], r.ext[0]);
                }

                j["tags"] = r.tags;
                j["baseType"] = r.baseType;

                if (r.baseData.size())
                {
                    j["baseData"] = json({});
                    to_json(j["baseData"], r.baseData[0]);
                }

                if (r.data.size())
                {
                    j["data"] = json({});
                    to_json(j["data"], r.data[0]);
                }
            }

        }
    }
}
#endif // HAVE_MAT_JSONHPP

#if 0   /* These routines should exist in scope of FuncTests and UnitTests project provided by SDK utils */

#ifndef MAX_GUID_LEN
#define MAX_GUID_LEN    36  /* maximum GUIDv4 length with hyphens, but without curly braces */
#endif

#ifndef SNPRINTF
#define SNPRINTF        snprintf
#endif

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

#ifdef _WIN32
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
        if (rc < 0)
        {
            TEST_LOG_ERROR("SNPRINTF failed!");
            return "00000000-0000-0000-0000-000000000000";
        }
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
#endif
}
#endif

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
            TEST_LOG_ERROR("Compression failed, error=%u", res);
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
                    TEST_LOG_ERROR("Decompression failed, error=%d, len=%u, destLen=%u", res, static_cast<unsigned int>(len), static_cast<unsigned int>(destLen));
                    delete[] decompBody;
                    return false;
                }
                *dest = decompBody;
                destLen = len;
                return true;
            }
        }
        catch (std::bad_alloc&) {
            TEST_LOG_ERROR("Decompression failed (out of memory): destLen=%zu", destLen);
            dest = NULL;
            destLen = 0;
        }
    }

    // OOM
    return false;
}

namespace AriaDecoderV3 {

void ExpandVector(std::vector<uint8_t> &in, std::vector<uint8_t> &out)
{
    size_t destLen = out.size();
    std::cout << "size=" << destLen << std::endl;
    char *buffer = nullptr;
    Expand((const char*)(in.data()), in.size(), &buffer, destLen, false); // TODO: Check if true
    out = std::vector<uint8_t>(buffer, buffer + destLen);
    if (buffer)
        delete[] buffer;

}

void InflateVector(std::vector<uint8_t> &in, std::vector<uint8_t> &out)
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
    // The problem with 32K is that it's too small and causes corruption
    // in zlib inflate. 128KB seems to be fine.
    // Allocate a buffer enough to hold an output with Zlib max compression
    // ratio 5:1 in case it is larger than 128KB.
    uInt outbufferSize = std::max((uInt)131072, zs.avail_in * 5);

    char* outbuffer = new char[outbufferSize];
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = outbufferSize;
        ret = inflate(&zs, Z_NO_FLUSH);
        out.insert(out.end(), outbuffer, outbuffer + zs.total_out);
    } while (ret == Z_OK);
    if (ret != Z_STREAM_END)
    {
        /* TODO: return error if buffer is corrupt */;
        TEST_LOG_ERROR("Unable to successfully decompress into buffer");
    }
    inflateEnd(&zs);
    delete[] outbuffer;
}

void decode(std::vector<uint8_t> &in, std::vector<uint8_t> &out, bool compressed)
{
#ifdef HAVE_MAT_JSONHPP

    using namespace clienttelemetry::data::v3;

    if (compressed)
    {
        InflateVector(in, out);
    }
    else
    {
        std::copy(in.begin(), in.end(), std::back_inserter(out));
    }

    json j = json::array();
    clienttelemetry::data::v3::to_json(j, out);
    std::string s = j.dump(2);
    out.clear();
    std::copy(s.begin(), s.end(), std::back_inserter(out));

#else

    (void) (in);
    (void) (out);
    (void) (compressed);
    assert(false /* json.hpp support is not enabled! */);

#endif // HAVE_MAT_JSONHPP
}

#ifdef HAVE_MAT_JSONHPP
void to_json(nlohmann::json& j, const CsProtocol::Record& r)
{
    clienttelemetry::data::v3::to_json(j, r);
}
#endif

}  // namespace AriaDecoderV3
