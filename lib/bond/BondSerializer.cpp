//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "BondSerializer.hpp"
#include "utils/StringUtils.hpp"
#include "utils/Utils.hpp"
#include "bond/All.hpp"
#include "bond/generated/CsProtocol_writers.hpp"
#include "bond/generated/CsProtocol_readers.hpp"
#include "oacr.h"

namespace MAT_NS_BEGIN {

    bool BondSerializer::handleSerialize(IncomingEventContextPtr const& ctx)
    {
        OACR_USE_PTR(this);
        {
            bond_lite::CompactBinaryProtocolWriter writer(ctx->record.blob);
            bond_lite::Serialize(writer, *ctx->source);
        }

        LOG_TRACE("Event %s/%s submitted, priority %u (%s), serialized size %u bytes, ID %s",
            tenantTokenToId(ctx->record.tenantToken).c_str(), ctx->source->baseType.c_str(),
            ctx->record.latency, latencyToStr(ctx->record.latency),
            static_cast<unsigned>(ctx->record.blob.size()), ctx->record.id.c_str());

        return true;
    }

} MAT_NS_END

