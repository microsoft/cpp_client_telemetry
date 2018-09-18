// Copyright (c) Microsoft. All rights reserved.

#include "BondSerializer.hpp"
#include "utils/Utils.hpp"
#include <bond_lite/All.hpp>
#include "bond/generated/AriaProtocol_writers.hpp"
#include "bond/generated/AriaProtocol_readers.hpp"
#include <oacr.h>

#include "json.hpp"
using nlohmann::json;

namespace ARIASDK_NS_BEGIN {

#if 0
    ::json to_json(IncomingEventContextPtr const& ctx)
    {
        auto &record = (*ctx->source);

        json obj =
        {
            { "ver",        record.ver },
            { "name",       record.name },
            { "time",       record.time },
            { "popSample",  record.popSample },
            { "iKey",       record.iKey },
            { "flags",      record.flags },
            { "cV",         record.cV },
            { "ext",
                {
                    { "protocol",
                        {
                            {"devMake", record.extProtocol[0].devMake },
                            {"devModel", record.extProtocol[0].devModel }
                        }
                    },
                    { "user" ,
                        {
                            { "id", record.extUser[0].id },
                            { "localId", record.extUser[0].localId },
                            { "authId", record.extUser[0].authId },
                            { "locale", record.extUser[0].locale }
                        }
                    },
                    { "device",
                         {
                            { "authId",      record.extDevice[0].authId },
                            { "authSecId",   record.extDevice[0].authSecId },
                            { "deviceClass", record.extDevice[0].deviceClass },
                            { "id",          record.extDevice[0].id },
                            { "localId",     record.extDevice[0].localId },
                            { "make",        record.extDevice[0].make },
                            { "model",       record.extDevice[0].model },
                         }
                    },
                    { "os",
                         {
                            { "bootId",  record.extOs[0].bootId },
                            { "expId",   record.extOs[0].expId },
                            { "locale",  record.extOs[0].locale },
                            { "name",    record.extOs[0].name },
                            { "ver",     record.extOs[0].ver }
                         }
                    },
                    { "net",
                         {
                            { "cost",     record.extNet[0].cost },
                            { "provider", record.extNet[0].provider },
                            { "type",     record.extNet[0].type }
                         }
                    },
                    { "sdk",
                         {
                            { "epoch",     record.extSdk[0].epoch },
                            { "installId", record.extSdk[0].installId },
                            { "libVer",    record.extSdk[0].libVer}
                         }
                    },
                }
            }
        };

        obj["data"] = json({});
        for (const auto &v : record.data)
        {
            for (const auto &kv : v.properties)
            {
                std::string val = "";
                switch (kv.second.type)
                {
                case ::AriaProtocol::ValueKind::ValueInt64:
                    val = std::to_string((int64_t)kv.second.longValue);
                    break;
                case ::AriaProtocol::ValueKind::ValueUInt64:
                    val = std::to_string((uint64_t)kv.second.longValue);
                    break;
                case ::AriaProtocol::ValueKind::ValueInt32:
                    val = std::to_string((int32_t)kv.second.longValue);
                    break;
                case ::AriaProtocol::ValueKind::ValueUInt32:
                    val = std::to_string((uint32_t)kv.second.longValue);
                    break;
                case ::AriaProtocol::ValueKind::ValueDouble:
                    val = std::to_string(kv.second.doubleValue);
                    break;
                case ::AriaProtocol::ValueKind::ValueString:
                    val = kv.second.stringValue;
                    break;
                case ::AriaProtocol::ValueKind::ValueBool:
                    val = (kv.second.longValue) ? true : false;
                    break;
                case ::AriaProtocol::ValueKind::ValueDateTime:
                    // FIXME: [MG] - convert to time
                    val = std::to_string((int64_t)kv.second.longValue);
                    break;
                case ::AriaProtocol::ValueKind::ValueGuid:
                    // TODO: [MG] - not implemented
                    break;
                case ::AriaProtocol::ValueKind::ValueArrayInt64:
                    // TODO: [MG] - not implemented
                    break;
                case ::AriaProtocol::ValueKind::ValueArrayUInt64:
                    // TODO: [MG] - not implemented
                    break;
                case ::AriaProtocol::ValueKind::ValueArrayInt32:
                    // TODO: [MG] - not implemented
                    break;
                case ::AriaProtocol::ValueKind::ValueArrayUInt32:
                    // TODO: [MG] - not implemented
                    break;
                case ::AriaProtocol::ValueKind::ValueArrayDouble:
                    // TODO: [MG] - not implemented
                    break;
                case ::AriaProtocol::ValueKind::ValueArrayString:
                    // TODO: [MG] - not implemented
                    break;
                case ::AriaProtocol::ValueKind::ValueArrayBool:
                    // TODO: [MG] - not implemented
                    break;
                case ::AriaProtocol::ValueKind::ValueArrayDateTime:
                    // TODO: [MG] - not implemented
                    break;
                case ::AriaProtocol::ValueKind::ValueArrayGuid:
                    // TODO: [MG] - not implemented
                    break;
                default:
                    break;
                }
                obj["data"][kv.first] = "";
            }
        }

        LOG_INFO("%s", obj.dump().c_str() );
        return obj;
    }
#endif

    bool BondSerializer::handleSerialize(IncomingEventContextPtr const& ctx)
    {
        OACR_USE_PTR(this);

        // XXX: [MG] - debug only
        // to_json(ctx);

        {
            bond_lite::CompactBinaryProtocolWriter writer(ctx->record.blob);
            bond_lite::Serialize(writer, *ctx->source);
        }

        //    AriaProtocol::Record result;
        //    std::vector<uint8_t> input(ctx->record.blob.data(), ctx->record.blob.data() + ctx->record.blob.size());
        //    bond_lite::CompactBinaryProtocolReader reader(input);
        //    bond_lite::Deserialize(reader, result);


         //   AriaProtocol::Record r;
         //   bond_lite::CompactBinaryProtocolReader reader(ctx->record.blob);
         //   bond_lite::Deserialize(reader, r);

        LOG_TRACE("Event %s/%s submitted, priority %u (%s), serialized size %u bytes, ID %s",
            tenantTokenToId(ctx->record.tenantToken).c_str(), ctx->source->baseType.c_str(),
            ctx->record.latency, latencyToStr(ctx->record.latency),
            static_cast<unsigned>(ctx->record.blob.size()), ctx->record.id.c_str());

        return true;
    }


} ARIASDK_NS_END
