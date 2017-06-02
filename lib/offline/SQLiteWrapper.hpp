// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <Version.hpp>
#include "PAL.hpp"
#include "../../sqlite/sqlite3.h"
#include "ISqlite3Proxy.hpp"
#include <algorithm>
#include <map>

namespace ARIASDK_NS_BEGIN {


class RealSqlite3Proxy : public ISqlite3Proxy {
  public:
    int sqlite3_bind_blob(sqlite3_stmt* stmt, int idx, void const* value, int size, void (* d)(void*)) override
    { return ::sqlite3_bind_blob(stmt, idx, value, size, d); }

    int sqlite3_bind_int(sqlite3_stmt* stmt, int idx, int value) override
    { return ::sqlite3_bind_int(stmt, idx, value); }

    int sqlite3_bind_int64(sqlite3_stmt* stmt, int idx, int64_t value) override
    { return ::sqlite3_bind_int64(stmt, idx, value); }

    int sqlite3_bind_text(sqlite3_stmt* stmt, int idx, char const* value, int size, void (* d)(void*)) override
    { return ::sqlite3_bind_text(stmt, idx, value, size, d); }

    int sqlite3_changes(sqlite3* db) override
    { return ::sqlite3_changes(db); }

    int sqlite3_clear_bindings(sqlite3_stmt* stmt) override
    { return ::sqlite3_clear_bindings(stmt); }

    int sqlite3_close(sqlite3* db) override
    { return ::sqlite3_close(db); }

    void const* sqlite3_column_blob(sqlite3_stmt* stmt, int iCol) override
    { return ::sqlite3_column_blob(stmt, iCol); }

    int sqlite3_column_bytes(sqlite3_stmt* stmt, int iCol) override
    { return ::sqlite3_column_bytes(stmt, iCol); }

    int sqlite3_column_int(sqlite3_stmt* stmt, int iCol) override
    { return ::sqlite3_column_int(stmt, iCol); }

    int64_t sqlite3_column_int64(sqlite3_stmt* stmt, int iCol) override
    { return ::sqlite3_column_int64(stmt, iCol); }

    unsigned const char* sqlite3_column_text(sqlite3_stmt* stmt, int iCol) override
    { return ::sqlite3_column_text(stmt, iCol); }

    int sqlite3_create_function_v2(sqlite3* db, char const* zFunctionName, int nArg, int eTextRep, void* pApp,
        void (* xFunc)(sqlite3_context*, int, sqlite3_value**), void (* xStep)(sqlite3_context*, int, sqlite3_value**),
        void (* xFinal)(sqlite3_context*), void (* xDestroy)(void*)) override
    { return ::sqlite3_create_function_v2(db, zFunctionName, nArg, eTextRep, pApp, xFunc, xStep, xFinal, xDestroy); }

    char const* sqlite3_errmsg(sqlite3* db) override
    { return ::sqlite3_errmsg(db); }

    int sqlite3_extended_result_codes(sqlite3* db, int on) override
    { return ::sqlite3_extended_result_codes(db, on); }

    int sqlite3_finalize(sqlite3_stmt* stmt) override
    { return ::sqlite3_finalize(stmt); }

    void* sqlite3_get_auxdata(sqlite3_context* ctx, int N) override
    { return ::sqlite3_get_auxdata(ctx, N); }

    int sqlite3_initialize() override
    { return ::sqlite3_initialize(); }

    int sqlite3_open_v2(char const* file, sqlite3** pdb, int flags, char const* zvfs) override
    { return ::sqlite3_open_v2(file, pdb, flags, zvfs); }

    int sqlite3_prepare_v2(sqlite3* db, char const* zsql, int size, sqlite3_stmt** pstmt, char const** pztail) override
    { return ::sqlite3_prepare_v2(db, zsql, size, pstmt, pztail); }

    int sqlite3_reset(sqlite3_stmt* stmt) override
    { return ::sqlite3_reset(stmt); }

    void sqlite3_result_null(sqlite3_context* ctx) override
    { return ::sqlite3_result_null(ctx); }

    void sqlite3_result_text(sqlite3_context* ctx, char const* value, int size, void (* d)(void*)) override
    { return ::sqlite3_result_text(ctx, value, size, d); }

    void sqlite3_set_auxdata(sqlite3_context* ctx, int N, void* data, void (* d)(void*)) override
    { return ::sqlite3_set_auxdata(ctx, N, data, d); }

    int sqlite3_shutdown() override
    { return ::sqlite3_shutdown(); }

    int sqlite3_step(sqlite3_stmt* stmt) override
    { return ::sqlite3_step(stmt); }

    void const* sqlite3_value_blob(sqlite3_value* value) override
    { return ::sqlite3_value_blob(value); }

    int sqlite3_value_bytes(sqlite3_value* value) override
    { return ::sqlite3_value_bytes(value); }

    sqlite3_vfs* sqlite3_vfs_find(char const* zVfsName) override
    { return ::sqlite3_vfs_find(zVfsName); }
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
  public:
    SqliteDB(bool skipInitAndShutdown)
      : m_db(nullptr),
        m_statementsOffset(0),
        m_skipInitAndShutdown(skipInitAndShutdown)
    {
    }

    bool initialize(std::string const& filename, bool deletePrevious)
    {
        int result;

        if (!m_skipInitAndShutdown) {
            result = g_sqlite3Proxy->sqlite3_initialize();
            if (result != SQLITE_OK) {
                ARIASDK_LOG_ERROR("Failed to initialize SQLite (%d)", result);
                return false;
            }
        }

        if (deletePrevious) {
            // We cannot call plain ::remove() here, filename is in UTF-8. Rather
            // than adding a new set of functions to PAL, let's use SQLite VFS.
            sqlite3_vfs* vfs = g_sqlite3Proxy->sqlite3_vfs_find(NULL);
            result = (vfs != NULL) ? vfs->xDelete(vfs, filename.c_str(), 0) : SQLITE_ERROR;
            if (result == SQLITE_OK) {
                ARIASDK_LOG_INFO("Unusable existing database file was successfully deleted");
            } else if (result != SQLITE_IOERR_DELETE_NOENT) {
                ARIASDK_LOG_WARNING("Failed to delete unusable database file (%d)", result);
                if (!m_skipInitAndShutdown) {
                    g_sqlite3Proxy->sqlite3_shutdown();
                }
                return false;
            }
        }

        // Take basename only, potential PII like profile name in the path must not be logged
        size_t ofs = filename.find_last_of("/\\", std::string::npos);
        std::string basename(filename, (ofs != std::string::npos) ? (ofs + 1) : 0);
        ARIASDK_LOG_INFO("Opening database \"%s\"...", basename.c_str());

        result = g_sqlite3Proxy->sqlite3_open_v2(filename.c_str(), &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL);
        if (result != SQLITE_OK) {
            ARIASDK_LOG_ERROR("Failed to open database file: (%d) %s",
                result, m_db ? g_sqlite3Proxy->sqlite3_errmsg(m_db) : "-");
            if (m_db) {
                g_sqlite3Proxy->sqlite3_close(m_db);
                m_db = nullptr;
            }
            if (!m_skipInitAndShutdown) {
                g_sqlite3Proxy->sqlite3_shutdown();
            }
            return false;
        }

        g_sqlite3Proxy->sqlite3_extended_result_codes(m_db, 1);

        if (!registerTokenizeFunction()) {
            shutdown();
            return false;
        }

        ARIASDK_LOG_DETAIL("Database file was successfully opened");
        return true;
    }

    void shutdown()
    {
        if (m_db == nullptr) {
            return;
        }

        ARIASDK_LOG_DETAIL("Closing database");

        for (sqlite3_stmt* stmt : m_statements) {
            if (stmt != nullptr) {
                g_sqlite3Proxy->sqlite3_finalize(stmt);
            }
        }
        m_statementsOffset = std::min(1000000000, static_cast<int>(m_statementsOffset + m_statements.size() + 100 - m_statements.size() % 100));
        m_statements.clear();

        g_sqlite3Proxy->sqlite3_close(m_db);
        m_db = nullptr;

        if (!m_skipInitAndShutdown) {
            g_sqlite3Proxy->sqlite3_shutdown();
        }
    }

    int prepare(char const* statement)
    {
        sqlite3_stmt* stmt;
        int result = g_sqlite3Proxy->sqlite3_prepare_v2(m_db, statement, -1, &stmt, NULL);
        if (result != SQLITE_OK) {
            std::string excerpt(statement);
            if (excerpt.length() > 100) {
                excerpt.resize(100);
                excerpt.append("...");
            }
            ARIASDK_LOG_ERROR("Failed to prepare SQL statement \"%s\": %d (%s)",
                excerpt.c_str(), result, g_sqlite3Proxy->sqlite3_errmsg(m_db));
            return -1;
        }

        size_t i = 0;
        while (i < m_statements.size()) {
            if (m_statements[i] == nullptr) {
                break;
            }
            i++;
        }
        if (i < m_statements.size()) {
            m_statements[i] = stmt;
        } else {
            m_statements.push_back(stmt);
        }
        return static_cast<int>(m_statementsOffset + i);
    }

    sqlite3_stmt* statement(int stmtId)
    {
        assert(stmtId >= m_statementsOffset && static_cast<size_t>(stmtId) < m_statementsOffset + m_statements.size());
        assert(m_statements[stmtId - m_statementsOffset] != nullptr);
        return m_statements[stmtId - m_statementsOffset];
    }

    void release(int stmtId)
    {
        assert(stmtId >= m_statementsOffset && static_cast<size_t>(stmtId) < m_statementsOffset + m_statements.size());
        assert(m_statements[stmtId - m_statementsOffset] != nullptr);
        g_sqlite3Proxy->sqlite3_finalize(m_statements[stmtId - m_statementsOffset]);
        m_statements[stmtId - m_statementsOffset] = nullptr;
    }

    operator sqlite3*()
    {
        return m_db;
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
        char const* sep  = static_cast<char const*>(memchr(data + ofs, 0, len - ofs));
        int pos = sep ? static_cast<int>(sep - data) : len;
        g_sqlite3Proxy->sqlite3_result_text(ctx, data + ofs, pos - ofs, SQLITE_STATIC);
        g_sqlite3Proxy->sqlite3_set_auxdata(ctx, 0, reinterpret_cast<void*>(static_cast<intptr_t>(pos + 1)), NULL);
    }

    bool registerTokenizeFunction()
    {
        int result = g_sqlite3Proxy->sqlite3_create_function_v2(m_db, "tokenize", 1, SQLITE_UTF8, NULL,
            &SqliteDB::sqliteFunc_tokenize, NULL, NULL, NULL);
        if (result != SQLITE_OK) {
            ARIASDK_LOG_ERROR("Could not create tokenize function: (%d) %s",
                result, g_sqlite3Proxy->sqlite3_errmsg(m_db));
            return false;
        }
        return true;
    }

  protected:
    sqlite3*                   m_db;
    std::vector<sqlite3_stmt*> m_statements;
    int                        m_statementsOffset;
    bool                       m_skipInitAndShutdown;

  private:
    ARIASDK_LOG_DECL_COMPONENT_CLASS();
};

ARIASDK_LOG_INST_COMPONENT_CLASS(SqliteDB, "AriaSDK.SQLiteDB", "Aria telemetry client - SqliteDB class");

//---

class SqliteStatement {
  public:
    SqliteStatement(SqliteDB& db, int stmtId)
      : m_db(db),
        m_stmtId(stmtId),
        m_stmt((stmtId >= 0) ? db.statement(stmtId) : nullptr),
        m_changes(0),
        m_duration(0),
        m_ownStmt(false),
        m_hasRow(false),
        m_error(false)
    {
        reset();
    }

    SqliteStatement(SqliteDB& db, char const* statement)
      : m_db(db),
        m_stmtId(db.prepare(statement)),
        m_stmt((m_stmtId >= 0) ? db.statement(m_stmtId) : nullptr),
        m_changes(0),
        m_duration(0),
        m_ownStmt(true),
        m_hasRow(false),
        m_error(false)
    {
        reset();
    }

    ~SqliteStatement()
    {
        if (m_ownStmt && m_stmtId >= 0) {
            m_db.release(m_stmtId);
        }
    }

    template<typename... TArgs>
    bool execute(TArgs&& ... args)
    {
        if (m_stmt != nullptr) {
            int result = bindAll(0, std::forward<TArgs>(args) ...);
            return execute2(result);
        } else {
            return false;
        }
    }

    template<typename... TArgs>
    bool select(TArgs&& ... args)
    {
        if (m_stmt != nullptr) {
            int result = bindAll(0, std::forward<TArgs>(args) ...);
            return select2(result);
        } else {
            return false;
        }
    }

    template<typename... TResults>
    bool getRow(TResults& ... results)
    {
        if (m_stmt != nullptr) {
            return getRow2() && retrieveAll(0, results...);
        } else {
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
    { return g_sqlite3Proxy->sqlite3_bind_int(m_stmt, idx, arg); }

    int bind(int idx, unsigned arg)
    { return g_sqlite3Proxy->sqlite3_bind_int64(m_stmt, idx, static_cast<int64_t>(static_cast<uint64_t>(arg))); }

    int bind(int idx, int64_t arg)
    { return g_sqlite3Proxy->sqlite3_bind_int64(m_stmt, idx, arg); }

    int bind(int idx, std::string const& arg)
    { return g_sqlite3Proxy->sqlite3_bind_text(m_stmt, idx, arg.data(), static_cast<int>(arg.size()), SQLITE_STATIC); }

    int bind(int idx, std::vector<uint8_t> const& arg)
    { return g_sqlite3Proxy->sqlite3_bind_blob(m_stmt, idx, arg.data(), static_cast<int>(arg.size()), SQLITE_STATIC); }

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
    { output = g_sqlite3Proxy->sqlite3_column_int(m_stmt, idx); }

    void retrieve(int idx, unsigned& output)
    { output = static_cast<unsigned>(g_sqlite3Proxy->sqlite3_column_int64(m_stmt, idx)); }

    void retrieve(int idx, int64_t& output)
    { output = g_sqlite3Proxy->sqlite3_column_int64(m_stmt, idx); }

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
            ARIASDK_LOG_ERROR("Failed to bind parameter #%d of statement #%d: %s",
                bindFailedIdx, m_stmtId, g_sqlite3Proxy->sqlite3_errmsg(m_db));
            m_error = true;
            return false;
        }

        int64_t startTime = PAL::getMonotonicTimeMs();
        int result = g_sqlite3Proxy->sqlite3_step(m_stmt);
        m_duration = static_cast<unsigned>(PAL::getMonotonicTimeMs() - startTime);

        if (result == SQLITE_ROW) {
            assert(!"executed statement returned a row, use select()");
        } else if (result != SQLITE_DONE) {
            ARIASDK_LOG_ERROR("Failed to modify database while executing statement #%d: %d (%s)",
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
            ARIASDK_LOG_ERROR("Failed to bind parameter #%d of statement #%d: %s",
                bindFailedIdx, m_stmtId, g_sqlite3Proxy->sqlite3_errmsg(m_db));
            m_error = true;
            return false;
        }

        int result = g_sqlite3Proxy->sqlite3_step(m_stmt);
        if (result == SQLITE_ROW) {
            m_hasRow = true;
            m_done = false;
            return true;
        } else if (result == SQLITE_DONE) {
            m_hasRow = false;
            m_done = true;
            return true;
        } else {
            ARIASDK_LOG_ERROR("Failed to query database while executing statement #%d: %d (%s)",
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
            ARIASDK_LOG_ERROR("Failed to read database while executing statement #%d: %d (%s)",
                m_stmtId, result, g_sqlite3Proxy->sqlite3_errmsg(m_db));
            m_error = true;
        }
        reset();
        return false;
    }

  protected:
    SqliteDB&     m_db;
    int           m_stmtId;
    sqlite3_stmt* m_stmt;
    unsigned      m_changes;
    unsigned      m_duration;
    bool          m_ownStmt;
    bool          m_hasRow;
    bool          m_done;
    bool          m_error;

  private:
    ARIASDK_LOG_DECL_COMPONENT_CLASS();
};

ARIASDK_LOG_INST_COMPONENT_CLASS(SqliteStatement, "AriaSDK.SQLiteStatement", "Aria telemetry client - Sqlite statement class");


} ARIASDK_NS_END
