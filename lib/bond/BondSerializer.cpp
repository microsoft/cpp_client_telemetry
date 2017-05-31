// Copyright (c) Microsoft. All rights reserved.

#include "BondSerializer.hpp"
#include "utils/Common.hpp"
#include <bond_lite/All.hpp>
#include "generated/AriaProtocol_writers.hpp"
#include <oacr.h>

namespace ARIASDK_NS_BEGIN {


bool BondSerializer::handleSerialize(IncomingEventContextPtr const& ctx)
{
    OACR_USE_PTR(this);

    {
        bond_lite::CompactBinaryProtocolWriter writer(ctx->record.blob);
        bond_lite::Serialize(writer, *ctx->source);
    }

    ARIASDK_LOG_INFO("Event %s/%s submitted, priority %u (%s), serialized size %u bytes, ID %s",
        tenantTokenToId(ctx->record.tenantToken).c_str(), ctx->source->EventType.c_str(),
        ctx->record.priority, priorityToStr(ctx->record.priority),
        static_cast<unsigned>(ctx->record.blob.size()), ctx->record.id.c_str());

    return true;
}


} ARIASDK_NS_END
