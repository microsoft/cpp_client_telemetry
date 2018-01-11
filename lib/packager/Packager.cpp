// Copyright (c) Microsoft. All rights reserved.

#include "Packager.hpp"
#include "api/CommonLogManagerInternal.hpp"
#include "utils/Utils.hpp"
#include "IRuntimeConfig.hpp"
#include <algorithm>

namespace ARIASDK_NS_BEGIN {

Packager::Packager(LogConfiguration& configuration, IRuntimeConfig const& runtimeConfig)
  : m_runtimeConfig(runtimeConfig)    
{  
    UNREFERENCED_PARAMETER(configuration);
    EVTStatus error;
    m_forcedTenantToken = toLower(configuration.GetProperty("forcedTenantToken", error));   
}

Packager::~Packager()
{
}

void Packager::handleAddEventToPackage(EventsUploadContextPtr const& ctx, StorageRecord const& record, bool& wantMore)
{
    if (ctx->maxUploadSize == 0) {
        ctx->maxUploadSize = m_runtimeConfig.GetMaximumUploadSizeBytes();
    }
    if (ctx->splicer.getSizeEstimate() + record.blob.size() > ctx->maxUploadSize) {
        wantMore = false;
        if (!ctx->recordIdsAndTenantIds.empty()) {
            ARIASDK_LOG_DETAIL("Maximum upload size %u bytes exceeded, not adding the next event (ID %s, size %u bytes)",
                ctx->maxUploadSize, record.id.c_str(), static_cast<unsigned>(record.blob.size()));
            return;
        } else {
            ARIASDK_LOG_INFO("Maximum upload size %u bytes exceeded by the first event",
                ctx->maxUploadSize);
        }
    }

    if (ctx->latency == EventPriority_Unspecified) {
        ctx->latency = record.latency;
        ARIASDK_LOG_DETAIL("The highest latency found was %d (%s)",
            ctx->latency, latencyToStr(ctx->latency));
    }

    ARIASDK_LOG_DETAIL("Adding event %s:%s, size %u bytes",
        tenantTokenToId(record.tenantToken).c_str(), record.id.c_str(), static_cast<unsigned>(record.blob.size()));

    std::string const& tenantToken = m_forcedTenantToken.empty() ? record.tenantToken : m_forcedTenantToken;
    auto it = ctx->packageIds.lower_bound(tenantToken);
    if (it == ctx->packageIds.end() || it->first != tenantToken)
    {
        DataPackage package;
        package.Type          = "Client";
        package.Source        = "act_default_source"; // from ReferenceSDK
        package.Version       = BUILD_VERSION_STR;
        package.DataPackageId = PAL::generateUuidString();
        package.Timestamp     = PAL::getUtcSystemTimeMs();
        it = ctx->packageIds.insert(it, {tenantToken, ctx->splicer.addDataPackage(tenantToken, package)});
    }

    ctx->splicer.addRecord(it->second, record.blob);

    ctx->recordIdsAndTenantIds[record.id] = record.tenantToken;
    ctx->recordTimestamps.push_back(record.timestamp);
    ctx->maxRetryCountSeen = std::max<int>(ctx->maxRetryCountSeen, record.retryCount);
}

void Packager::handleFinalizePackage(EventsUploadContextPtr const& ctx)
{
    if (ctx->packageIds.empty()) {
        emptyPackage(ctx);
        return;
    }

    ctx->body = ctx->splicer.splice();
    ctx->splicer.clear();

    packagedEvents(ctx);
    
    DebugEvent evt;
    evt.type = EVT_SENT;
    evt.param1 = ctx->recordIdsAndTenantIds.size();
    evt.size = ctx->recordIdsAndTenantIds.size();
    CommonLogManagerInternal::DispatchEvent(evt);
}


} ARIASDK_NS_END
