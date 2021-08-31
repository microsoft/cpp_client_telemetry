//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

#include "PayloadDecoder.hpp"

#if !defined(HAVE_MAT_ZLIB) || !defined(HAVE_MAT_JSONHPP)

/* PayloadDecoder functionality requires ZLib and json.hpp.
 * If these components are not included in the build, then
 * replace decoder utility functions with stubs that return
 * false.
 */
namespace MAT_NS_BEGIN
{
    namespace exporters
    {
        bool DecodeRecord(const CsProtocol::Record&, std::string&)
        {
            return false;
        }

        bool DecodeRequest(const std::vector<uint8_t>&, std::string&, bool)
        {
            return false;
        }
    };
}
MAT_NS_END

#else
#include <algorithm>
#include <chrono>
#include <fstream>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifndef TEST_LOG_ERROR
#define TEST_LOG_ERROR(arg0, ...)     fprintf(stderr, arg0 "\n", ##__VA_ARGS__)
#endif

/* PayloadDecoder functionality requires json.hpp library */
#include "json.hpp"

/* Bond definition of CsProtocol::Record is auto-generated and could be different for each SDK version */
#include "bond/All.hpp"
#include "CsProtocol_types.hpp"
#include "bond/generated/CsProtocol_readers.hpp"
#include "utils/ZlibUtils.hpp"

#include "zlib.h"
#undef compress

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
                                if (test[j + 1] == ('0'+::CsProtocol::CS_VER_MAJOR) && test[j + 2] == '.')
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

            bool to_json(json& out, const std::vector<uint8_t>& request)
            {
                auto records = decodeRequest(request);
                if (records.size() == 0)
                {
                    return false;
                }
                for (const auto &r : records)
                {
                    json j;
                    to_json(j, r);
                    out.push_back(j);
                }
                return true;
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

            void to_json(json& j, const Record& r)
            {

                j = json
                {
                    { "ver",        r.ver },        // 1: required string ver
                    { "name",       r.name },       // 2: required string name
                    { "time",       r.time },       // 3: required int64 time
                    { "popSample",  r.popSample },  // 4: optional double popSample
                    { "iKey",       r.iKey },       // 5: optional string iKey
                    { "flags",      r.flags },      // 6: optional int64 flags
                    { "cV",         r.cV },         // 7: optional string cV
                    { "ext",
                        {
                    /*
                                                { "ingest", r.extIngest },                           // 20: optional vector<Ingest> extIngest
                     */
                                                { "protocol",                                        // 21: optional vector<Protocol> extProtocol
                                                    {
                                                        { "metadataCrc", r.extProtocol[0].metadataCrc },
                                                        { "ticketKeys",  r.extProtocol[0].ticketKeys },
                                                        { "devMake",     r.extProtocol[0].devMake },
                                                        { "devModel",    r.extProtocol[0].devModel }
#ifdef HAVE_CS4
                                                        ,
                                                        { "msp",         r.extProtocol[0].msp }
#endif
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
                                                        { "model",       r.extDevice[0].model }
#ifdef HAVE_CS4
                                                        ,
                                                        { "authIdEnt",   r.extDevice[0].authIdEnt }
#endif
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
#ifdef HAVE_CS4
                                                        ,
                                                        { "sesId",   r.extApp[0].sesId }
#endif
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
#ifdef HAVE_CS4
                                                        { "ver",       r.extSdk[0].ver }
#else
                                                        { "libVer",    r.extSdk[0].libVer}
#endif
                                                     }
                                                }
                    /*
                                                { "loc",      r.extLoc },       // 33: optional vector<Loc> extLoc
                                                { "cloud",    r.extCloud }      // 34: optional vector<Cloud> extCloud
                     */
                                            }
                                        }
                };

#ifdef HAVE_CS4
                if (r.extM365a.size())
                {
                    j["ext"]["m365"] = json
                    {
                        {"enrolledTenantId", r.extM365a[0].enrolledTenantId},
                        {"msp", r.extM365a[0].msp }
                    };
                }
#endif

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
                {
                    return false;
                }

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
                {
                    TEST_LOG_ERROR("Invalid input buffer!");
                    return false;
                }

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
                    {
                        reserved += sizeof(uint32_t);
                    }
                    destLen = s32;
                }

                // Allocate memory for the new uncompressed buffer
                if (destLen > 0)
                {
                    try
                    {
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
                    catch (std::bad_alloc&)
                    {
                        TEST_LOG_ERROR("Decompression failed (out of memory): destLen=%zu", destLen);
                        dest = NULL;
                        destLen = 0;
                    }
                }

                // OOM
                TEST_LOG_ERROR("Unable to allocate memory for uncompressed buffer!");
                return false;
            }


            bool ExpandVector(std::vector<uint8_t>& in, std::vector<uint8_t>& out)
            {
                bool result = true;
                size_t destLen = out.size();
                char *buffer = nullptr;

                result = Expand((const char*)(in.data()), in.size(), &buffer, destLen, false);
                if (result)
                {
                    if (buffer)
                    {
                        out = std::vector<uint8_t>(buffer, buffer + destLen);
                        delete[] buffer;
                    }
                }

                return result;
            }
        }
    }
}

using namespace clienttelemetry::data::v3;

namespace MAT_NS_BEGIN {

    namespace exporters {

        /// <summary>
        /// Decodes the request from binary into human-readable format.
        /// </summary>
        /// <param name="in">Input request buffer containing HTTP request body</param>
        /// <param name="out">Event payload in a human-readable format, e.g. JSON</param>
        /// <param name="compressed">If set to <c>true</c> then the input buffer is [compressed] (optional)</param>
        bool DecodeRequest(const std::vector<uint8_t>& in, std::string& out, bool compressed)
        {
            out.clear();

            std::vector<uint8_t> buffer;
            if (compressed)
            {
                if (!ZlibUtils::InflateVector(in, buffer, false /* isGzip */))
                {
                    TEST_LOG_ERROR("Failed to inflate compressed data");
                    return false;
                }
            }
            else
            {
                std::copy(in.begin(), in.end(), std::back_inserter(buffer));
            }

            bool result = true;
            json j = json::array();
            result = to_json(j, buffer);

            if (result)
            {
                out = j.dump(2);
            }

            return result;
        }

        /// <summary>
        /// Decodes the record contents from binary into human-readable format.
        /// </summary>
        /// <param name="in">Input record containing CsProtocol Record</param>
        /// <param name="out">Event payload in a human-readable format, e.g. JSON</param>
        bool DecodeRecord(const CsProtocol::Record& in, std::string& out)
        {
            out.clear();

            nlohmann::json j;
            to_json(j, in);
            std::string s = j.dump(4);
            out.assign(s.begin(), s.end());

            return true;
        }

    }

} MAT_NS_END
#endif
