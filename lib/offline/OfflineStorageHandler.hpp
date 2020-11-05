//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#ifndef OFFLINESTORAGEHANDLER_HPP
#define OFFLINESTORAGEHANDLER_HPP

#include "pal/PAL.hpp"
#include "IOfflineStorage.hpp"

#include "api/IRuntimeConfig.hpp"
#include "ILogManager.hpp"
#include "pal/TaskDispatcher.hpp"

#include <memory>
#include <atomic>
#include <list>
#include <string>

#include "KillSwitchManager.hpp"
#include "ClockSkewManager.hpp"

namespace MAT_NS_BEGIN {

    class OfflineStorageHandler : public IOfflineStorage, public IOfflineStorageObserver
    {
    public:
        OfflineStorageHandler(ILogManager& logManager, IRuntimeConfig& runtimeConfig, ITaskDispatcher& taskDispatcher);
        virtual ~OfflineStorageHandler() override;
        virtual void Initialize(IOfflineStorageObserver& observer) override;
        virtual void Shutdown() override;
        virtual void Flush() override;
        virtual bool StoreRecord(StorageRecord const& record) override;
        virtual size_t StoreRecords(std::vector<StorageRecord> & records) override;
        virtual bool GetAndReserveRecords(std::function<bool(StorageRecord&&)> const& consumer, unsigned leaseTimeMs, EventLatency minLatency = EventLatency_Unspecified, unsigned maxCount = 0) override;

        virtual bool IsLastReadFromMemory() override;
        virtual unsigned LastReadRecordCount() override;

        virtual void DeleteRecords(const std::map<std::string, std::string> & whereFilter) override;
        virtual void DeleteRecords(std::vector<StorageRecordId> const& ids, HttpHeaders headers, bool& fromMemory) override;
        virtual void DeleteAllRecords() override;
        virtual void ReleaseRecords(std::vector<StorageRecordId> const& ids, bool incrementRetryCount, HttpHeaders headers, bool& fromMemory) override;

        virtual bool StoreSetting(std::string const& name, std::string const& value) override;
        virtual std::string GetSetting(std::string const& name) override;
        virtual bool DeleteSetting(std::string const& name) override;

        virtual size_t GetSize() override;
        virtual size_t GetRecordCount(EventLatency latency = EventLatency_Unspecified) const override;

        virtual std::vector<StorageRecord> GetRecords(bool shutdown, EventLatency minLatency = EventLatency_Unspecified, unsigned maxCount = 0) override;
        virtual bool ResizeDb() override;

        virtual void OnStorageOpened(std::string const& type) override;
        virtual void OnStorageFailed(std::string const& reason) override;
        virtual void OnStorageOpenFailed(std::string const& reason) override;
        virtual void OnStorageTrimmed(std::map<std::string, size_t> const& numRecords) override;
        virtual void OnStorageRecordsDropped(std::map<std::string, size_t> const& numRecords) override;
        virtual void OnStorageRecordsRejected(std::map<std::string, size_t> const& numRecords) override;
        virtual void OnStorageRecordsSaved(size_t numRecords) override;

    protected:
        virtual void DeleteRecordsByKeys(const std::list<std::string> & keys);

        IOfflineStorageObserver   * m_observer;
        ILogManager &               m_logManager;
        std::string                 m_databasePath;
        IRuntimeConfig&             m_config;
        ITaskDispatcher&            m_taskDispatcher;
        
        KillSwitchManager           m_killSwitchManager;
        ClockSkewManager            m_clockSkewManager;

        virtual bool isKilled(StorageRecord const& record);

        std::mutex                             m_flushLock;
        bool                                   m_flushPending;
        PAL::DeferredCallbackHandle            m_flushHandle;
        PAL::Event                             m_flushComplete;

        std::unique_ptr<IOfflineStorage>       m_offlineStorageMemory;
        std::shared_ptr<IOfflineStorage>       m_offlineStorageDisk;

        bool                                   m_readFromMemory;
        unsigned                               m_lastReadCount;

        bool                                   m_shutdownStarted;
        unsigned                               m_memoryDbSize;
        unsigned                               m_memoryDbSizeNotificationLimit;
        unsigned                               m_queryDbSize;
        bool                                   m_isStorageFullNotificationSend;

    protected:
        MATSDK_LOG_DECL_COMPONENT_CLASS();

    private:
        void WaitForFlush();

    };


} MAT_NS_END

#endif
