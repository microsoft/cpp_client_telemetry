//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef STORAGEOBSERVER_HPP
#define STORAGEOBSERVER_HPP

#include "IOfflineStorage.hpp"

#include "system/Contexts.hpp"
#include "system/Route.hpp"
#include "system/ITelemetrySystem.hpp"

namespace MAT_NS_BEGIN {

    class StorageObserver :
        public IOfflineStorageObserver,
        public DebugEventDispatcher
    {
    public:
        StorageObserver(ITelemetrySystem& system, IOfflineStorage& offlineStorage);
        ~StorageObserver();

        virtual bool DispatchEvent(DebugEvent evt) override
        {
            return m_system.DispatchEvent(std::move(evt));
        }

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
        virtual void OnStorageOpenFailed(std::string const &reason) override;
        virtual void OnStorageTrimmed(std::map<std::string, size_t> const& numRecords) override;
        virtual void OnStorageRecordsDropped(std::map<std::string, size_t> const& numRecords) override;
        virtual void OnStorageRecordsRejected(std::map<std::string, size_t> const& numRecords) override;
        virtual void OnStorageRecordsSaved(size_t numRecords) override;

    protected:
        ITelemetrySystem & m_system;
        IOfflineStorage  & m_offlineStorage;

    public:

        size_t GetSize()
        {
            return m_offlineStorage.GetSize();
        }

        size_t GetRecordCount() const
        {
            return m_offlineStorage.GetRecordCount();
        }

        RoutePassThrough<StorageObserver>                                        start{ this, &StorageObserver::handleStart };
        RoutePassThrough<StorageObserver>                                        stop{ this, &StorageObserver::handleStop };

        RouteSource<IncomingEventContextPtr const&>                              storeRecordFailed;
        RoutePassThrough<StorageObserver, IncomingEventContextPtr const&>        storeRecord{ this, &StorageObserver::handleStoreRecord };

        RouteSink<StorageObserver, EventsUploadContextPtr const&>                retrieveEvents{ this, &StorageObserver::handleRetrieveEvents };
        RouteSource<EventsUploadContextPtr const&, StorageRecord const&, bool&>  retrievedEvent;
        RouteSource<EventsUploadContextPtr const&>                               retrievalFinished;
        RouteSource<EventsUploadContextPtr const&>                               retrievalFailed;

        RoutePassThrough<StorageObserver, EventsUploadContextPtr const&>         deleteRecords{ this, &StorageObserver::handleDeleteRecords };
        RoutePassThrough<StorageObserver, EventsUploadContextPtr const&>         releaseRecords{ this, &StorageObserver::handleReleaseRecords };
        RoutePassThrough<StorageObserver, EventsUploadContextPtr const&>         releaseRecordsIncRetryCount{ this, &StorageObserver::handleReleaseRecordsIncRetryCount };

        RouteSource<StorageNotificationContext const*>                          opened;
        RouteSource<StorageNotificationContext const*>                          failed;
        RouteSource<StorageNotificationContext const*>                          trimmed;
        RouteSource<StorageNotificationContext const*>                          recordsDropped;
        RouteSource<StorageNotificationContext const*>                          recordsRejected;
    };


} MAT_NS_END
#endif

