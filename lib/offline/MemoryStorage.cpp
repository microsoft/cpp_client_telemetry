//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "MemoryStorage.hpp"

#include "utils/StringUtils.hpp"
#include <climits>

namespace MAT_NS_BEGIN {

    MATSDK_LOG_INST_COMPONENT_CLASS(MemoryStorage, "EventsSDK.MemoryStorage", "Events telemetry client - MemoryStorage class");

    MemoryStorage::MemoryStorage(ILogManager & logManager, IRuntimeConfig & runtimeConfig) :
        m_observer(nullptr),
        m_config(runtimeConfig),
        m_logManager(logManager),
        m_size(0),
        m_lastReadCount(0)
    {
    }
    
    /// <summary>
    /// Initializes the storage and sets the observer for callback notifications.
    /// NOT IMPLEMENTED: does not support IOfflineStorageObserver notifications.
    /// </summary>
    /// <param name="observer">The observer.</param>
    void MemoryStorage::Initialize(IOfflineStorageObserver & observer)
    {
        m_observer = &observer;
    }
    
    /// <summary>
    /// Shut down the offline storage.
    /// </summary>
    /// <remarks>
    /// Flush any outstanding operations, close the underlying files etc.
    /// No other methods can be called after shutdown. Called from the
    /// internal worker thread.
    /// </remarks>
    void MemoryStorage::Shutdown()
    {
        LOCKGUARD(m_reserved_lock);
        LOCKGUARD(m_records_lock);

        for (unsigned latency = EventLatency_Off; (latency <= EventLatency_Max); latency++)
        {
            size_t numRecords = m_records[latency].size();
            if (numRecords)
            {
                // OfflineStorageHandler high-level wrapper must flush these on graceful shutdown
                LOG_WARN("Discarding %u unflushed records of latency %u", numRecords, latency);
            }
        }

        if (m_reserved_records.size())
        {
            LOG_WARN("Discarding %u reserved records", m_reserved_records.size());
        }
    }
    
    /// <summary>
    /// Save pending records to persistent storage.
    ///
    /// This method is NOT IMPLEMENTED because the Flush is done by OfflineStorageHandler.
    ///
    /// </summary>
    void MemoryStorage::Flush()
    {
    }
    
    template<typename T, typename V>
    bool contains(T vec, V value)
    {
        auto it = std::find(vec.begin(), vec.end(), value);
        if (it != vec.end())
            return true;
        return false;
    }

    /// <summary>
    /// Store one telemetry event record
    /// </summary>
    /// <param name="record">Record data to store</param>
    /// <returns>
    /// Whether the record was successfully stored
    /// </returns>
    /// <remarks>
    /// The offline storage might need to trim the oldest events before
    /// inserting the new one in order to maintain its configured size limit.
    /// Called from the internal worker thread.
    /// </remarks>
    bool MemoryStorage::StoreRecord(StorageRecord const & record)
    {
        // Don't store events with latency set to off. Logger API already does a similar check.
        if (record.latency == EventLatency_Off)
            return false;

        LOCKGUARD(m_records_lock);
        m_size += record.blob.size() + sizeof(record); // approximate contents size

#ifdef DEBUG_DUPLICATE_ROUTES
        if (contains(m_records[record.latency], record))
            LOG_WARN("Vector already contains this element!");
#endif

        m_records[record.latency].push_back(std::move(record));
        return true;
    }

    size_t MemoryStorage::StoreRecords(std::vector<StorageRecord> & records)
    {
        size_t stored = 0;
        for (auto  & i : records) {
            if (StoreRecord(i)) {
                ++stored;
            }
        }
        return stored;
    }

    /// <summary>
    /// Get records from MemoryStorage.
    /// Getting records automatically deletes them.
    /// Reservation is not currently supported.
    /// </summary>
    /// <param name="consumer">The consumer.</param>
    /// <param name="leaseTimeMs">The lease time ms.</param>
    /// <param name="minLatency">The minimum latency.</param>
    /// <param name="maxCount">The maximum count.</param>
    /// <returns></returns>
    bool MemoryStorage::GetAndReserveRecords(std::function<bool(StorageRecord&&)> const & consumer, unsigned leaseTimeMs, EventLatency minLatency, unsigned maxCount)
    {
        UNREFERENCED_PARAMETER(leaseTimeMs);

        LOG_TRACE("Retrieving max. %u%s events of latency at least %d (%s)",
            maxCount, (maxCount > 0) ? "" : " (unlimited)",
            minLatency, latencyToStr(static_cast<EventLatency>(minLatency)));

        if (maxCount == 0)
            maxCount = UINT_MAX;

        if (minLatency == EventLatency_Unspecified)
            minLatency = EventLatency_Off;

        LOCKGUARD(m_reserved_lock);
        LOCKGUARD(m_records_lock);
        m_lastReadCount = 0;
        // Start processing events of critical latency first
        for (int latency = static_cast<int>(EventLatency_Max); (latency >= static_cast<int>(minLatency)) && (maxCount); latency--)
        {
            while (maxCount && (m_records[latency]).size())
            {
                StorageRecord & record = m_records[latency].back();

                size_t recordSize = record.blob.size() + sizeof(record);
                StorageRecord forConsumer(record);
                if (leaseTimeMs)
                {
                    forConsumer.reservedUntil = PAL::getUtcSystemTimeMs() + leaseTimeMs;
                }

                bool wantMore = consumer(std::move(forConsumer)); // move to consumer
                if (!wantMore) {
                    return true;
                }

                if (leaseTimeMs) {
                    m_reserved_records[record.id] = std::move(record); // move to reserved
                }
                m_records[latency].pop_back();
                m_size -= std::min(m_size, recordSize);
                maxCount--;
                m_lastReadCount++;
            }
        }
        return true;
    }
    
    /// <summary>
    /// Determines whether the records were last read from memory. Always returns true.
    /// </summary>
    /// <returns>
    ///   <c>true</c> if [is last read from memory]; otherwise, <c>false</c>.
    /// </returns>
    bool MemoryStorage::IsLastReadFromMemory()
    {
        return true;
    }
    
    /// <summary>
    /// Lasts read record count. This routine assumes that there is only one reader.
    /// </summary>
    /// <returns></returns>
    unsigned MemoryStorage::LastReadRecordCount()
    {
        LOCKGUARD(m_records_lock);
        return static_cast<unsigned>(m_lastReadCount);
    }

    void MemoryStorage::DeleteAllRecords()
    {
        {
            LOCKGUARD(m_reserved_lock);
            if (m_reserved_records.size())
            {
                m_reserved_records.clear();
            }
        }
        {
            LOCKGUARD(m_records_lock);
            for (unsigned latency = EventLatency_Off; (latency <= EventLatency_Max); latency++)
            {
                auto& records = m_records[latency];
                if (records.size()) {
                    records.clear();
                }
            }
            m_size = 0;
            m_lastReadCount = 0;
        }

    }

    void MemoryStorage::DeleteRecords(const std::map<std::string, std::string> & whereFilter)
    {
        auto matcher = [&](const StorageRecord &r, const std::map<std::string, std::string> & whereFilter)
        {
            bool matched = true;
            for (const auto &kv : whereFilter)
            {
                matched &=
                    (kv.first == "record_id") ? (r.id == kv.second) :
                    (kv.first == "tenant_token") ? (r.tenantToken == kv.second) :
                    (kv.first == "latency") ? (std::to_string(r.latency) == kv.second) :
                    (kv.first == "persistence") ? (std::to_string(r.persistence) == kv.second) :
                    (kv.first == "retry_count") ? (std::to_string(r.retryCount) == kv.second) : false;
                if (!matched)
                    break;
            }
            return matched;
        };

        // Delete from reserved, which is typically a shorter list
        std::vector<StorageRecordId> m_reserved_ids;
        {
            LOCKGUARD(m_reserved_lock);
            for (const auto & kv : m_reserved_records)
            {
                if (matcher(kv.second, whereFilter))
                {
                    m_reserved_ids.push_back(kv.first);
                }
            }
        }
        if (!m_reserved_ids.empty())
        {
            bool fromMemory = true;
            DeleteRecords(m_reserved_ids, HttpHeaders(), fromMemory);
        }

        // Delete from ram queue, which is a bigger list
        {
            LOCKGUARD(m_records_lock);
            for (unsigned latency = EventLatency_Off; latency <= EventLatency_Max;  latency++)
            {
                auto& records = m_records[latency];
                auto it = records.begin();
                // remove from records all ids that were found in the set
                while (it != records.end()) {
                    auto &v = *it;
                    if (matcher(v, whereFilter))
                    {
                        size_t recordSize = v.blob.size() + sizeof(v);
                        m_size -= std::min(m_size, recordSize);
                        it = records.erase(it);
                        continue;
                    }
                    ++it;
                }
            }
        }
    }

    /// <summary>
    /// MemoryStorage delete from reserved and ram queue.
    /// </summary>
    /// <param name="ids"></param>
    /// <param name="headers"></param>
    /// <param name="fromMemory"></param>
    void MemoryStorage::DeleteRecords(std::vector<StorageRecordId> const & ids, HttpHeaders headers, bool & fromMemory)
    {
        UNREFERENCED_PARAMETER(headers);
        UNREFERENCED_PARAMETER(fromMemory);

        {
            // Delete from reserved records (m_reserved_records)
            LOCKGUARD(m_reserved_lock);
            if (m_reserved_records.size())
            {
                std::unordered_set<StorageRecordId> idSet(ids.begin(), ids.end());
                auto it = m_reserved_records.begin();
                while (it != m_reserved_records.end()) {
                    auto &kv = *it;
                    if (idSet.count(kv.first))
                    {
                        idSet.erase(kv.first);
                        it = m_reserved_records.erase(it);
                        continue;
                    }
                    ++it;
                }
                if (idSet.size() == 0) // done
                    return;
            }
        }

        {
            // Delete from ram queue (m_records[])
            LOCKGUARD(m_records_lock);

            // convert vector of ids to unordered set
            std::unordered_set<StorageRecordId> idSet(ids.begin(), ids.end());

            // For each latency - delete from current unreserved records
            for (unsigned latency = EventLatency_Off; (latency <= EventLatency_Max); latency++)
            {
                auto& records = m_records[latency];
                if (records.size() && idSet.size())
                {
                    auto it = records.begin();
                    // remove from records all ids that were found in the set
                    while (it != records.end()) {
                        auto &v = *it;
                        if (idSet.count(v.id))
                        {
                            // record id appears once only, so remove from set
                            idSet.erase(v.id);
                            size_t recordSize = v.blob.size() + sizeof(v);
                            m_size -= std::min(m_size, recordSize);
                            it = records.erase(it);
                            continue;
                        }
                        ++it;
                    }
                }
            }
        }

    }

    /// <summary>
    /// ReleaseRecords returns records back from in-flight into RAM queue.
    /// </summary>
    /// <param name="ids"></param>
    /// <param name="incrementRetryCount"></param>
    /// <param name="headers"></param>
    /// <param name="fromMemory"></param>
    void MemoryStorage::ReleaseRecords(std::vector<StorageRecordId> const & ids, bool incrementRetryCount, HttpHeaders headers, bool & fromMemory)
    {
        UNREFERENCED_PARAMETER(incrementRetryCount);
        UNREFERENCED_PARAMETER(headers);
        UNREFERENCED_PARAMETER(fromMemory);

        // Move back from reserved records to ram queue
        LOCKGUARD(m_reserved_lock);
        if (m_reserved_records.size())
        {
            std::unordered_set<StorageRecordId> idSet(ids.begin(), ids.end());
            auto it = m_reserved_records.begin();
            while (it != m_reserved_records.end()) {
                auto &kv = *it;
                if (idSet.count(kv.first))
                {
                    if (incrementRetryCount)
                        kv.second.retryCount++;
                    StoreRecord(kv.second);
                    idSet.erase(kv.first);
                    it = m_reserved_records.erase(it);
                    continue;
                }
                ++it;
            }
        }
    }

    void MemoryStorage::ReleaseAllRecords()
    {
        // In case if HTTP upload has been canceled or didn't succeed,
        // we'd move all reserved records to regular ram queue
        LOCKGUARD(m_reserved_lock);
        if (m_reserved_records.size())
        {
            auto it = m_reserved_records.begin();
            while (it != m_reserved_records.end())
            {
                auto &kv = *it;
                StoreRecord(kv.second);
                it = m_reserved_records.erase(it);
            }
        }
    }

    /// <summary>
    /// Storing settings in RAM storage is not supported
    /// </summary>
    /// <param name="name"></param>
    /// <param name="value"></param>
    /// <returns></returns>
    bool MemoryStorage::StoreSetting(std::string const & name, std::string const & value)
    {
        UNREFERENCED_PARAMETER(name);
        UNREFERENCED_PARAMETER(value);

        LOG_WARN("Not implemented!");
        return false;
    }

    /// <summary>
    /// Retrieving settings from RAM storage is not supported
    /// </summary>
    /// <param name="name"></param>
    /// <returns></returns>
    std::string MemoryStorage::GetSetting(std::string const & name)
    {
        UNREFERENCED_PARAMETER(name);

        LOG_WARN("Not implemented!");
        return std::string();
    }

    /// <summary>
    /// Deleting settings from RAM storage is not supported
    /// </summary>
    /// <param name="name"></param>
    /// <returns></returns>
    bool MemoryStorage::DeleteSetting(std::string const & name)
    {
        UNREFERENCED_PARAMETER(name);

        LOG_WARN("Not implemented!");
        return false;
    }
    
    /// <summary>
    /// Get size of the ram DB excluding reserved (in-flight) records.
    /// </summary>
    /// <returns>
    /// Approximate ram queue size
    /// </returns>
    /// <remarks>
    /// Called from the internal worker thread.
    /// </remarks>
    size_t MemoryStorage::GetSize()
    {
        LOCKGUARD(m_records_lock);
        return m_size;
    }

    /// <summary>
    /// Gets the number of records of specific latency in the DB.
    /// If latency is unspecified, get the total number of records.
    /// </summary>
    /// <returns></returns>
    size_t MemoryStorage::GetRecordCount(EventLatency latency) const
    {
        LOCKGUARD(m_records_lock);
        size_t numRecords = 0;
        if (latency == EventLatency_Unspecified)
        {
            for (unsigned lat = EventLatency_Off; lat <= EventLatency_Max; lat++)
                numRecords += m_records[lat].size();
        }
        else
        {
            numRecords = m_records[latency].size();
        }
        return numRecords;
    }

    std::vector<StorageRecord> MemoryStorage::GetRecords(bool shutdown, EventLatency minLatency, unsigned maxCount)
    {
        UNREFERENCED_PARAMETER(shutdown);
        UNREFERENCED_PARAMETER(minLatency);
        UNREFERENCED_PARAMETER(maxCount);

        std::vector<StorageRecord> records;
        auto consumer = [&records](StorageRecord&& record) -> bool {
            records.push_back(std::move(record));
            return true; // want more
        };
        GetAndReserveRecords(consumer, 0, minLatency, maxCount);
        return records;
    }
    
    /// <summary>
    /// Resizes the database.
    /// </summary>
    /// <returns>If DB has been resized successfully</returns>
    /// <remarks>This method is not currently implemented</remarks>
    bool MemoryStorage::ResizeDb()
    {
        LOG_WARN("Not implemented!");
        return true;
    }

    MemoryStorage::~MemoryStorage()
    {
        // Shutdown();
    }
    
    /// <summary>
    /// Gets the reserved (in-flight) record count.
    /// This method is currently internal, but up for consideration to add it to common storage interface.
    /// </summary>
    /// <returns></returns>
    size_t MemoryStorage::GetReservedCount()
    {
        LOCKGUARD(m_reserved_lock);
        return m_reserved_records.size();
    }

} MAT_NS_END

