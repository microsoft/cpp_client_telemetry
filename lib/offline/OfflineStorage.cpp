// Copyright (c) Microsoft. All rights reserved.

#include "OfflineStorage.hpp"
#include "api/CommonLogManagerInternal.hpp"

namespace ARIASDK_NS_BEGIN {

    
OfflineStorage::OfflineStorage(IOfflineStorage& offlineStorage)
  : m_offlineStorage(offlineStorage)
{
}

OfflineStorage::~OfflineStorage()
{
}

bool OfflineStorage::handleStart()
{
    m_offlineStorage.Initialize(*this);
    return true;
}

bool OfflineStorage::handleStop()
{
    m_offlineStorage.Shutdown();
    return true;
}

bool OfflineStorage::handleStoreRecord(IncomingEventContextPtr const& ctx)
{
    CommonLogManagerInternal::DispatchEvent(DebugEventType::EVT_CACHED);
    ctx->record.timestamp = PAL::getUtcSystemTimeMs();

    if (!m_offlineStorage.StoreRecord(ctx->record)) {
        storeRecordFailed(ctx);
        return false;
    }

    return true;
}

void OfflineStorage::handleRetrieveEvents(EventsUploadContextPtr const& ctx)
{
    auto consumer = [&ctx, this](StorageRecord&& record) -> bool {
            bool wantMore = true;
            retrievedEvent(ctx, std::move(record), wantMore);
            return wantMore;
        };

    if (!m_offlineStorage.GetAndReserveRecords(consumer, 120000, ctx->requestedMinLatency, ctx->requestedMaxCount))
	{
		ctx->fromMemory = m_offlineStorage.IsLastReadFromMemory();
        retrievalFailed(ctx);
    } 
	else
	{
		ctx->fromMemory = m_offlineStorage.IsLastReadFromMemory();
        retrievalFinished(ctx);
    }
}

bool OfflineStorage::handleDeleteRecords(EventsUploadContextPtr const& ctx)
{	
    HttpHeaders headers;
    if (ctx->httpResponse)
    {
        headers = ctx->httpResponse->GetHeaders();
    }
    std::vector<StorageRecordId> recordIds;
    for (auto item : ctx->recordIdsAndTenantIds)
    {
        recordIds.push_back(item.first);
    }
    m_offlineStorage.DeleteRecords(recordIds, headers, ctx->fromMemory);
    return true;
}

bool OfflineStorage::handleReleaseRecords(EventsUploadContextPtr const& ctx)
{
    HttpHeaders headers;
    if (ctx->httpResponse)
    {
        headers = ctx->httpResponse->GetHeaders();
    }
    std::vector<StorageRecordId> recordIds;
    for (auto item : ctx->recordIdsAndTenantIds)
    {
        recordIds.push_back(item.first);
    }
    m_offlineStorage.ReleaseRecords(recordIds, false, headers, ctx->fromMemory);
    return true;
}

bool OfflineStorage::handleReleaseRecordsIncRetryCount(EventsUploadContextPtr const& ctx)
{
    CommonLogManagerInternal::DispatchEvent(DebugEventType::EVT_SEND_RETRY);
    HttpHeaders headers;
    if (ctx->httpResponse)
    {
        headers = ctx->httpResponse->GetHeaders();
    }
    std::vector<StorageRecordId> recordIds;
    for (auto item : ctx->recordIdsAndTenantIds)
    {
        recordIds.push_back(item.first);
    }

    m_offlineStorage.ReleaseRecords(recordIds, true, headers, ctx->fromMemory);
    return true;
}

void OfflineStorage::OnStorageOpened(std::string const& type)
{
    StorageNotificationContext ctx;
    ctx.str = type;
    opened(&ctx);
}

void OfflineStorage::OnStorageFailed(std::string const& reason)
{   
    StorageNotificationContext ctx;
    ctx.str = reason;
    failed(&ctx);
}

void OfflineStorage::OnStorageTrimmed(std::map<std::string, size_t> const& numRecords)
{   
    StorageNotificationContext ctx;
   
    size_t overallCount = 0;
    for (auto records : numRecords)
    {
        ctx.countonTenant[records.first] = records.second;
        overallCount += records.second;
    }
    trimmed(&ctx);

    DebugEvent evt;
    evt.type = EVT_DROPPED;
    evt.param1 = overallCount;
    evt.size = overallCount;
    CommonLogManagerInternal::DispatchEvent(evt);
}

void OfflineStorage::OnStorageRecordsDropped(std::map<std::string, size_t> const& numRecords)
{
    StorageNotificationContext ctx;
    size_t overallCount = 0;
    for (auto records : numRecords)
    {
        ctx.countonTenant[records.first] = records.second;
        overallCount += records.second;
    }
    recordsDropped(&ctx);
   
    DebugEvent evt;
    evt.type = EVT_DROPPED;
    evt.param1 = overallCount;
    evt.size = overallCount;
    CommonLogManagerInternal::DispatchEvent(evt);
}

void OfflineStorage::OnStorageRecordsRejected(std::map<std::string, size_t> const& numRecords)
{
    StorageNotificationContext ctx;
    size_t overallCount = 0;
    for (auto records : numRecords)
    {
        ctx.countonTenant[records.first] = records.second;
        overallCount += records.second;
    }
    recordsRejected(&ctx);

    DebugEvent evt;
    evt.type = EVT_REJECTED;
    evt.param1 = overallCount;
    evt.size = overallCount;
    CommonLogManagerInternal::DispatchEvent(evt);
}

} ARIASDK_NS_END
