// Copyright (c) Microsoft. All rights reserved.

#include "OfflineStorage.hpp"
#include "api\LogManager.hpp"

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

    if (!m_offlineStorage.GetAndReserveRecords(consumer, 120000, ctx->requestedMinPriority, ctx->requestedMaxCount)) {
        retrievalFailed(ctx);
    } else {
        retrievalFinished(ctx);
    }
}

bool OfflineStorage::handleDeleteRecords(EventsUploadContextPtr const& ctx)
{
	LogManager::DispatchEvent(DebugEventType::EVT_HTTP_OK);
	HttpHeaders headers;
	if (ctx->httpResponse)
	{
		headers = ctx->httpResponse->GetHeaders();
	}
    m_offlineStorage.DeleteRecords(ctx->recordIds, headers);
    return true;
}

bool OfflineStorage::handleReleaseRecords(EventsUploadContextPtr const& ctx)
{
	LogManager::DispatchEvent(DebugEventType::EVT_SEND_RETRY);
	HttpHeaders headers;
	if (ctx->httpResponse)
	{
		headers = ctx->httpResponse->GetHeaders();
	}
    m_offlineStorage.ReleaseRecords(ctx->recordIds, false, headers);
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
    m_offlineStorage.ReleaseRecords(ctx->recordIds, true, headers);
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
	LogManager::DispatchEvent(DebugEventType::EVT_STORAGE_FULL);
    StorageNotificationContext ctx;
    ctx.str = reason;
    failed(&ctx);
}

void OfflineStorage::OnStorageTrimmed(unsigned numRecords)
{
    StorageNotificationContext ctx;
    ctx.count = numRecords;
    trimmed(&ctx);
}

void OfflineStorage::OnStorageRecordsDropped(unsigned numRecords)
{
	LogManager::DispatchEvent(DebugEventType::EVT_DROPPED);
    StorageNotificationContext ctx;
    ctx.count = numRecords;
    recordsDropped(&ctx);
}


} ARIASDK_NS_END
