// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <IOfflineStorage.hpp>
#include "system/Contexts.hpp"
#include "system/Route.hpp"

namespace ARIASDK_NS_BEGIN {


class OfflineStorage : public IOfflineStorageObserver {
  public:
    OfflineStorage(IOfflineStorage& offlineStorage);
    ~OfflineStorage();

  protected:
    bool handleStart();
    bool handleStop();

    bool handleStoreRecord(IncomingEventContextPtr const& ctx);
    void handleRetrieveEvents(EventsUploadContextPtr const& ctx);

    bool handleDeleteRecords(EventsUploadContextPtr const& ctx);
    bool handleReleaseRecords(EventsUploadContextPtr const& ctx);
    bool handleReleaseRecordsIncRetryCount(EventsUploadContextPtr const& ctx);

  protected:
    virtual void OnStorageOpened(std::string const& type) override;
    virtual void OnStorageFailed(std::string const& reason) override;
    virtual void OnStorageTrimmed(unsigned numRecords) override;
    virtual void OnStorageRecordsDropped(unsigned numRecords) override;

  protected:
    IOfflineStorage& m_offlineStorage;

  public:
    RoutePassThrough<OfflineStorage>                                        start{this, &OfflineStorage::handleStart};
    RoutePassThrough<OfflineStorage>                                        stop{this, &OfflineStorage::handleStop};

    RouteSource<IncomingEventContextPtr const&>                             storeRecordFailed;
    RoutePassThrough<OfflineStorage, IncomingEventContextPtr const&>        storeRecord{this, &OfflineStorage::handleStoreRecord};

    RouteSink<OfflineStorage, EventsUploadContextPtr const&>                retrieveEvents{this, &OfflineStorage::handleRetrieveEvents};
    RouteSource<EventsUploadContextPtr const&, StorageRecord const&, bool&> retrievedEvent;
    RouteSource<EventsUploadContextPtr const&>                              retrievalFinished;
    RouteSource<EventsUploadContextPtr const&>                              retrievalFailed;

    RoutePassThrough<OfflineStorage, EventsUploadContextPtr const&>         deleteRecords{this, &OfflineStorage::handleDeleteRecords};
    RoutePassThrough<OfflineStorage, EventsUploadContextPtr const&>         releaseRecords{this, &OfflineStorage::handleReleaseRecords};
    RoutePassThrough<OfflineStorage, EventsUploadContextPtr const&>         releaseRecordsIncRetryCount{this, &OfflineStorage::handleReleaseRecordsIncRetryCount};


    RouteSource<StorageNotificationContext const*>                          opened;
    RouteSource<StorageNotificationContext const*>                          failed;
    RouteSource<StorageNotificationContext const*>                          trimmed;
    RouteSource<StorageNotificationContext const*>                          recordsDropped;
};


} ARIASDK_NS_END
