#ifndef MEMORYSTORAGE_HPP
#define MEMORYSTORAGE_HPP

#pragma once

#include <pal/PAL.hpp>

#include <IOfflineStorage.hpp>

#include "api/IRuntimeConfig.hpp"

#include "ILogManager.hpp"

#include <algorithm>
#include <memory>
#include <atomic>
#include <mutex>

namespace ARIASDK_NS_BEGIN {

    class MemoryStorage : public IOfflineStorage
    {

    public:
        MemoryStorage(ILogManager& logManager, IRuntimeConfig& runtimeConfig);

        virtual void Initialize(IOfflineStorageObserver& observer) override;

        virtual void Shutdown() override;

        virtual void Flush() override;

        virtual bool StoreRecord(StorageRecord const& record) override;

        virtual bool GetAndReserveRecords(std::function<bool(StorageRecord&&)> const& consumer, unsigned leaseTimeMs,
            EventLatency minLatency = EventLatency_Unspecified, unsigned maxCount = 0) override;

        virtual bool IsLastReadFromMemory() override;

        virtual unsigned LastReadRecordCount() override;

        virtual void DeleteRecords(std::vector<StorageRecordId> const& ids, HttpHeaders headers, bool& fromMemory) override;

        virtual void ReleaseRecords(std::vector<StorageRecordId> const& ids, bool incrementRetryCount, HttpHeaders headers, bool& fromMemory) override;

        virtual bool StoreSetting(std::string const& name, std::string const& value) override;

        virtual std::string GetSetting(std::string const& name) override;

        virtual unsigned GetSize() override;

        virtual std::vector<StorageRecord>* GetRecords(bool shutdown, EventLatency minLatency = EventLatency_Unspecified, unsigned maxCount = 0) override;

        virtual bool ResizeDb() override;

        virtual ~MemoryStorage() override;

    protected:
        std::mutex                  m_lock;

        IOfflineStorageObserver*    m_observer;
        IRuntimeConfig&             m_config;
        ILogManager&                m_logManager;

        std::vector<StorageRecord>  m_records[EventLatency_Max];

        std::atomic<size_t>         m_size;

        ARIASDK_LOG_DECL_COMPONENT_CLASS();

    private:
        std::atomic<size_t>         m_lastReadCount;

    };

} ARIASDK_NS_END
#endif
