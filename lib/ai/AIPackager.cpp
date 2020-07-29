// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"
#ifdef HAVE_MAT_AI
#include "AIPackager.hpp"
#include "ILogManager.hpp"
#include "utils/Utils.hpp"
#include <algorithm>

namespace ARIASDK_NS_BEGIN {

    AIPackager::AIPackager(IRuntimeConfig& runtimeConfig)
        : m_config(runtimeConfig)
    {
        const char *forcedTenantToken = runtimeConfig["forcedTenantToken"];
        if (forcedTenantToken != nullptr)
        {
            m_forcedTenantToken = forcedTenantToken;
        }
    }

    void AIPackager::handleAddEventToPackage(EventsUploadContextPtr const& ctx, StorageRecord const& record, bool& wantMore)
    {
        try {
            if (ctx->maxUploadSize == 0) {
                ctx->maxUploadSize = m_config.GetMaximumUploadSizeBytes();
            }
            wantMore = false;
//            if (ctx->splicer.getSizeEstimate() + record.blob.size() > ctx->maxUploadSize) {
//                wantMore = false;
//                if (!ctx->recordIdsAndTenantIds.empty()) {
//                     LOG_TRACE("Maximum upload size %u bytes exceeded, not adding the next event (ID %s, size %u bytes)",
//                         ctx->maxUploadSize, record.id.c_str(), static_cast<unsigned>(record.blob.size()));
//                     return;
//                }
//                else {
//                     LOG_INFO("Maximum upload size %u bytes exceeded by the first event",
//                         ctx->maxUploadSize);
//                }
//            }

            if (ctx->latency == EventLatency_Unspecified) {
                ctx->latency = record.latency;
                LOG_TRACE("The highest latency found was %d (%s)",
                    ctx->latency, latencyToStr(ctx->latency));
            }

            LOG_TRACE("Adding event %s:%s, size %u bytes",
                tenantTokenToId(record.tenantToken).c_str(), record.id.c_str(), static_cast<unsigned>(record.blob.size()));

            std::string const& tenantToken = m_forcedTenantToken.empty() ? record.tenantToken : m_forcedTenantToken;
            auto it = ctx->packageIds.lower_bound(tenantToken);
            if (it == ctx->packageIds.end() || it->first != tenantToken)
            {
                it = ctx->packageIds.insert(it, { tenantToken, ctx->splicer.addTenantToken(tenantToken) });
            }

            ctx->body = record.blob;
//            ctx->splicer.addRecord(it->second, record.blob);

            ctx->recordIdsAndTenantIds[record.id] = record.tenantToken;
            ctx->recordTimestamps.push_back(record.timestamp);
            ctx->maxRetryCountSeen = std::max<int>(ctx->maxRetryCountSeen, record.retryCount);
            handleFinalizePackage(ctx);
        }
        catch (const std::bad_alloc&) {
            wantMore = false;
            LOG_ERROR("Failed to add new record to package: record.blob.size=%zu", record.blob.size());
        }
    }

    void AIPackager::handleFinalizePackage(EventsUploadContextPtr const& ctx)
    {
        if (ctx->body.empty()) {
            emptyPackage(ctx);
            return;
        }
        packagedEvents(ctx);
    }


} ARIASDK_NS_END
#endif
