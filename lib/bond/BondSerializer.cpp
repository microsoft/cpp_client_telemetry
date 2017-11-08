// Copyright (c) Microsoft. All rights reserved.

#include "BondSerializer.hpp"
#include "utils/Utils.hpp"
#include <bond_lite/All.hpp>
#include "bond/generated/AriaProtocol_writers.hpp"
#include "bond/generated/AriaProtocol_readers.hpp"
#include <oacr.h>

namespace ARIASDK_NS_BEGIN {

bool BondSerializer::handleSerialize(IncomingEventContextPtr const& ctx)
{
    OACR_USE_PTR(this);

    {
        bond_lite::CompactBinaryProtocolWriter writer(ctx->record.blob);
        bond_lite::Serialize(writer, *ctx->source);
    }

    AriaProtocol::CsEvent result;
    std::vector<uint8_t> input(ctx->record.blob.data(), ctx->record.blob.data() + ctx->record.blob.size());
    bond_lite::CompactBinaryProtocolReader reader(input);
    bond_lite::Deserialize(reader, result);


 //   AriaProtocol::CsEvent r;
 //   bond_lite::CompactBinaryProtocolReader reader(ctx->record.blob);
 //   bond_lite::Deserialize(reader, r);

    ARIASDK_LOG_INFO("Event %s/%s submitted, priority %u (%s), serialized size %u bytes, ID %s",
        tenantTokenToId(ctx->record.tenantToken).c_str(), ctx->source->baseType.c_str(),
        ctx->record.latency, latencyToStr(ctx->record.latency),
        static_cast<unsigned>(ctx->record.blob.size()), ctx->record.id.c_str());

    return true;
}


} ARIASDK_NS_END
