// Copyright (c) Microsoft. All rights reserved.

#include "OfflineStorage.hpp"
#include "LogManager.hpp"

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
    LogManager::DispatchEvent(DebugEventType::EVT_CACHED);
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
    m_offlineStorage.DeleteRecords(ctx->recordIds, headers, ctx->fromMemory);
    return true;
}

bool OfflineStorage::handleReleaseRecords(EventsUploadContextPtr const& ctx)
{
    HttpHeaders headers;
    if (ctx->httpResponse)
    {
        headers = ctx->httpResponse->GetHeaders();
    }
    m_offlineStorage.ReleaseRecords(ctx->recordIds, false, headers, ctx->fromMemory);
    return true;
}

bool OfflineStorage::handleReleaseRecordsIncRetryCount(EventsUploadContextPtr const& ctx)
{
    LogManager::DispatchEvent(DebugEventType::EVT_SEND_RETRY);
    HttpHeaders headers;
    if (ctx->httpResponse)
    {
        headers = ctx->httpResponse->GetHeaders();
    }
    m_offlineStorage.ReleaseRecords(ctx->recordIds, true, headers, ctx->fromMemory);
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

void OfflineStorage::OnStorageTrimmed(unsigned numRecords)
{   
    StorageNotificationContext ctx;
    ctx.count = numRecords;
    trimmed(&ctx);

    DebugEvent evt;
    evt.type = EVT_DROPPED;
    evt.param1 = numRecords;
    evt.size = numRecords;
    LogManager::DispatchEvent(evt);
}

void OfflineStorage::OnStorageRecordsDropped(unsigned numRecords)
{
    StorageNotificationContext ctx;
    ctx.count = numRecords;
    recordsDropped(&ctx);
   
    DebugEvent evt;
    evt.type = EVT_DROPPED;
    evt.param1 = numRecords;
    evt.size = numRecords;
    LogManager::DispatchEvent(evt);
}


} ARIASDK_NS_END
