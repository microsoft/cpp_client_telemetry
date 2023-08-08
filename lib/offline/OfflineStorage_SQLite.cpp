//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"
#ifdef HAVE_MAT_STORAGE

#include "OfflineStorage_SQLite.hpp"
#include "ILogManager.hpp"
#include "SQLiteWrapper.hpp"
#include "utils/StringUtils.hpp"
#include <algorithm>
#include <numeric>
#include <set>

namespace MAT_NS_BEGIN {

    constexpr static size_t kBlockSize = 8192;

    std::mutex OfflineStorage_SQLite::m_initAndShutdownLock;
    int OfflineStorage_SQLite::m_instanceCount = 0;

    class DbTransaction {
        SqliteDB* m_db;
    public:
        bool locked;

        DbTransaction(SqliteDB* db) : m_db(db), locked(false)
        {
            if (m_db)
            {
                locked = m_db->trylock();
            }
        };

        ~DbTransaction()
        {
            if (locked)
            {
                m_db->unlock();
            }
        }
    };

    MATSDK_LOG_INST_COMPONENT_CLASS(OfflineStorage_SQLite, "EventsSDK.Storage", "Events telemetry client - OfflineStorage_SQLite class");

    static int const CURRENT_SCHEMA_VERSION = 1;
#define TABLE_NAME_EVENTS   "events"
#define TABLE_NAME_SETTINGS "settings"
#define TABLE_NAME_PACKAGES "packages"

    bool OfflineStorage_SQLite::isOpen()
    {
        if ((!m_db) || (!m_isOpened))
        {
            LOG_ERROR("Database is not open!");
            m_observer->OnStorageFailed("Database is not open");
            return false;
        }
        return true;
    }

    OfflineStorage_SQLite::OfflineStorage_SQLite(ILogManager & logManager, IRuntimeConfig& runtimeConfig, bool inMemory)
        : m_config(runtimeConfig)
        , m_logManager(logManager)
    {
        uint32_t percentage = (inMemory) ? m_config[CFG_INT_RAMCACHE_FULL_PCT] : m_config[CFG_INT_STORAGE_FULL_PCT];
        m_DbSizeLimit = (inMemory) ? static_cast<uint32_t>(m_config[CFG_INT_RAM_QUEUE_SIZE])
                : m_config.GetOfflineStorageMaximumSizeBytes();
        m_offlineStorageFileName = (inMemory) ? ":memory:" : (const char *)m_config[CFG_STR_CACHE_FILE_PATH];

        if ((percentage == 0)||(percentage > 100))
        {
            percentage = DB_FULL_NOTIFICATION_DEFAULT_PERCENTAGE; // 75%
        }
        m_DbSizeNotificationLimit = (percentage * (uint32_t)m_DbSizeLimit) / 100;
        m_DbSizeNotificationInterval = m_config[CFG_INT_STORAGE_FULL_CHECK_TIME];

        uint32_t ramSizeLimit = m_config[CFG_INT_RAM_QUEUE_SIZE];
        m_DbSizeHeapLimit = ramSizeLimit;

        const char* skipSqliteInit = m_config["skipSqliteInitAndShutdown"];
        if (skipSqliteInit != nullptr)
        {
            if (std::string(skipSqliteInit) == "true")
                m_skipInitAndShutdown = true;
        }
    }

    OfflineStorage_SQLite::~OfflineStorage_SQLite()
    {
        assert(!m_db);
    }

    void OfflineStorage_SQLite::Initialize(IOfflineStorageObserver& observer)
    {
        m_observer = &observer;

        assert(!m_db);
        m_db.reset(new SqliteDB(m_skipInitAndShutdown, &m_initAndShutdownLock,
                                &m_instanceCount));

        LOG_TRACE("Initializing offline storage: %s", m_offlineStorageFileName.c_str());
        auto sqlStartTime = GetUptimeMs();
        if (m_db->initialize(m_offlineStorageFileName, false, m_DbSizeHeapLimit) && initializeDatabase()) {
            LOG_INFO("Using configured on-disk database");
            m_observer->OnStorageOpened("SQLite/Default");
            sqlStartTime = GetUptimeMs() - sqlStartTime;
            LOG_INFO("Storage opened in %lld ms", sqlStartTime);
            m_isOpened = true;
            return;
        }

        if (recreate(1))
        {
            return;
        }

        // DB is not opened
        m_db.reset();
        m_isOpened = false;
    }

    void OfflineStorage_SQLite::Shutdown()
    {
        LOG_TRACE("Shutting down offline storage %s", m_offlineStorageFileName.c_str());
        LOCKGUARD(m_lock);
        if (m_db) {
            if (m_isOpened) {
                m_db->shutdown();
                m_db.reset();
            }
            m_isOpened = false;
        }
    }

    void OfflineStorage_SQLite::Flush() 
    {
        if (m_db)
            m_db->flush();
    }
    
    void OfflineStorage_SQLite::Execute(std::string command)
    {
        if (m_db)
            m_db->execute(command.c_str());
    }

    bool OfflineStorage_SQLite::StoreRecord(StorageRecord const& record)
    {
        // TODO: [MG] - this works, but may not play nicely with several LogManager instances
        // static SqliteStatement sql_insert(*m_db, m_stmtInsertEvent_id_tenant_prio_ts_data);

        if (record.id.empty() || record.tenantToken.empty() || static_cast<int>(record.latency) < 0 || record.timestamp <= 0) {
            LOG_ERROR("Failed to store event %s:%s: Invalid parameters",
                tenantTokenToId(record.tenantToken).c_str(), record.id.c_str());
            m_observer->OnStorageFailed("Invalid parameters");
            return false;
        }

        if (!m_db) {
            LOG_ERROR("Failed to store event %s:%s: Database is not open",
                tenantTokenToId(record.tenantToken).c_str(), record.id.c_str());
            m_observer->OnStorageOpenFailed("Database is not open");
            return false;
        }

        {
#ifdef ENABLE_LOCKING
            LOCKGUARD(m_lock);
            DbTransaction transaction(m_db.get());
            if (!transaction.locked)
            {
                LOG_ERROR("Failed to store event %s:%s: Database error", tenantTokenToId(record.tenantToken).c_str(), record.id.c_str());
                m_observer->OnStorageFailed("Database error");
                return false;
            }
#endif
            SqliteStatement(*m_db, m_stmtInsertEvent_id_tenant_prio_ts_data).execute(record.id, record.tenantToken, static_cast<int>(record.latency), static_cast<int>(record.persistence), record.timestamp, record.blob);
            m_DbSizeEstimate += record.id.size() + record.tenantToken.size() + record.blob.size();
        }

        if ((m_DbSizeNotificationLimit != 0) && (m_DbSizeEstimate>m_DbSizeNotificationLimit))
        {
            auto now = PAL::getMonotonicTimeMs();
            if (static_cast<uint64_t>(now-m_isStorageFullNotificationSendTime) > m_DbSizeNotificationInterval)
            {
                // Notify the client that the DB is getting full, but only once in DB_FULL_CHECK_TIME_MS
                m_isStorageFullNotificationSendTime = now;
                m_DbSizeEstimate = GetSize();
                DebugEvent evt;
                evt.type = DebugEventType::EVT_STORAGE_FULL;
                evt.param1 = (100 * m_DbSizeEstimate) / m_DbSizeLimit;
                m_logManager.DispatchEvent(evt);
            }
        }

        if ((m_DbSizeLimit != 0) && (m_DbSizeEstimate > m_DbSizeLimit))
        {
            auto shouldResize = m_config[CFG_BOOL_ENABLE_DB_DROP_IF_FULL] && !m_resizing;
            if (shouldResize)
            {
                LOCKGUARD(m_resizeLock); //Serialize resize operations
                m_resizing = true;
                if (m_DbSizeEstimate > m_DbSizeLimit)
                {
                    ResizeDb();
                }
                m_resizing = false;
            }
        }

        return true;

    }

    size_t OfflineStorage_SQLite::StoreRecords(std::vector<StorageRecord> & records)
    {
        size_t stored = 0;
        for (auto & i : records) {
            if (StoreRecord(i)) {
                ++stored;
            }
        }
        return stored;
    }

    // Debug routine to print record count in the DB
    void OfflineStorage_SQLite::printRecordCount()
    {
#ifndef NDEBUG
        for (unsigned lat = static_cast<unsigned>(EventLatency_Off); lat <= static_cast<unsigned>(EventLatency_Max); lat++)
        {
            size_t count = GetRecordCount(static_cast<EventLatency>(lat));
            UNREFERENCED_PARAMETER(count);
            LOG_TRACE("latency[%u] count=%zu", lat, count);
        }
#endif
    }

    /// <summary>
    /// Gets the and reserve records.
    /// </summary>
    /// <param name="consumer">The consumer.</param>
    /// <param name="leaseTimeMs">The lease time ms.</param>
    /// <param name="minLatency">The minimum latency.</param>
    /// <param name="maxCount">The maximum count.</param>
    /// <returns></returns>
    bool OfflineStorage_SQLite::GetAndReserveRecords(std::function<bool(StorageRecord&&)> const& consumer, unsigned leaseTimeMs, EventLatency minLatency, unsigned maxCount)
    {
        m_lastReadCount = 0;

        if (!m_db) {
            LOG_ERROR("Failed to retrieve events to send: Database is not open");
            return false;
        }

        LOG_TRACE("Retrieving max. %u%s events of latency at least %d (%s)",
            maxCount, (maxCount > 0) ? "" : " (unlimited)", minLatency, latencyToStr(static_cast<EventLatency>(minLatency)));

        /* ============================================================================================================= */
        LOCKGUARD(m_lock);
        {
#ifdef ENABLE_LOCKING
            DbTransaction transaction(m_db.get());
            if (!transaction.locked)
            {
                LOG_ERROR("Failed to lock");
                return false;
            }
#endif
            SqliteStatement releaseStmt(*m_db, m_stmtReleaseExpiredEvents);

            if (!releaseStmt.execute(PAL::getUtcSystemTimeMs()))
                LOG_ERROR("Failed to release expired reserved events: Database error occurred");
            else {
                if (releaseStmt.changes() > 0) {
                    LOG_TRACE("Released %u expired reserved events", static_cast<unsigned>(releaseStmt.changes()));
                }
            }

            SqliteStatement selectStmt(*m_db, m_stmtSelectEvents);
            if (!selectStmt.select(static_cast<int>(minLatency), maxCount > 0 ? maxCount : -1)) {
                LOG_ERROR("Failed to retrieve events to send: Database error occurred, recreating database");
                recreate(204);
                return false;
            }

            std::vector<StorageRecordId> consumedIds;
            std::map<std::string, size_t> deletedData;

            StorageRecord record;
            int latency;

            while (selectStmt.getRow(record.id, record.tenantToken, latency, record.timestamp, record.retryCount, record.reservedUntil, record.blob))
            {
                if (latency < EventLatency_Off || latency > EventLatency_Max) {
                    record.latency = EventLatency_Normal;
                }
                else {
                    record.latency = static_cast<EventLatency>(latency);
                }
                consumedIds.push_back(record.id);
                if (!consumer(std::move(record)))
                {
                    consumedIds.pop_back();
                    break;
                }
            }

            selectStmt.reset();

            if (selectStmt.error()) {
                LOG_ERROR("Failed to search for events to send: Database error has occurred, recreating database");
                recreate(205);
                return false;
            }

            if (consumedIds.empty()) {
                return false;
            }

            LOG_TRACE("Reserving %u event(s) {%s%s} for %u milliseconds",
                static_cast<unsigned>(consumedIds.size()), consumedIds.front().c_str(), (consumedIds.size() > 1) ? ", ..." : "", leaseTimeMs);

            for (size_t i = 0; i < consumedIds.size(); i += kBlockSize)
            {
                auto count = std::min(kBlockSize, consumedIds.size() - i);
                std::vector<uint8_t> idList = packageIdList(consumedIds.begin() + i, consumedIds.begin() + i + count);
                if (!SqliteStatement(*m_db, m_stmtReserveEvents).execute(idList, PAL::getUtcSystemTimeMs() + leaseTimeMs))
                {
                    LOG_ERROR("Failed to reserve events to send: Database error occurred, recreating database");
                    recreate(207);
                    return false;
                }
            }
            m_lastReadCount = static_cast<unsigned>(consumedIds.size());
        }
        return true;
    }

    bool OfflineStorage_SQLite::IsLastReadFromMemory()
    {
        return false;
    }

    unsigned OfflineStorage_SQLite::LastReadRecordCount()
    {
        return  m_lastReadCount;
    }

    std::vector<StorageRecord> OfflineStorage_SQLite::GetRecords(bool shutdown, EventLatency minLatency, unsigned maxCount)
    {
        std::vector<StorageRecord> records;
        StorageRecord record;

        if (!isOpen()) {
            return records;
        }

        if (shutdown)
        {
            SqliteStatement selectStmt(*m_db, m_stmtSelectEventAtShutdown);
            if (selectStmt.select(static_cast<int>(minLatency), maxCount > 0 ? maxCount : -1))
            {
                int latency;
                while (selectStmt.getRow(record.id, record.tenantToken, latency, record.timestamp, record.retryCount, record.reservedUntil, record.blob))
                {
                    record.latency = static_cast<EventLatency>(latency);
                    records.push_back(record);
                }
                selectStmt.reset();
            }
        }
        else
        {
            SqliteStatement selectStmt(*m_db, m_stmtSelectEventsMinlatency);
            if (selectStmt.select(static_cast<int>(minLatency), maxCount > 0 ? maxCount : -1))
            {
                int latency;
                while (selectStmt.getRow(record.id, record.tenantToken, latency, record.timestamp, record.retryCount, record.reservedUntil, record.blob))
                {
                    record.latency = static_cast<EventLatency>(latency);
                    records.push_back(record);
                }
                selectStmt.reset();
            }
        }
        return records;
    }

    void OfflineStorage_SQLite::DeleteAllRecords()
    {
        std::string sql = "DELETE FROM "  TABLE_NAME_EVENTS ;
        Execute(sql);

    }

    void OfflineStorage_SQLite::DeleteRecords(const std::map<std::string, std::string> & whereFilter)
    {
        UNREFERENCED_PARAMETER(whereFilter);
        if (!isOpen()) {
            return;
        }

        LOCKGUARD(m_lock);
        {
#ifdef ENABLE_LOCKING
            DbTransaction transaction(m_db.get());
            if (!transaction.locked)
            {
                LOG_ERROR("Failed to DeleteRecords");
                return;
            }
#endif
            auto formatter = [&](const std::map<std::string, std::string> & whereFilter)
            {
                std::string clause;
                for (const auto &kv : whereFilter)
                {
                    bool quotes = false;
                    if ((kv.first == "record_id") ||
                        (kv.first == "tenant_token"))
                    {
                        // string types
                        quotes = true;
                    } 
                    else if (
                        // integer types
                        (kv.first == "latency") ||
                        (kv.first == "persistence") ||
                        (kv.first == "retry_count"))
                    {
                        quotes = false;
                    }
                    if (!clause.empty())
                    {
                        clause += " AND ";
                    }
                    clause += kv.first;
                    clause += "=";
                    clause += (quotes) ?
                        ("\"" + kv.second + "\"") :
                        kv.second;
                }
                return clause;
            };
            std::string sql = "DELETE FROM " TABLE_NAME_EVENTS " WHERE ";
            Execute(sql + formatter(whereFilter));
        }
    }

    void OfflineStorage_SQLite::DeleteRecords(std::vector<StorageRecordId> const& ids, HttpHeaders headers, bool& fromMemory)
    {
        UNREFERENCED_PARAMETER(fromMemory);
        UNREFERENCED_PARAMETER(headers); // could be unused

        if (ids.empty()) {
            return;
        }

        if (!m_db) {
            LOG_ERROR("Failed to delete %u sent event(s) {%s%s}: Database is not open",
                static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "");
            return;
        }

        /* ============================================================================================================= */
        LOCKGUARD(m_lock);
        {
#ifdef ENABLE_LOCKING
            DbTransaction transaction(m_db.get());
            if (!transaction.locked)
            {
                LOG_ERROR("Failed to DeleteRecords");
                return;
            }
#endif
            LOG_TRACE("Deleting %u sent event(s) {%s%s}...", static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "");

            for (size_t i = 0; i < ids.size(); i += kBlockSize) {
                size_t count = std::min(kBlockSize, ids.size() - i);
                std::vector<uint8_t> idList = packageIdList(ids.begin() + i,
                                                            ids.begin() + i + count);
                if (!SqliteStatement(*m_db, m_stmtDeleteEvents_ids).execute(idList)) {
                    LOG_ERROR(
                            "Failed to delete %u sent event(s) {%s%s}: Database error occurred, recreating database",
                            static_cast<unsigned>(ids.size()), ids.front().c_str(),
                            (ids.size() > 1) ? ", ..." : "");
                    recreate(302);
                    return;
                }
            }
        }
    }

    void OfflineStorage_SQLite::ReleaseRecords(std::vector<StorageRecordId> const& ids, bool incrementRetryCount, HttpHeaders headers, bool& fromMemory)
    {
        UNREFERENCED_PARAMETER(fromMemory);
        UNREFERENCED_PARAMETER(headers); // could be unused

        if (ids.empty()) {
            return;
        }
        if (!m_db) {
            LOG_ERROR("Failed to release %u event(s) {%s%s}, retry count %s: Database is not open",
                static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "", incrementRetryCount ? "+1" : "not changed");
            return;
        }

        LOCKGUARD(m_lock);
        {
#ifdef ENABLE_LOCKING
            DbTransaction transaction(m_db.get());
            if (!transaction.locked)
            {
                LOG_ERROR("Failed to ReleaseRecords");
                return;
            }
#endif
            LOG_TRACE("Releasing %u event(s) {%s%s}, retry count %s...",
                static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "", incrementRetryCount ? "+1" : "not changed");

            SqliteStatement releaseStmt(*m_db, m_stmtReleaseEvents_ids_retryCountDelta);
            for (size_t i = 0; i < ids.size(); i += kBlockSize) {
                size_t count = std::min(kBlockSize, ids.size() - i);
                std::vector<uint8_t> idList = packageIdList(ids.begin() + i, ids.begin() + i + count);
                if (!releaseStmt.execute(idList, incrementRetryCount ? 1 : 0)) {
                    LOG_ERROR(
                            "Failed to release %u event(s) {%s%s}, retry count %s: Database error occurred, recreating database",
                            static_cast<unsigned>(ids.size()), ids.front().c_str(),
                            (ids.size() > 1) ? ", ..." : "",
                            incrementRetryCount ? "+1" : "not changed");
                    recreate(403);
                    return;
                }
            }
            LOG_TRACE("Successfully released %u requested event(s), %u were not found anymore",
                releaseStmt.changes(), static_cast<unsigned>(ids.size()) - releaseStmt.changes());

            if (incrementRetryCount)
            {
                unsigned maxRetryCount = m_config.GetMaximumRetryCount();

                SqliteStatement getRowstobedeleteStmt(*m_db, m_stmtSelectEventsRetried_maxRetryCount);
                if (!getRowstobedeleteStmt.select(maxRetryCount)) {
                    LOG_ERROR("Failed to get events with exceeded retry count: Database error occurred, recreating database");
                    recreate(404);
                    return;
                }

                std::map<std::string, size_t> deletedData;
                std::string tenantToken;
                while (getRowstobedeleteStmt.getRow(tenantToken))
                {
                    deletedData[tenantToken]++;
                }

                getRowstobedeleteStmt.reset();

                SqliteStatement deleteStmt(*m_db, m_stmtDeleteEventsRetried_maxRetryCount);
                if (!deleteStmt.execute(maxRetryCount)) {
                    LOG_ERROR("Failed to delete events with exceeded retry count: Database error occurred, recreating database");
                    recreate(404);
                    return;
                }

                unsigned droppedCount = deleteStmt.changes();
                if (droppedCount > 0)
                {
                    LOG_ERROR("Deleted %u events over maximum retry count %u",
                        droppedCount, maxRetryCount);
                    m_observer->OnStorageRecordsDropped(deletedData);
                }
            }
        }

    }

    bool OfflineStorage_SQLite::StoreSetting(std::string const& name, std::string const& value)
    {
        if (name.empty()) {
            LOG_ERROR("Failed to set setting \"%s\": Name cannot be empty", name.c_str());
            return false;
        }

        if (!m_db) {
            LOG_ERROR("Failed to set setting \"%s\": Database is not open", name.c_str());
            return false;
        }

        if (!value.empty()) {
            if (!SqliteStatement(*m_db, m_stmtInsertSetting_name_value).execute(name, value)) {
                LOG_ERROR("Failed to set setting \"%s\": Database error occurred, recreating database", name.c_str());
                recreate(502);
                return false;
            }
        }
        else {
            if (!SqliteStatement(*m_db, m_stmtDeleteSetting_name).execute(name)) {
                LOG_ERROR("Failed to set setting \"%s\": Database error occurred, recreating database", name.c_str());
                recreate(503);
                return false;
            }
        }

        return true;
    }

    std::string OfflineStorage_SQLite::GetSetting(std::string const& name)
    {
        std::string result;

        if (name.empty()) {
            LOG_ERROR("Failed to get setting \"%s\": Name cannot be empty", name.c_str());
            return result;
        }

        if (!isOpen()) {
            LOG_ERROR("Oddly closed");
            return result;
        }
        {
#ifdef ENABLE_LOCKING
            DbTransaction transaction(m_db.get());
            if (!transaction.locked)
            {
                LOG_WARN("Failed to get setting \"%s\"", name.c_str());
                return result;
            }
#endif
            SqliteStatement stmt(*m_db, m_stmtSelectSetting_name);
            if (!stmt.select(name)) {
                LOG_WARN("Failed to get setting \"%s\"", name.c_str());
                return result;
            }
            stmt.getOneValue(result);
        }

        return result;
    }

    bool OfflineStorage_SQLite::DeleteSetting(std::string const& name)
    {
        if (name.empty()) {
            LOG_ERROR("Failed to delete setting \"%s\": Name cannot be empty", name.c_str());
            return false;
        }
        if (!isOpen()) {
            LOG_ERROR("Oddly closed");
            return false;
        }
#ifdef ENABLE_LOCKING
        DbTransaction transaction(m_db.get());
        if (!transaction.locked)
        {
            LOG_WARN("Failed to delete setting \"%s\"", name.c_str());
            return false;
        }
#endif
        if(!SqliteStatement(*m_db, m_stmtDeleteSetting_name).execute(name))
        {
            LOG_ERROR("Failed to delete setting \"%s\": Database error occurred, recreating database", name.c_str());
            return false;
        }
        return true;
    }

    bool OfflineStorage_SQLite::recreate(unsigned failureCode)
    {
        m_observer->OnStorageFailed(toString(failureCode));

        if (m_db)
        {
            m_db->shutdown();
            // Try again with deletePrevious = true
            if (m_db->initialize(m_offlineStorageFileName, true)) {
                if (initializeDatabase()) {
                    m_observer->OnStorageOpened("SQLite/Clean");
                    LOG_INFO("Using configured on-disk database after deleting the existing one");
                    m_isOpened = true;
                    return true;
                }
                m_db->shutdown();
            }
        }

        m_isOpened = false;
        LOG_ERROR("No database could be opened");
        m_observer->OnStorageOpened("SQLite/None");
        return false;
    }

    bool OfflineStorage_SQLite::initializeDatabase()
    {
        SqliteStatement(*m_db, "PRAGMA auto_vacuum=FULL").select();
        SqliteStatement(*m_db, "PRAGMA journal_mode=WAL").select();
        SqliteStatement(*m_db, "PRAGMA synchronous=NORMAL").select();
        {
            std::ostringstream tempPragma;
            tempPragma << "PRAGMA temp_store_directory = '" << GetTempDirectory() << "'";
            SqliteStatement(*m_db, tempPragma.str().c_str()).select();
            LOG_INFO("Set sqlite3 temp_store_directory to '%s'", sqlite3_temp_directory);
        }

        int openedDbVersion;
        {
            SqliteStatement stmt(*m_db, "PRAGMA user_version");
            if (!stmt.select() || !stmt.getRow(openedDbVersion)) { return false; }
        }

        if (openedDbVersion != CURRENT_SCHEMA_VERSION) {
            if (openedDbVersion == 0) {
                LOG_TRACE("No stored version found, assuming fresh database");
            }
            else if (openedDbVersion < CURRENT_SCHEMA_VERSION) {
                LOG_INFO("Database has older version %d, upgrading to %d",
                    openedDbVersion, CURRENT_SCHEMA_VERSION);
            }
            else {
                LOG_WARN("Database version %d is newer than current %d, erasing and replacing with new",
                    openedDbVersion, CURRENT_SCHEMA_VERSION);
                return false;
            }
            if (!SqliteStatement(*m_db,
                ("PRAGMA user_version=" + toString(CURRENT_SCHEMA_VERSION)).c_str()
            ).execute()) {
                return false;
            }
        }

        if (!SqliteStatement(*m_db,
            "CREATE TABLE IF NOT EXISTS " TABLE_NAME_EVENTS " ("
            "record_id"      " TEXT,"
            "tenant_token"   " TEXT NOT NULL,"
            "latency"        " INTEGER,"
            "persistence"    " INTEGER,"
            "timestamp"      " INTEGER,"
            "retry_count"    " INTEGER DEFAULT 0,"
            "reserved_until" " INTEGER DEFAULT 0,"
            "payload"        " BLOB"
            ")"
        ).execute()) {
            return false;
        }

        if (!SqliteStatement(*m_db,
            "CREATE INDEX IF NOT EXISTS k_latency_timestamp ON " TABLE_NAME_EVENTS
            " (latency DESC, persistence DESC, timestamp ASC)"
        ).execute()) {
            return false;
        }

        if (!SqliteStatement(*m_db,
            "CREATE TABLE IF NOT EXISTS " TABLE_NAME_SETTINGS " ("
            "name"  " TEXT,"
            "value" " TEXT,"
            " PRIMARY KEY (name))"
        ).execute()) {
            return false;
        }

        {
            SqliteStatement stmt(*m_db, "PRAGMA page_size");
            if (!stmt.select() || !stmt.getRow(m_pageSize)) { return false; }
        }

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4296) // expression always false.
#elif defined( __clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtype-limits" // error: comparison of unsigned expression < 0 is always false [-Werror=type-limits]
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"  // error: comparison of unsigned expression < 0 is always false [-Werror=type-limits]
#endif

#define PREPARE_SQL(var_, stmt_) \
    if ((var_ = m_db->prepare(stmt_)) < 0) { return false; }

#ifdef ENABLE_LOCKING
        PREPARE_SQL(m_stmtBeginTransaction,
            "BEGIN IMMEDIATE");
        PREPARE_SQL(m_stmtCommitTransaction,
            "COMMIT");
        PREPARE_SQL(m_stmtRollbackTransaction,
            "ROLLBACK");
#endif

        PREPARE_SQL(m_stmtGetPageCount,
            "PRAGMA page_count");

        PREPARE_SQL(m_stmtGetRecordCount,
            "SELECT count(*) FROM " TABLE_NAME_EVENTS);
        PREPARE_SQL(m_stmtGetRecordCountBylatency,
            "SELECT count(*) FROM " TABLE_NAME_EVENTS " WHERE latency=?");

        PREPARE_SQL(m_stmtPerTenantTrimCount,
            "SELECT tenant_token FROM " TABLE_NAME_EVENTS " ORDER BY persistence ASC, timestamp ASC LIMIT MAX(1,"
            "(SELECT COUNT(record_id) FROM " TABLE_NAME_EVENTS ")"
            "* ? / 100)");
        PREPARE_SQL(m_stmtTrimEvents_percent,
                    "DELETE FROM " TABLE_NAME_EVENTS " WHERE record_id IN ("
                                                     "SELECT record_id FROM " TABLE_NAME_EVENTS " ORDER BY persistence ASC, timestamp ASC LIMIT MAX(1,"
                                                                                                "(SELECT COUNT(record_id) FROM " TABLE_NAME_EVENTS ")"
                                                                                                                                                   "* ? / 100)"
                                                                                                                                                   ")");

        PREPARE_SQL(m_stmtDeleteEvents_tenants,
                SQL_SUPPLY_PACKAGED_IDS
                "DELETE FROM " TABLE_NAME_EVENTS " WHERE tenant_token IN ids");
        PREPARE_SQL(m_stmtDeleteEvents_ids,
            SQL_SUPPLY_PACKAGED_IDS
            "DELETE FROM " TABLE_NAME_EVENTS " WHERE record_id IN ids");
        PREPARE_SQL(m_stmtReleaseExpiredEvents,
            "UPDATE " TABLE_NAME_EVENTS
            " SET reserved_until=0, retry_count=retry_count+1"
            " WHERE reserved_until<>0 AND reserved_until<=?");
        PREPARE_SQL(m_stmtSelectEvents,
            "SELECT record_id,tenant_token,latency,timestamp,retry_count,reserved_until,payload"
            " FROM " TABLE_NAME_EVENTS
            " WHERE latency>=? AND reserved_until=0"
            " ORDER BY latency DESC,persistence DESC, timestamp ASC LIMIT ?");
        PREPARE_SQL(m_stmtSelectEventAtShutdown,
            "SELECT record_id,tenant_token,latency,timestamp,retry_count,reserved_until,payload"
            " FROM " TABLE_NAME_EVENTS
            " WHERE latency>=?"
            " ORDER BY latency DESC,persistence DESC, timestamp ASC LIMIT ?");
        PREPARE_SQL(m_stmtSelectEventsMinlatency,
            "SELECT record_id,tenant_token,latency,timestamp,retry_count,reserved_until,payload"
            " FROM " TABLE_NAME_EVENTS
            " WHERE latency=(SELECT MIN(latency) FROM " TABLE_NAME_EVENTS " WHERE reserved_until=0 AND latency>=?) AND reserved_until=0"
            " ORDER BY timestamp ASC LIMIT ?");

        PREPARE_SQL(m_stmtReserveEvents,
            SQL_SUPPLY_PACKAGED_IDS
            "UPDATE " TABLE_NAME_EVENTS
            " SET reserved_until=?"
            " WHERE record_id IN ids");
        PREPARE_SQL(m_stmtReleaseEvents_ids_retryCountDelta,
            SQL_SUPPLY_PACKAGED_IDS
            "UPDATE " TABLE_NAME_EVENTS
            " SET reserved_until=0, retry_count=retry_count+?"
            " WHERE record_id IN ids AND reserved_until>0");
        PREPARE_SQL(m_stmtSelectEventsRetried_maxRetryCount,
            "SELECT tenant_token FROM " TABLE_NAME_EVENTS
            " WHERE retry_count>?");
        PREPARE_SQL(m_stmtDeleteEventsRetried_maxRetryCount,
            "DELETE FROM " TABLE_NAME_EVENTS
            " WHERE retry_count>?");
        PREPARE_SQL(m_stmtInsertEvent_id_tenant_prio_ts_data,
            "REPLACE INTO " TABLE_NAME_EVENTS " (record_id,tenant_token,latency,persistence,timestamp,payload) VALUES (?,?,?,?,?,?)");
        PREPARE_SQL(m_stmtInsertSetting_name_value,
            "REPLACE INTO " TABLE_NAME_SETTINGS " (name,value) VALUES (?,?)");
        PREPARE_SQL(m_stmtDeleteSetting_name,
            "DELETE FROM " TABLE_NAME_SETTINGS " WHERE name=?");
        PREPARE_SQL(m_stmtSelectSetting_name,
            "SELECT value FROM " TABLE_NAME_SETTINGS " WHERE name=?");

        /* Delete v1 records */
        Execute("DELETE FROM " TABLE_NAME_PACKAGES);

#undef PREPARE_SQL

#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

        ResizeDb();
        return true;
}

    size_t OfflineStorage_SQLite::GetSize()
    {
        if (!m_db) {
            LOG_ERROR("Failed to get DB size: database is not open");
            return 0;
        }

        LOCKGUARD(m_lock);
        unsigned pageCount = 0;
        SqliteStatement pageCountStmt(*m_db, m_stmtGetPageCount);
        if (!pageCountStmt.select())
        {
            LOG_TRACE("Failed to get DB size: database is busy");
            return 0;
        }
        pageCountStmt.getRow(pageCount);
        pageCountStmt.reset();
        return size_t(pageCount) * size_t(m_pageSize);
    }

    size_t OfflineStorage_SQLite::GetRecordCountUnsafe(EventLatency latency) const
    {
        int count = 0;
        if (latency == EventLatency_Unspecified)
        {
            SqliteStatement recordCount(*m_db, m_stmtGetRecordCount);
            recordCount.select();
            recordCount.getOneValue(count);
            recordCount.reset();
        }
        else
        {
            SqliteStatement recordCount(*m_db, m_stmtGetRecordCountBylatency);
            recordCount.select(latency);
            recordCount.getOneValue(count);
            recordCount.reset();
        }
        return count;
    }

    size_t OfflineStorage_SQLite::GetRecordCount(EventLatency latency = EventLatency_Unspecified) const
    {
        if (!m_db) {
            LOG_ERROR("Failed to get DB size: database is not open");
            return 0;
        }

        LOCKGUARD(m_lock);
        return OfflineStorage_SQLite::GetRecordCountUnsafe(latency);
    }

    bool OfflineStorage_SQLite::ResizeDb()
    {
        if (!m_db) {
            LOG_ERROR("Failed to resize DB: database is not open");
            return false;
        }

        size_t eventsDropped = 0;
        m_DbSizeEstimate = GetSize();
        if (m_DbSizeEstimate <= m_DbSizeLimit)
            return false;

        LOCKGUARD(m_lock);
        {
#ifdef ENABLE_LOCKING
            DbTransaction transaction(m_db.get());
            if (!transaction.locked)
            {
                LOG_WARN("Failed to trim database");
                return false;
            }
#endif
            auto count = GetRecordCountUnsafe(EventLatency::EventLatency_Unspecified);
            if (m_DbSizeEstimate > 2 * m_DbSizeLimit)
            {
                LOG_TRACE("DB is too big, deleting...");
                Execute("DELETE FROM " TABLE_NAME_EVENTS);
                Execute("VACUUM");
                return true;
            }

            SqliteStatement trimStmt(*m_db, m_stmtTrimEvents_percent);
            if (!trimStmt.execute(25))
            {
                // If something went wrong with trimming 25%, try more radical measure
                LOG_TRACE("Evict all non-critical");
                Execute("DELETE FROM " TABLE_NAME_EVENTS " WHERE persistence=1");
            }
            eventsDropped = count - GetRecordCountUnsafe(EventLatency::EventLatency_Unspecified);
            LOG_TRACE("Db resized, events dropeed: %d", eventsDropped);
            trimStmt.reset();
        }

        m_DbSizeEstimate = GetSize();
        DebugEvent evt(DebugEventType::EVT_DROPPED);
        evt.param1 = eventsDropped;
        evt.size = eventsDropped;
        m_logManager.DispatchEvent(evt);

        return true;
    }

    std::vector<uint8_t> OfflineStorage_SQLite::packageIdList(
        std::vector<std::string>::const_iterator const & begin,
        std::vector<std::string>::const_iterator const & end) const
    {
        size_t size = std::accumulate(begin, end, size_t(0), [](size_t sum, std::string const& id) -> size_t {
            return sum + id.length() + 1;
        });

        std::vector<uint8_t> result;
        result.reserve(size);

        for (auto i = begin; i != end; ++i)
        {
            std::string const & id(*i);
            uint8_t const* ptr = reinterpret_cast<uint8_t const*>(id.c_str());
            result.insert(result.end(), ptr, ptr + id.size() + 1);
        }

        return result;
    }
    
} MAT_NS_END
#endif

