//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef ISQLITE3PROXY_HPP
#define ISQLITE3PROXY_HPP

#include "pal/PAL.hpp"

#include <cinttypes>

struct Mem;
struct sqlite3;
struct sqlite3_context;
struct sqlite3_stmt;
struct sqlite3_vfs;


namespace MAT_NS_BEGIN {


class ISqlite3Proxy {
  public:
    virtual ~ISqlite3Proxy() {}
    virtual int                  sqlite3_bind_blob(sqlite3_stmt* stmt, int idx, void const* value, int size, void (* d)(void*)) = 0;
    virtual int                  sqlite3_bind_int(sqlite3_stmt* stmt, int idx, int value) = 0;
    virtual int                  sqlite3_bind_int64(sqlite3_stmt* stmt, int idx, int64_t value) = 0;
    virtual int                  sqlite3_bind_text(sqlite3_stmt* stmt, int idx, char const* value, int size, void (* d)(void*)) = 0;
    virtual int                  sqlite3_changes(sqlite3* db) = 0;
    virtual int                  sqlite3_clear_bindings(sqlite3_stmt* stmt) = 0;
    virtual int                  sqlite3_close(sqlite3* db) = 0;
    virtual int                  sqlite3_close_v2(sqlite3* db) = 0;
    virtual const void*          sqlite3_column_blob(sqlite3_stmt* stmt, int iCol) = 0;
    virtual int                  sqlite3_column_bytes(sqlite3_stmt* stmt, int iCol) = 0;
    virtual int                  sqlite3_column_int(sqlite3_stmt* stmt, int iCol) = 0;
    virtual int64_t              sqlite3_column_int64(sqlite3_stmt* stmt, int iCol) = 0;
    virtual unsigned char const* sqlite3_column_text(sqlite3_stmt* stmt, int iCol) = 0;
    virtual int                  sqlite3_create_function_v2(sqlite3* db, char const* zFunctionName, int nArg, int eTextRep, void* pApp,
        void (* xFunc)(sqlite3_context*, int, sqlite3_value**), void (* xStep)(sqlite3_context*, int, sqlite3_value**),
        void (* xFinal)(sqlite3_context*), void (* xDestroy)(void*)) = 0;
    virtual const char*          sqlite3_errmsg(sqlite3* db) = 0;
    virtual int                  sqlite3_extended_result_codes(sqlite3* db, int on) = 0;
    virtual int                  sqlite3_finalize(sqlite3_stmt* stmt) = 0;
    virtual void*                sqlite3_get_auxdata(sqlite3_context* ctx, int N) = 0;
    virtual int                  sqlite3_initialize() = 0;
    virtual int                  sqlite3_open_v2(char const* file, sqlite3** pdb, int flags, char const* zvfs) = 0;
    virtual int                  sqlite3_prepare_v2(sqlite3* db, char const* zsql, int size, sqlite3_stmt** pstmt, char const** pztail) = 0;
    virtual int                  sqlite3_reset(sqlite3_stmt* stmt) = 0;
    virtual void                 sqlite3_result_null(sqlite3_context* ctx) = 0;
    virtual void                 sqlite3_result_text(sqlite3_context* ctx, char const* value, int size, void (* d)(void*)) = 0;
    virtual void                 sqlite3_set_auxdata(sqlite3_context* ctx, int N, void* data, void (* d)(void*)) = 0;
    virtual int                  sqlite3_shutdown() = 0;
    virtual int                  sqlite3_step(sqlite3_stmt* stmt) = 0;
    virtual int64_t              sqlite3_soft_heap_limit64(int64_t N) = 0;
    virtual const void*          sqlite3_value_blob(sqlite3_value* value) = 0;
    virtual int                  sqlite3_value_bytes(sqlite3_value* value) = 0;
    virtual sqlite3_vfs*         sqlite3_vfs_find(char const* zVfsName) = 0;
};

extern ISqlite3Proxy* g_sqlite3Proxy;


} MAT_NS_END
#endif

