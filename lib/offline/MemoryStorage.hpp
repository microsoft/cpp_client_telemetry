//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef MEMORYSTORAGE_HPP
#define MEMORYSTORAGE_HPP

#pragma once

#include "pal/PAL.hpp"

#include "IOfflineStorage.hpp"

#include "api/IRuntimeConfig.hpp"

#include "ILogManager.hpp"

#include <algorithm>
#include <memory>
#include <mutex>
#include <map>
#include <string>
#include <unordered_set>

namespace MAT_NS_BEGIN {

    class MemoryStorage : public IOfflineStorage
    {

    public:
        MemoryStorage(ILogManager& logManager, IRuntimeConfig& runtimeConfig);

        virtual void Initialize(IOfflineStorageObserver& observer) override;

        virtual void Shutdown() override;

        virtual void Flush() override;

        virtual bool StoreRecord(StorageRecord const& record) override;

        virtual size_t StoreRecords(std::vector<StorageRecord> & records) override;

        virtual bool GetAndReserveRecords(std::function<bool(StorageRecord&&)> const& consumer, unsigned leaseTimeMs,
            EventLatency minLatency = EventLatency_Unspecified, unsigned maxCount = 0) override;

        virtual bool IsLastReadFromMemory() override;

        virtual unsigned LastReadRecordCount() override;

        virtual void DeleteAllRecords() override;

        virtual void DeleteRecords(const std::map<std::string, std::string> & whereFilter = {}) override;

        virtual void DeleteRecords(std::vector<StorageRecordId> const& ids, HttpHeaders headers, bool& fromMemory) override;

        virtual void ReleaseRecords(std::vector<StorageRecordId> const& ids, bool incrementRetryCount, HttpHeaders headers, bool& fromMemory) override;

        virtual void ReleaseAllRecords() override;

        virtual bool StoreSetting(std::string const& name, std::string const& value) override;

        virtual std::string GetSetting(std::string const& name) override;

        virtual bool DeleteSetting(std::string const& name) override;

        virtual size_t GetSize() override;

        virtual size_t GetRecordCount(EventLatency latency = EventLatency_Unspecified) const override;

        virtual size_t GetReservedCount();

        virtual std::vector<StorageRecord> GetRecords(bool shutdown = false, EventLatency minLatency = EventLatency_Unspecified, unsigned maxCount = 0) override;

        virtual bool ResizeDb() override;

        virtual ~MemoryStorage() override;

    protected:
        
        IOfflineStorageObserver*    m_observer;
        IRuntimeConfig&             m_config;
        ILogManager&                m_logManager;

        mutable std::mutex          m_records_lock;
        std::vector<StorageRecord>  m_records[EventLatency_Max+1];
        
        /// <summary>
        /// Contains reserved (aka in-flight) records.
        /// Current storage interface API requires deletion and release by StorageRecordId.
        /// </summary>
        std::mutex                  m_reserved_lock;
        std::map<StorageRecordId, StorageRecord> m_reserved_records;

        size_t                      m_size;

        MATSDK_LOG_DECL_COMPONENT_CLASS();

    private:
        size_t                      m_lastReadCount;

    };

} MAT_NS_END
#endif

