// Copyright (c) Microsoft. All rights reserved.

#include "Statistics.hpp"
#include "LogManager.hpp"
#include "utils/Utils.hpp"
#include <oacr.h>

namespace ARIASDK_NS_BEGIN {
        

Statistics::Statistics(IRuntimeConfig& runtimeConfig, ContextFieldsProvider const& globalContext, ITelemetrySystem* telemetrySystem)
  : m_metaStats(runtimeConfig, globalContext),
    m_runtimeConfig(runtimeConfig),
    m_isStarted(false),
    m_iTelemetrySystem(telemetrySystem)
{
}

Statistics::~Statistics()
{
}

void Statistics::scheduleSend()
{
    if (!m_isStarted) {
        return;
    }

    unsigned intervalMs = m_runtimeConfig.GetMetaStatsSendIntervalSec() * 1000;
    if (nullptr == m_iTelemetrySystem)
    {
        if (!m_isScheduled && intervalMs != 0)
        {
            m_isScheduled = true;
            m_scheduledSend = PAL::scheduleOnWorkerThread(intervalMs, self(), &Statistics::send, ACT_STATS_ROLLUP_KIND_ONGOING);
            ARIASDK_LOG_DETAIL("Ongoing stats event generation scheduled in %u msec", intervalMs);
        }
    }
    else
    { //Utc does not schedule stats
        std::int64_t timedelta = PAL::getMonotonicTimeMs() - m_statEventSentTime;
        if (!m_isScheduled && intervalMs != 0 && timedelta > intervalMs)
        {
            m_isScheduled = true;
            send(ACT_STATS_ROLLUP_KIND_ONGOING);
            ARIASDK_LOG_DETAIL("Ongoing stats event generation scheduled in %u msec", intervalMs);
        }
    }
}

void Statistics::send(ActRollUpKind rollupKind)
{
    m_isScheduled = false;

    std::vector< ::AriaProtocol::CsEvent> records = m_metaStats.generateStatsEvent(rollupKind);
    std::string tenantToken = m_runtimeConfig.GetMetaStatsTenantToken();

    for (auto& record : records)
    {
        IncomingEventContextPtr event = IncomingEventContext::create(PAL::generateUuidString(), tenantToken, EventLatency_RealTime, EventPersistence_Critical, &record);
        if (nullptr == m_iTelemetrySystem)
        {
            eventGenerated(event);			
        }
        else
        {   //Utc Stats go back to Utc
            event->policyBitFlags = MICROSOFT_EVENTTAG_REALTIME_LATENCY | MICROSOFT_KEYWORD_CRITICAL_DATA;
            m_iTelemetrySystem->addIncomingEventSystem(event);			
        }
    }
    m_statEventSentTime = PAL::getMonotonicTimeMs();
}

bool Statistics::handleOnStart()
{
    send(ACT_STATS_ROLLUP_KIND_START);

    m_isStarted = true;
    return true;
}

bool Statistics::handleOnStop()
{
    m_isStarted = false;

    if (m_isScheduled) {
        m_scheduledSend.cancel();
        m_isScheduled = false;
    }

    send(ACT_STATS_ROLLUP_KIND_STOP);
    return true;
}

bool Statistics::handleOnIncomingEventAccepted(IncomingEventContextPtr const& ctx)
{
    bool metastats = (ctx->record.tenantToken == m_runtimeConfig.GetMetaStatsTenantToken());
    m_metaStats.updateOnEventIncoming(static_cast<unsigned>(ctx->record.blob.size()), ctx->record.latency, metastats);
    scheduleSend();
    
    LogManager::DispatchEvent(DebugEventType::EVT_ADDED);
    return true;
}

bool Statistics::handleOnIncomingEventFailed(IncomingEventContextPtr const& ctx)
{
    UNREFERENCED_PARAMETER(ctx);
    m_metaStats.updateOnRecordsDropped(DROPPED_REASON_OFFLINE_STORAGE_SAVE_FAILED, 1);
    scheduleSend();

    LogManager::DispatchEvent(DebugEventType::EVT_DROPPED);
    return true;
}

bool Statistics::handleOnUploadStarted(EventsUploadContextPtr const& ctx)
{
    bool metastatsOnly = (ctx->packageIds.count(m_runtimeConfig.GetMetaStatsTenantToken()) == ctx->packageIds.size());
    m_metaStats.updateOnPostData(static_cast<unsigned>(ctx->httpRequest->GetSizeEstimate()), metastatsOnly);
    scheduleSend();
    return true;
}

bool Statistics::handleOnPackagingFailed(EventsUploadContextPtr const& ctx)
{
    UNREFERENCED_PARAMETER(ctx);
    OACR_USE_PTR(this);
    return true;
}

bool Statistics::handleOnUploadSuccessful(EventsUploadContextPtr const& ctx)
{
    int64_t now = PAL::getUtcSystemTimeMs();
    std::vector<unsigned> latencyToSendMs;
    latencyToSendMs.reserve(ctx->recordTimestamps.size());
    for (int64_t ts : ctx->recordTimestamps) {
        latencyToSendMs.push_back(static_cast<unsigned>(std::max<int64_t>(0, std::min<int64_t>(0xFFFFFFFFu, now - ts))));
    }

    bool metastatsOnly = (ctx->packageIds.count(m_runtimeConfig.GetMetaStatsTenantToken()) == ctx->packageIds.size());
    m_metaStats.updateOnPackageSentSucceeded(ctx->latency, ctx->maxRetryCountSeen, ctx->durationMs, latencyToSendMs, metastatsOnly);
    scheduleSend();
    return true;
}

bool Statistics::handleOnUploadRejected(EventsUploadContextPtr const& ctx)
{
    unsigned status = ctx->httpResponse->GetStatusCode();
    m_metaStats.updateOnPackageFailed(status);

    auto reason = (status >= 400 && status < 500) ? DROPPED_REASON_SERVER_DECLINED_4XX
        : (status >= 500 && status < 600) ? DROPPED_REASON_SERVER_DECLINED_5XX
        : DROPPED_REASON_SERVER_DECLINED_OTHER;
    m_metaStats.updateOnRecordsDropped(reason, static_cast<unsigned>(ctx->recordIds.size()));

    scheduleSend();
    return true;
}

bool Statistics::handleOnUploadFailed(EventsUploadContextPtr const& ctx)
{
    m_metaStats.updateOnPackageRetry(ctx->httpResponse->GetStatusCode(), ctx->maxRetryCountSeen);
    scheduleSend();
    return true;
}

bool Statistics::handleOnStorageOpened(StorageNotificationContext const* ctx)
{
    m_metaStats.updateOnStorageOpened(ctx->str);
    return true;
}

bool Statistics::handleOnStorageFailed(StorageNotificationContext const* ctx)
{
    m_metaStats.updateOnStorageFailed(ctx->str);
    return true;
}

bool Statistics::handleOnStorageTrimmed(StorageNotificationContext const* ctx)
{
    m_metaStats.updateOnRecordsDropped(DROPPED_REASON_OFFLINE_STORAGE_OVERFLOW, ctx->count);
    scheduleSend();
    return true;
}

bool Statistics::handleOnStorageRecordsDropped(StorageNotificationContext const* ctx)
{
    m_metaStats.updateOnRecordsDropped(DROPPED_REASON_RETRY_EXCEEDED, ctx->count);
    scheduleSend();
    return true;
}


} ARIASDK_NS_END
