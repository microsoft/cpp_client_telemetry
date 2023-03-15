//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef IOFFLINESTORAGE_HPP
#define IOFFLINESTORAGE_HPP

#include "Enums.hpp"
#include "IHttpClient.hpp"
#include "ctmacros.hpp"
#include "ILogManager.hpp"

#include <functional>
#include <string>
#include <vector>
#include <map>

namespace MAT_NS_BEGIN {

    constexpr unsigned int DB_FULL_NOTIFICATION_DEFAULT_PERCENTAGE = 75;
    constexpr uint64_t     DB_FULL_CHECK_INTERVAL_DEFAULT_MS = 5000;

    using StorageRecordId = std::string;

    using StorageBlob = std::vector<uint8_t>;

    struct StorageRecord {
        StorageRecordId id;
        std::string     tenantToken;
        EventLatency    latency = EventLatency_Unspecified;
        EventPersistence persistence = EventPersistence_Normal;
        int64_t         timestamp = 0;
        StorageBlob     blob;
        int             retryCount = 0;
        int64_t         reservedUntil = 0;
#ifdef HAVE_MAT_EVT_TRACEID 
        std::string     traceId;
#endif // HAVE_MAT_EVT_TRACEID

        StorageRecord()
        {}

#ifdef HAVE_MAT_EVT_TRACEID
        StorageRecord(std::string const& id, std::string const& tenantToken, EventLatency latency, EventPersistence persistence, std::string traceId)
            : id(id), tenantToken(tenantToken), latency(latency), persistence(persistence), traceId(traceId)
        {}
#else
        StorageRecord(std::string const& id, std::string const& tenantToken, EventLatency latency, EventPersistence persistence)
            : id(id), tenantToken(tenantToken), latency(latency), persistence(persistence)
        {}
#endif // HAVE_MAT_EVT_TRACEID

        StorageRecord(std::string const& id, std::string const& tenantToken, EventLatency latency, EventPersistence persistence,
            int64_t timestamp, std::vector<uint8_t>&& blob, int retryCount = 0, int64_t reservedUntil = 0)
            : id(id), tenantToken(tenantToken), latency(latency), persistence(persistence), timestamp(timestamp), blob(blob), retryCount(retryCount), reservedUntil(reservedUntil)
        {}

        bool operator==(const StorageRecord& rhs) {
            return ((*this).id == rhs.id);
        }

    };

    using StorageRecordVector = std::vector<StorageRecord>;
    using DroppedMap = std::map<std::string, size_t>;

    class IOfflineStorageObserver {
    public:
        virtual ~IOfflineStorageObserver() {}

        /// <summary>
        /// Called when the offline storage (re)opens its backing storage
        /// <summary>
        /// <remarks>
        /// The <paramref name="type"> parameter is any textual description
        /// of the active underlying offline storage implementation. The
        /// recommended format is "<c>&lt;impl&gt;/&lt;state&gt;</c>", where
        /// <c>&lt;impl&gt;</c> is name of the implementation (e.g. "SQLite")
        /// and <c>&lt;state&gt;</c> is description of the current state
        /// (e.g. if using the "Default", "Temp" or "None" database file).
        /// </remarks>
        /// <param name="type">Current storage description</param>
        virtual void OnStorageOpened(std::string const& type) = 0;

        /// <summary>
        /// Called when the offline storage encounters some failure
        /// <summary>
        /// <remarks>
        /// The parameter <paramref name="reason"> is any textual description
        /// of the problem that occurred. It does not necessarily have to be
        /// human-readable or self-explaining, it can be just a numerical code
        /// that only that implementation's maintainer can understand.
        /// </remarks>
        /// <param name="reason">Reason of the current/recent failure</param>
        virtual void OnStorageFailed(std::string const& reason) = 0;

        /// <summary>
        /// Called when the offline storage is not open.
        /// <summary>
        /// <remarks>
        /// The parameter <paramref name="reason"> is any textual description
        /// of the problem that occurred. It does not necessarily have to be
        /// human-readable or self-explaining, it can be just a numerical code
        /// that only that implementation's maintainer can understand.
        /// </remarks>
        /// <param name="reason">Reason of the current/recent failure</param>
        virtual void OnStorageOpenFailed(std::string const& reason) = 0;

        /// <summary>
        /// Called when the offline storage trims some records off in order to
        /// maintain its configured size limit
        /// <summary>
        /// <param name="numRecords">Number of records trimmed</param>
        virtual void OnStorageTrimmed(DroppedMap const& numRecords) = 0;

        /// <summary>
        /// Called when the offline storage drops some records with retry count
        /// over the configured limit
        /// <summary>
        /// <param name="numRecords">Number of records dropped</param>
        virtual void OnStorageRecordsDropped(std::map<std::string, size_t> const& numRecords) = 0;

        /// <summary>
        /// Called when the offline storage rejects some records for reason like killSwitch
        /// over the configured limit
        /// <summary>
        /// <param name="numRecords">Number of records dropped</param>
        virtual void OnStorageRecordsRejected(std::map<std::string, size_t> const& numRecords) = 0;

        virtual void OnStorageRecordsSaved(size_t numRecords) = 0;
    };

    class IOfflineStorage
    {
    public:

        IOfflineStorage() noexcept = default;
        virtual ~IOfflineStorage() noexcept = default;

        /// <summary>
        /// Initialize the offline storage
        /// </summary>
        /// <remarks>
        /// Prepare any external libraries, open files etc. Called from the
        /// internal worker thread as the initialization can take longer time.
        /// Any other methods can be called only after initializing. If the
        /// offline storage cannot be initialized, calling other methods later
        /// must be still possible, they should return some default/error values.
        /// The argument <paramref name="observer"/> specifies an instance of
        /// <see cref="IOfflineStorageObserver"/> which will be used to notify the
        /// owner about side actions performed by the storage implementation
        /// (failures, dropping trimmed events etc.). The callback methods of
        /// <paramref name="observer"/> can be invoked during execution of any of
        /// the other methods of this interface, so the observer object must stay
        /// alive until after the storage has been fully shut down.
        /// <param name="observer">Notification observer instance</param>
        virtual void Initialize(IOfflineStorageObserver& observer) = 0;

        /// <summary>
        /// Shut down the offline storage
        /// </summary>
        /// <remarks>
        /// Flush any outstanding operations, close the underlying files etc.
        /// No other methods can be called after shutdown. Called from the
        /// internal worker thread.
        /// </remarks>
        virtual void Shutdown() = 0;

        /// <summary>
        /// Save pending records to persistent storage
        /// </summary>
        virtual void Flush() = 0;

        /// <summary>
        /// Store one telemetry event record
        /// </summary>
        /// <remarks>
        /// The offline storage might need to trim the oldest events before
        /// inserting the new one in order to maintain its configured size limit.
        /// Called from the internal worker thread.
        /// </remarks>
        /// <param name="record">Record data to store</param>
        /// <returns>Whether the record was successfully stored</returns>
        virtual bool StoreRecord(StorageRecord const& record) = 0;

        /// <summary>
        /// Store several telemetry event records
        /// </summary>
        /// <remarks>
        /// The offline storage might need to trim the oldest events before
        /// inserting the new one in order to maintain its configured size limit.
        /// Called from the internal worker thread.
        /// </remarks>
        /// <param name="record">Record data to store</param>
        /// <returns>Number of records stored</returns>
        virtual size_t StoreRecords(StorageRecordVector & records) = 0;

        /// <summary>
        /// Retrieve the best records to upload based on specified parameters
        /// </summary>
        /// <remarks>
        /// Retrieves stored records one by one, filtered and ordered based on the
        /// specified parameters. The priority is considered first: only events of
        /// the highest priority found, higher or equal to
        /// <paramref name="minPriority"/>, are returned during one call to this
        /// method. The timestamp is considered next: events are returned in a
        /// decreasing timestamp order, i.e. from the oldest to newest. The
        /// specified <paramref name="maxCount"/> is checked the last. The
        /// retrieval can be aborted after any record if the
        /// <paramref name="consumer"/> returns <c>false</c>. Records which were
        /// accepted by the consumer are reserved for the specified amount of time
        /// <paramref name="leaseTimeMs"/> and will not be returned again by this
        /// method until explicitly released or deleted or until their reservation
        /// period expires. Called from the internal worker thread.
        /// <param name="consumer">Callback functor processing the individual
        /// retrieved records</param>
        /// <param name="leaseTimeMs">Amount of time all acccepted records should
        /// be reserved for, in milliseconds</param>
        /// <param name="minPriority">Minimum priority of events to be
        /// retrieved</param>
        /// <param name="maxCount">Maximum number of events to retrieve</param>
        /// <returns><c>true</c> if everything went well (even with no events
        /// really accepted by the consumer), <c>false</c> if an error occurred and
        /// the retrieval ended prematurely, records could not be reserved
        /// etc.</returns>
        virtual bool GetAndReserveRecords(std::function<bool(StorageRecord&&)> const& consumer, unsigned leaseTimeMs,
            EventLatency minLatency = EventLatency_Unspecified, unsigned maxCount = 0) = 0;

        /// <summary>
        /// return where the last read was memory or disk
        /// </summary>
        /// <remarks>
        virtual bool IsLastReadFromMemory() = 0;

        /// <summary>
        /// return last read count
        /// </summary>
        /// <remarks>
        virtual unsigned LastReadRecordCount() = 0;

        /// <summary>
        /// Delete all records from storage
        /// </summary>
        virtual void DeleteAllRecords() = 0;

        /// <summary>
        /// Bulk delete records using "where" clause.
        /// Specify condition using key-value pairs in the map.
        /// </summary>
        /// <remarks>
        virtual void DeleteRecords(const std::map<std::string, std::string> & whereFilter) = 0;

        /// <summary>
        /// Delete records with specified IDs
        /// </summary>
        /// <remarks>
        /// IDs of records that are no longer found in the storage are silently
        /// ignored. Called from the internal worker thread.
        /// </remarks>
        /// <param name="ids">Identifiers of records to delete</param>
        virtual void DeleteRecords(std::vector<StorageRecordId> const& ids, HttpHeaders headers, bool& fromMemory) = 0;

        /// <summary>
        /// Release event records with specified IDs
        /// </summary>
        /// <remarks>
        /// IDs of events that are no longer found in the storage are silently
        /// ignored. If <paramref name="incrementRetryCount"/> is set and the retry
        /// counter of some records reaches the maximum retry count, those events
        /// may be dropped as part of the releasing procedure. Persistent storage
        /// implementations of this interface drop these records. MemoryStorage does not.
        /// Called from the internal worker thread.
        /// </remarks>
        /// <param name="ids">Identifiers of records to release</param>
        /// <param name="incrementRetryCount">Determines whether the retry
        /// counter should be incremented for the records</param>
        virtual void ReleaseRecords(std::vector<StorageRecordId> const& ids, bool incrementRetryCount, HttpHeaders headers, bool& fromMemory) = 0;

        /// <summary>
        /// Delete value of an auxiliary persistent configuration value
        /// </summary>
        /// <remarks>
        /// If a setting with the specified name does not exist, success is returned.
        /// </remarks>
        /// <param name="name">Name of the setting to retrieve</param>
        /// <returns>Status of operation</returns>
        virtual bool DeleteSetting(std::string const& name) = 0;

        /// <summary>
        /// Set value of an auxiliary persistent configuration value
        /// </summary>
        /// <remarks>
        /// Set <paramref name="value"/> to an empty string to delete any existing
        /// value. Called from the internal worker thread.
        /// </remarks>
        /// <param name="name">Name of the setting to update</param>
        /// <param name="value">New setting value</param>
        /// <returns></returns>
        virtual bool StoreSetting(std::string const& name, std::string const& value) = 0;

        /// <summary>
        /// Get value of an auxiliary persistent configuration value
        /// </summary>
        /// <remarks>
        /// If a setting with the specified name does not exist, an empty string is
        /// returned. Called from the internal worker thread.
        /// </remarks>
        /// <param name="name">Name of the setting to retrieve</param>
        /// <returns>Value of the requested setting or an empty string</returns>
        virtual std::string GetSetting(std::string const& name) = 0;

        /// <summary>
        /// Get size of the DB
        /// </summary>
        /// <remarks>
        /// Get current Db size returned. Called from the internal worker thread.
        /// </remarks>
        /// <returns>Value of the requested DB size</returns>
        virtual size_t GetSize() = 0;

        /// <summary>
        /// Get number of records of specific latency.
        /// If latency is unspecified, then get the total number of records in storage.
        /// </summary>
        /// <remarks>
        /// Gets the total number of records. Primarily used on shutdown to evaluate
        /// if upload still has to be done for the remaining records.
        /// </remarks>
        /// <returns>Number of records</returns>
        virtual size_t GetRecordCount(EventLatency latency = EventLatency_Unspecified) const = 0;

        /// <summary>
        /// Get Vector of records from DB
        /// </summary>
        /// <remarks>
        /// If a setting with the specified name does not exist, an empty string is
        /// returned. Called from the internal worker thread.
        /// </remarks>
        /// <param name="shutdown">if this is called at shutdown ot not</param>
        /// <param name="minPriority">lowest priority selected</param>
        /// <param name="maxCount"> max count to be selected</param>
        /// <returns>Value of the requested setting or an empty string</returns>
        virtual std::vector<StorageRecord> GetRecords(bool shutdown, EventLatency minLatency = EventLatency_Unspecified, unsigned maxCount = 0) = 0;

        virtual bool ResizeDb() = 0;

        virtual void ReleaseAllRecords() {};

    };

    // IOfflineStorage as Module. External offline storage implementations need to inherit from it.
    class IOfflineStorageModule : public IOfflineStorage, public IModule
    {
    };


} MAT_NS_END
#endif

