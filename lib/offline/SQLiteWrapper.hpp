//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef SQLITEWRAPPER_HPP
#define SQLITEWRAPPER_HPP

#include "pal/PAL.hpp"

#include "sqlite3.h"
#include "ISqlite3Proxy.hpp"

#include <algorithm>
#include <map>
#include <mutex>
#include <vector>
#include <string>

namespace MAT_NS_BEGIN {

    using SQLRecord = std::vector<std::string>;
    using SQLRecords = std::vector<SQLRecord>;

    static const unsigned MAX_DB_LOCKWAIT_DELAY = 500; // 500 ms

    class RealSqlite3Proxy : public ISqlite3Proxy {
    public:

        int sqlite3_bind_blob(sqlite3_stmt* stmt, int idx, void const* value, int size, void(*d)(void*)) override
        {
            return ::sqlite3_bind_blob(stmt, idx, value, size, d);
        }

        int sqlite3_bind_int(sqlite3_stmt* stmt, int idx, int value) override
        {
            return ::sqlite3_bind_int(stmt, idx, value);
        }

        int sqlite3_bind_int64(sqlite3_stmt* stmt, int idx, int64_t value) override
        {
            return ::sqlite3_bind_int64(stmt, idx, value);
        }

        int sqlite3_bind_text(sqlite3_stmt* stmt, int idx, char const* value, int size, void(*d)(void*)) override
        {
            return ::sqlite3_bind_text(stmt, idx, value, size, d);
        }

        int sqlite3_changes(sqlite3* db) override
        {
            return ::sqlite3_changes(db);
        }

        int sqlite3_clear_bindings(sqlite3_stmt* stmt) override
        {
            return ::sqlite3_clear_bindings(stmt);
        }

        int sqlite3_close(sqlite3* db) override
        {
            return ::sqlite3_close(db);
        }

        int sqlite3_close_v2(sqlite3* db) override
        {
            return ::sqlite3_close_v2(db);
        }

        void const* sqlite3_column_blob(sqlite3_stmt* stmt, int iCol) override
        {
            return ::sqlite3_column_blob(stmt, iCol);
        }

        int sqlite3_column_bytes(sqlite3_stmt* stmt, int iCol) override
        {
            return ::sqlite3_column_bytes(stmt, iCol);
        }

        int sqlite3_column_int(sqlite3_stmt* stmt, int iCol) override
        {
            return ::sqlite3_column_int(stmt, iCol);
        }

        int64_t sqlite3_column_int64(sqlite3_stmt* stmt, int iCol) override
        {
            return ::sqlite3_column_int64(stmt, iCol);
        }

        unsigned const char* sqlite3_column_text(sqlite3_stmt* stmt, int iCol) override
        {
            return ::sqlite3_column_text(stmt, iCol);
        }

        int sqlite3_create_function_v2(sqlite3* db, char const* zFunctionName, int nArg, int eTextRep, void* pApp,
            void(*xFunc)(sqlite3_context*, int, sqlite3_value**), void(*xStep)(sqlite3_context*, int, sqlite3_value**),
            void(*xFinal)(sqlite3_context*), void(*xDestroy)(void*)) override
        {
            return ::sqlite3_create_function_v2(db, zFunctionName, nArg, eTextRep, pApp, xFunc, xStep, xFinal, xDestroy);
        }

        char const* sqlite3_errmsg(sqlite3* db) override
        {
            return ::sqlite3_errmsg(db);
        }

        int sqlite3_extended_result_codes(sqlite3* db, int on) override
        {
            return ::sqlite3_extended_result_codes(db, on);
        }

        int sqlite3_finalize(sqlite3_stmt* stmt) override
        {
            return ::sqlite3_finalize(stmt);
        }

        void* sqlite3_get_auxdata(sqlite3_context* ctx, int N) override
        {
            return ::sqlite3_get_auxdata(ctx, N);
        }

        int sqlite3_initialize() override
        {
            return ::sqlite3_initialize();
        }

        int sqlite3_open_v2(char const* file, sqlite3** pdb, int flags, char const* zvfs) override
        {
            assert(file != nullptr);    // Don't allow nullptr filename
            assert(file[0] != 0);       // Don't allow empty   filename
            return ::sqlite3_open_v2(file, pdb, flags, zvfs);
        }

        int sqlite3_prepare_v2(sqlite3* db, char const* zsql, int size, sqlite3_stmt** pstmt, char const** pztail) override
        {
            return ::sqlite3_prepare_v2(db, zsql, size, pstmt, pztail);
        }

        int sqlite3_reset(sqlite3_stmt* stmt) override
        {
            return ::sqlite3_reset(stmt);
        }

        void sqlite3_result_null(sqlite3_context* ctx) override
        {
            return ::sqlite3_result_null(ctx);
        }

        void sqlite3_result_text(sqlite3_context* ctx, char const* value, int size, void(*d)(void*)) override
        {
            return ::sqlite3_result_text(ctx, value, size, d);
        }

        void sqlite3_set_auxdata(sqlite3_context* ctx, int N, void* data, void(*d)(void*)) override
        {
            return ::sqlite3_set_auxdata(ctx, N, data, d);
        }

        int sqlite3_shutdown() override
        {
            return ::sqlite3_shutdown();
        }

        int sqlite3_step(sqlite3_stmt* stmt) override
        {
            return ::sqlite3_step(stmt);
        }

        int64_t sqlite3_soft_heap_limit64(int64_t N) override
        {
            return ::sqlite3_soft_heap_limit64((sqlite3_int64)N);
        }

        void const* sqlite3_value_blob(sqlite3_value* value) override
        {
            return ::sqlite3_value_blob(value);
        }

        int sqlite3_value_bytes(sqlite3_value* value) override
        {
            return ::sqlite3_value_bytes(value);
        }

        sqlite3_vfs* sqlite3_vfs_find(char const* zVfsName) override
        {
            return ::sqlite3_vfs_find(zVfsName);
        }
    } g_realSqlite3Proxy;

    ISqlite3Proxy* g_sqlite3Proxy = &g_realSqlite3Proxy;

    //---

    /// Provide virtual table 'ids' filled from NUL-separated list parameter
    /// (input string is tokenized with a custom function 'tokenize')
#define SQL_SUPPLY_PACKAGED_IDS    \
    "WITH RECURSIVE ids(id) AS ("  \
    "SELECT 0 "                    \
    "UNION ALL "                   \
    "SELECT tokenize(?) FROM ids " \
    "WHERE id IS NOT NULL "        \
    "LIMIT 10000 OFFSET 1"         \
    ") "

    class SqliteDB {
        std::mutex m_lock;
    public:
        SqliteDB(bool skipInitAndShutdown,
                 std::mutex* initAndShutdownLock = nullptr,
                 int* instanceCount = nullptr)
            : m_db(nullptr),
              m_skipInitAndShutdown(skipInitAndShutdown),
              m_initAndShutdownLock(initAndShutdownLock),
              m_instanceCount(instanceCount)
        {
        }

        bool initialize(std::string const& filename, bool deletePrevious, size_t maxHeapLimit = 0)
        {
            int result = SQLITE_OK;

            if (!m_skipInitAndShutdown) {
                if (m_initAndShutdownLock && m_instanceCount)
                {
                    LOCKGUARD(*m_initAndShutdownLock);
                    if (*m_instanceCount > 0) {
                        *m_instanceCount += 1;
                    } else {
                        result = g_sqlite3Proxy->sqlite3_initialize();
                        if (result == SQLITE_OK) {
                            *m_instanceCount = 1;
                        }
                    }
                } else {
                    result = g_sqlite3Proxy->sqlite3_initialize();
                }

                if (result != SQLITE_OK) {
                    LOG_ERROR("Failed to initialize SQLite (%d)", result);
                    return false;
                }
            }

            if (deletePrevious) {
                // We cannot call plain ::remove() here, filename is in UTF-8. Rather
                // than adding a new set of functions to PAL, let's use SQLite VFS.
                sqlite3_vfs* vfs = g_sqlite3Proxy->sqlite3_vfs_find(NULL);
                result = (vfs != NULL) ? vfs->xDelete(vfs, filename.c_str(), 0) : SQLITE_ERROR;
                if (result == SQLITE_OK) {
                    LOG_INFO("Unusable existing database file was successfully deleted");
                }
                else if (result != SQLITE_IOERR_DELETE_NOENT) {
                    LOG_WARN("Failed to delete unusable database file (%d)", result);
                    shutdown_sqlite();
                    return false;
                }
            }

            // Take basename only, potential PII like profile name in the path must not be logged
            size_t ofs = filename.find_last_of("/\\", std::string::npos);
            std::string basename(filename, (ofs != std::string::npos) ? (ofs + 1) : 0);
            LOG_INFO("Opening database \"%s\"...", basename.c_str());

            result = g_sqlite3Proxy->sqlite3_open_v2(filename.c_str(), &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL);
           if (result != SQLITE_OK) {
                LOG_ERROR("Failed to open database file: (%d) %s",
                    result, m_db ? g_sqlite3Proxy->sqlite3_errmsg(m_db) : "-");
                if (m_db) {
                    g_sqlite3Proxy->sqlite3_close_v2(m_db);
                    m_db = nullptr;
                }
                shutdown_sqlite();
                return false;
            }

            g_sqlite3Proxy->sqlite3_extended_result_codes(m_db, 1);

            if (!registerTokenizeFunction()) {
                shutdown();
                return false;
            }

            if (maxHeapLimit) {
                g_sqlite3Proxy->sqlite3_soft_heap_limit64(maxHeapLimit);
            }

            LOG_TRACE("Database file was successfully opened");
            return true;
        }

        void shutdown_sqlite()
        {
            if (!m_skipInitAndShutdown)
            {
                if (m_initAndShutdownLock && m_instanceCount)
                {
                    LOCKGUARD(*m_initAndShutdownLock);
                    if (*m_instanceCount > 1) {
                        *m_instanceCount -= 1;
                    } else if (*m_instanceCount == 1) {
                        *m_instanceCount = 0;
                        g_sqlite3Proxy->sqlite3_shutdown();
                    }
                } else
                {
                    g_sqlite3Proxy->sqlite3_shutdown();
                }
            }
        }

        void shutdown()
        {
            if (m_db == nullptr) {
                return;
            }

            LOG_TRACE("Closing database");

            for (sqlite3_stmt* stmt : m_statements) {
                if (stmt != nullptr) {
                    g_sqlite3Proxy->sqlite3_finalize(stmt);
                }
            }
            m_statements.clear();

            g_sqlite3Proxy->sqlite3_close_v2(m_db);
            m_db = nullptr;
            shutdown_sqlite();
        }

        size_t prepare(char const* statement)
        {
            LOCKGUARD(m_lock);
            sqlite3_stmt* stmt;
            int result = g_sqlite3Proxy->sqlite3_prepare_v2(m_db, statement, -1, &stmt, NULL);
            if (result != SQLITE_OK) {
                std::string excerpt(statement);
                if (excerpt.length() > 100) {
                    excerpt.resize(100);
                    excerpt.append("...");
                }
                LOG_ERROR("Failed to prepare SQL statement \"%s\": %d (%s)",
                    excerpt.c_str(), result, g_sqlite3Proxy->sqlite3_errmsg(m_db));
                return 0;
            }
            m_statements.push_back(stmt);
            LOG_INFO("+++ [%p] = %s", stmt, statement);
            return (size_t)(stmt);
        }

        sqlite3_stmt* statement(size_t stmtId)
        {
            return (sqlite3_stmt*)stmtId;
        }

        void release(size_t stmtId)
        {
            sqlite3_stmt* stmt = statement(stmtId);
            {
                LOCKGUARD(m_lock);
                auto it = std::find(m_statements.begin(), m_statements.end(), statement(stmtId));
                if (it != std::end(m_statements))
                {
                    m_statements.erase(it);
                    g_sqlite3Proxy->sqlite3_finalize(stmt);
                }
            }
        }

        operator sqlite3*()
        {
            return m_db;
        }
        
        /// Dummy sqlite3 callback
        static int sqlite3_noop_callback(void *, int, char **, char **) {
            return 0;
        }

        // sqlite3 callback to translate result set into vector of vectors of string
        static int sqlite3_select_callback(void *p_data, int num_fields, char **p_fields, char **p_col_names)
        {
            UNREFERENCED_PARAMETER(p_col_names);
            SQLRecords* records = static_cast<SQLRecords*>(p_data);
            try {
                records->emplace_back(p_fields, p_fields + num_fields);
            }
            catch (...) {
                return 1;
            }
            return 0;
        }
        
        /// <summary>
        /// Check if return code is OK
        /// </summary>
        /// <param name="rc">sqlite3 return code</param>
        /// <param name="zErrMsg">Error message to print if not OK</param>
        /// <returns>
        ///   <c>true</c> if the specified rc is ok; otherwise, <c>false</c>.
        /// </returns>
        bool isOK(int rc, char *zErrMsg = nullptr)
        {
            if (rc != SQLITE_OK) {
                LOG_DEBUG("rc=%u: %s", rc, (zErrMsg != nullptr) ? zErrMsg : sqlite3_errmsg(m_db));
                if (zErrMsg) {
                    ::sqlite3_free(zErrMsg);
                }
            }
            return (rc == SQLITE_OK);
        }

        int sqlite3_exec(const char *sql, int(*callback)(void*, int, char**, char**) = sqlite3_noop_callback, void *arg = nullptr) {
            char *errmsg = nullptr;
            int result = 0;
            LOG_DEBUG("%s", sql);
            result = ::sqlite3_exec(m_db, sql, callback, arg, &errmsg);
            if (!isOK(result, errmsg))
            {
                LOG_DEBUG("Failed to execute query: %s [rc=%d]", sql, result);
            }
            return result;
        }

        bool trylock() {
            return isOK(sqlite3_exec("BEGIN EXCLUSIVE;")); 
        }

        /**
        * @fn  void SQLiteStorage::Unlock()
        *
        * @brief   Release exclusive DB lock.
        */
        bool unlock() {
            return isOK(sqlite3_exec("COMMIT;"));
        }

        bool lock() {
            unsigned count = 0;
            unsigned waitTime = 0;
            while (!trylock()) {
                if (waitTime >= MAX_DB_LOCKWAIT_DELAY) {
                    // We try 3 times: at 0ms, 500ms, 1000ms. If we still can't acquire the DB lock,
                    // then something real bad happening with DB at the moment. So we have to fail
                    // the pending operation and in worst case that will impact the current pending
                    // DB transaction.
                    LOG_ERROR("Cannot acquire the DB lock for %u ms", waitTime);
                    return false;
                }
                waitTime += MAX_DB_LOCKWAIT_DELAY;  // 500ms, 1000ms
                count++;
                LOG_DEBUG("Lock: waiting to acquire the lock: count=%u, waitTime=%u", count, waitTime);
                PAL::sleep(MAX_DB_LOCKWAIT_DELAY);
            }
            LOG_DEBUG("Lock: acquired [time=%u]", waitTime);
            return true;
        }

        SQLRecords execute(const char* sql)
        {
            SQLRecords records;
            sqlite3_exec(sql, sqlite3_select_callback, &records);
            return records;
        }

    protected:

        static void sqliteFunc_tokenize(sqlite3_context* ctx, int argc, sqlite3_value** argv)
        {
            UNREFERENCED_PARAMETER(argc);
            int len = g_sqlite3Proxy->sqlite3_value_bytes(argv[0]);
            int ofs = static_cast<int>(reinterpret_cast<intptr_t>(g_sqlite3Proxy->sqlite3_get_auxdata(ctx, 0)));
            if (ofs >= len) {
                g_sqlite3Proxy->sqlite3_result_null(ctx);
                return;
            }
            char const* data = static_cast<char const*>(g_sqlite3Proxy->sqlite3_value_blob(argv[0]));
            char const* sep = static_cast<char const*>(memchr(data + ofs, 0, len - ofs));
            int pos = sep ? static_cast<int>(sep - data) : len;
            g_sqlite3Proxy->sqlite3_result_text(ctx, data + ofs, pos - ofs, SQLITE_STATIC);
            g_sqlite3Proxy->sqlite3_set_auxdata(ctx, 0, reinterpret_cast<void*>(static_cast<intptr_t>(pos + 1)), NULL);
        }

        bool registerTokenizeFunction()
        {
            int result = g_sqlite3Proxy->sqlite3_create_function_v2(m_db, "tokenize", 1, SQLITE_UTF8, NULL,
                &SqliteDB::sqliteFunc_tokenize, NULL, NULL, NULL);
            if (result != SQLITE_OK) {
                LOG_ERROR("Could not create tokenize function: (%d) %s",
                    result, g_sqlite3Proxy->sqlite3_errmsg(m_db));
                return false;
            }
            return true;
        }

    protected:
        sqlite3 * m_db;
        std::vector<sqlite3_stmt*> m_statements;
        // int                        m_statementsOffset;
        bool                       m_skipInitAndShutdown;
        std::mutex*                m_initAndShutdownLock;
        int*                       m_instanceCount;

    private:
        MATSDK_LOG_DECL_COMPONENT_CLASS();
    };

    MATSDK_LOG_INST_COMPONENT_CLASS(SqliteDB, "EventsSDK.SQLiteDB", "Events telemetry client - SqliteDB class");

    //---

    class SqliteStatement {
    public:
        SqliteStatement(SqliteDB& db, size_t stmtId)
            : m_db(db),
            m_stmtId(stmtId),
            m_stmt(db.statement(stmtId)),
            m_changes(0),
            m_duration(0),
            m_ownStmt(false),
            m_hasRow(false),
            m_done(false),
            m_error(false)
        {
            reset();
        }

        SqliteStatement(SqliteDB& db, char const* statement)
            : m_db(db),
            m_stmtId(db.prepare(statement)),
            m_stmt(db.statement(m_stmtId)),
            m_changes(0),
            m_duration(0),
            m_ownStmt(true),
            m_hasRow(false),
            m_done(false),
            m_error(false)
        {
            reset();
        }

        ~SqliteStatement()
        {
            if (m_ownStmt) {
                m_db.release(m_stmtId);
            }
        }

        template<typename... TArgs>
        bool execute(TArgs&& ... args)
        {
            if (m_stmt != nullptr) {
                int result = bindAll(0, std::forward<TArgs>(args) ...);
                return execute2(result);
            }
            else {
                return false;
            }
        }

        template<typename... TArgs>
        bool select(TArgs&& ... args)
        {
            if (m_stmt != nullptr) {
                int result = bindAll(0, std::forward<TArgs>(args) ...);
                return select2(result);
            }
            else {
                return false;
            }
        }

        template<typename... TResults>
        bool getRow(TResults& ... results)
        {
            if (m_stmt != nullptr) {
                return getRow2() && retrieveAll(0, results...);
            }
            else {
                return false;
            }
        }

        template<typename... TArgs, typename TResult>
        bool getOneValue(TResult& result)
        {
            return m_hasRow && retrieveAll(0, result);
        }

        void reset()
        {
            if (m_stmt != nullptr) {
                g_sqlite3Proxy->sqlite3_reset(m_stmt);
                g_sqlite3Proxy->sqlite3_clear_bindings(m_stmt);
            }
        }

        unsigned duration() const
        {
            return m_duration;
        }

        unsigned changes() const
        {
            return m_changes;
        }

        bool error() const
        {
            return m_error;
        }

    protected:
        int bind(int idx, int arg)
        {
            return g_sqlite3Proxy->sqlite3_bind_int(m_stmt, idx, arg);
        }

        int bind(int idx, unsigned arg)
        {
            return g_sqlite3Proxy->sqlite3_bind_int64(m_stmt, idx, static_cast<int64_t>(static_cast<uint64_t>(arg)));
        }

        int bind(int idx, int64_t arg)
        {
            return g_sqlite3Proxy->sqlite3_bind_int64(m_stmt, idx, arg);
        }

        int bind(int idx, std::string const& arg)
        {
            return g_sqlite3Proxy->sqlite3_bind_text(m_stmt, idx, arg.data(), static_cast<int>(arg.size()), SQLITE_STATIC);
        }

        int bind(int idx, std::vector<uint8_t> const& arg)
        {
            return g_sqlite3Proxy->sqlite3_bind_blob(m_stmt, idx, arg.data(), static_cast<int>(arg.size()), SQLITE_STATIC);
        }

        int bindAll(int idx)
        {
            UNREFERENCED_PARAMETER(idx);
            return 0;
        }

        template<typename T, typename... TArgs>
        int bindAll(int idx, T const& arg, TArgs&& ... args)
        {
            idx++;
            int result = bind(idx, arg);
            if (result != SQLITE_OK) {
                return idx;
            }
            return bindAll(idx, std::forward<TArgs>(args) ...);
        }

    protected:
        void retrieve(int idx, int& output)
        {
            output = g_sqlite3Proxy->sqlite3_column_int(m_stmt, idx);
        }

        void retrieve(int idx, unsigned& output)
        {
            output = static_cast<unsigned>(g_sqlite3Proxy->sqlite3_column_int64(m_stmt, idx));
        }

        void retrieve(int idx, int64_t& output)
        {
            output = g_sqlite3Proxy->sqlite3_column_int64(m_stmt, idx);
        }

        void retrieve(int idx, std::string& output)
        {
            int len = g_sqlite3Proxy->sqlite3_column_bytes(m_stmt, idx);
            output.assign(reinterpret_cast<char const*>(g_sqlite3Proxy->sqlite3_column_text(m_stmt, idx)), len);
        }

        void retrieve(int idx, std::vector<uint8_t>& output)
        {
            int len = g_sqlite3Proxy->sqlite3_column_bytes(m_stmt, idx);
            uint8_t const* ptr = reinterpret_cast<uint8_t const*>(g_sqlite3Proxy->sqlite3_column_blob(m_stmt, idx));
            output.assign(ptr, ptr + len);
        }

        bool retrieveAll(int idx)
        {
            UNREFERENCED_PARAMETER(idx);
            return true;
        }

        template<typename T, typename... TResults>
        bool retrieveAll(int idx, T& output, TResults& ... outputs)
        {
            retrieve(idx, output);
            return retrieveAll(idx + 1, outputs...);
        }

    protected:
        bool execute2(int bindFailedIdx)
        {
            if (bindFailedIdx > 0) {
                LOG_ERROR("Failed to bind parameter #%d of statement #[%p]: %s",
                    bindFailedIdx, m_stmtId, g_sqlite3Proxy->sqlite3_errmsg(m_db));
                m_error = true;
                return false;
            }

            auto startTime = PAL::getMonotonicTimeMs();
            LOG_DEBUG("=== [%p] execute2 step...", m_stmt);
            int result = g_sqlite3Proxy->sqlite3_step(m_stmt);
            m_duration = static_cast<unsigned>(PAL::getMonotonicTimeMs() - startTime);

            if (result == SQLITE_ROW) {
                assert(!"executed statement returned a row, use select()");
            }
            else if (result != SQLITE_DONE) {
                LOG_ERROR("Failed to modify database while executing statement [%p]: %d (%s)",
                    m_stmtId, result, g_sqlite3Proxy->sqlite3_errmsg(m_db));
                m_error = true;
            }

            m_changes = g_sqlite3Proxy->sqlite3_changes(m_db);
            reset();

            return (result == SQLITE_DONE) || (result == SQLITE_ROW);
        }

        bool select2(int bindFailedIdx)
        {
            if (bindFailedIdx > 0) {
                LOG_ERROR("Failed to bind parameter #%d of statement #[%p]: %s",
                    bindFailedIdx, m_stmtId, g_sqlite3Proxy->sqlite3_errmsg(m_db));
                m_error = true;
                return false;
            }

            int result = g_sqlite3Proxy->sqlite3_step(m_stmt);
            if (result == SQLITE_ROW) {
                m_hasRow = true;
                m_done = false;
                return true;
            }
            else if (result == SQLITE_DONE) {
                m_hasRow = false;
                m_done = true;
                return true;
            }
            else {
                LOG_ERROR("Failed to query database while executing statement #[%p]: %d (%s)",
                    m_stmtId, result, g_sqlite3Proxy->sqlite3_errmsg(m_db));
                m_error = true;
                reset();
                return false;
            }
        }

        bool getRow2()
        {
            if (m_hasRow) {
                m_hasRow = false;
                return true;
            }
            if (m_done) {
                return false;
            }

            int result = g_sqlite3Proxy->sqlite3_step(m_stmt);
            if (result == SQLITE_ROW) {
                return true;
            }

            if (result != SQLITE_DONE) {
                LOG_ERROR("Failed to read database while executing statement #[%p]: %d (%s)",
                    m_stmtId, result, g_sqlite3Proxy->sqlite3_errmsg(m_db));
                m_error = true;
            }
            reset();
            return false;
        }

    protected:
        SqliteDB      & m_db;
        size_t        m_stmtId;
        sqlite3_stmt* m_stmt;
        unsigned      m_changes;
        unsigned      m_duration;
        bool          m_ownStmt;
        bool          m_hasRow;
        bool          m_done;
        bool          m_error;

    public:
        sqlite3_stmt * handle() { return m_stmt; };

    private:
        MATSDK_LOG_DECL_COMPONENT_CLASS();
    };

    MATSDK_LOG_INST_COMPONENT_CLASS(SqliteStatement, "EventsSDK.SQLiteStatement", "Events telemetry client - Sqlite statement class");


} MAT_NS_END
#endif

