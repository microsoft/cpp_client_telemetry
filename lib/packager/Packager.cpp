//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "Packager.hpp"
#include "ILogManager.hpp"
#include "utils/StringUtils.hpp"
#include <algorithm>

namespace MAT_NS_BEGIN {

    Packager::Packager(IRuntimeConfig& runtimeConfig)
        : m_config(runtimeConfig)
    {
        const char *forcedTenantToken = runtimeConfig["forcedTenantToken"];
        if (forcedTenantToken != nullptr)
        {
            m_forcedTenantToken = forcedTenantToken;
        }
    }

    void Packager::handleAddEventToPackage(EventsUploadContextPtr const& ctx, StorageRecord const& record, bool& wantMore)
    {
        try {
            if (ctx->maxUploadSize == 0) {
                ctx->maxUploadSize = m_config.GetMaximumUploadSizeBytes();
            }
            if (ctx->splicer->getSizeEstimate() + record.blob.size() > ctx->maxUploadSize) {
                wantMore = false;
                if (!ctx->recordIdsAndTenantIds.empty()) {
                    LOG_TRACE("Maximum upload size %u bytes exceeded, not adding the next event (ID %s, size %u bytes)",
                        ctx->maxUploadSize, record.id.c_str(), static_cast<unsigned>(record.blob.size()));
                    return;
                }
                else {
                    LOG_INFO("Maximum upload size %u bytes exceeded by the first event",
                        ctx->maxUploadSize);
                }
            }

            if (ctx->latency == EventLatency_Unspecified) {
                ctx->latency = record.latency;
                LOG_TRACE("The highest latency found was %d (%s)",
                    ctx->latency, latencyToStr(ctx->latency));
            }

            LOG_TRACE("Adding event %s:%s, size %u bytes",
                tenantTokenToId(record.tenantToken).c_str(), record.id.c_str(), static_cast<unsigned>(record.blob.size()));

            #ifdef HAVE_MAT_EVT_TRACEID
                        ctx->traceId = record.traceId;
            #endif // HAVE_MAT_EVT_TRACEID

            std::string const& tenantToken = m_forcedTenantToken.empty() ? record.tenantToken : m_forcedTenantToken;
            auto it = ctx->packageIds.lower_bound(tenantToken);
            if (it == ctx->packageIds.end() || it->first != tenantToken)
            {
                it = ctx->packageIds.insert(it, { tenantToken, ctx->splicer->addTenantToken(tenantToken) });
            }

            ctx->splicer->addRecord(it->second, record.blob);

            ctx->recordIdsAndTenantIds[record.id] = record.tenantToken;
            ctx->recordTimestamps.push_back(record.timestamp);
            ctx->maxRetryCountSeen = std::max<int>(ctx->maxRetryCountSeen, record.retryCount);
        }
        catch (const std::bad_alloc&) {
            wantMore = false;
            LOG_ERROR("Failed to add new record to package: record.blob.size=%zu", record.blob.size());
        }
    }

    void Packager::handleFinalizePackage(EventsUploadContextPtr const& ctx)
    {
        if (ctx->packageIds.empty()) {
            emptyPackage(ctx);
            return;
        }

        ctx->body = ctx->splicer->splice();
        ctx->splicer->clear();

        packagedEvents(ctx);
    }


} MAT_NS_END

