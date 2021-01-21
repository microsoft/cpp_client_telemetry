//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "sqlite3.h"
#include "offline/ISqlite3Proxy.hpp"

#include "pal/PAL.hpp"

namespace testing {

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"  // GMock MOCK_METHOD* macros don't use override.
#endif

class MockISqlite3Proxy : public MAT::ISqlite3Proxy {
  public:
    MockISqlite3Proxy();
    virtual ~MockISqlite3Proxy();

    MOCK_METHOD5(sqlite3_bind_blob, int(sqlite3_stmt * stmt, int idx, void const* value, int size, void (* d)(void*)));
    MOCK_METHOD3(sqlite3_bind_int, int(sqlite3_stmt * stmt, int idx, int value));
    MOCK_METHOD3(sqlite3_bind_int64, int(sqlite3_stmt * stmt, int idx, int64_t value));
    MOCK_METHOD5(sqlite3_bind_text, int(sqlite3_stmt * stmt, int idx, char const* value, int size, void (* d)(void*)));
    MOCK_METHOD1(sqlite3_changes, int(sqlite3 * db));
    MOCK_METHOD1(sqlite3_clear_bindings, int(sqlite3_stmt * stmt));
    MOCK_METHOD1(sqlite3_close, int(sqlite3 * db));
    MOCK_METHOD1(sqlite3_close_v2, int(sqlite3 * db));
    MOCK_METHOD2(sqlite3_column_blob, void const*(sqlite3_stmt * stmt, int iCol));
    MOCK_METHOD2(sqlite3_column_bytes, int(sqlite3_stmt * stmt, int iCol));
    MOCK_METHOD2(sqlite3_column_int, int(sqlite3_stmt * stmt, int iCol));
    MOCK_METHOD2(sqlite3_column_int64, int64_t(sqlite3_stmt * stmt, int iCol));
    MOCK_METHOD2(sqlite3_column_text, unsigned const char*(sqlite3_stmt * stmt, int iCol));
    MOCK_METHOD9(sqlite3_create_function_v2, int(sqlite3 * db, char const* zFunctionName, int nArg, int eTextRep, void* pApp,
        void (* xFunc)(sqlite3_context*, int, sqlite3_value**), void (* xStep)(sqlite3_context*, int, sqlite3_value**),
        void (* xFinal)(sqlite3_context*), void (* xDestroy)(void*)));
    MOCK_METHOD1(sqlite3_errmsg, char const*(sqlite3 * db));
    MOCK_METHOD2(sqlite3_extended_result_codes, int(sqlite3 * db, int on));
    MOCK_METHOD1(sqlite3_finalize, int(sqlite3_stmt * stmt));
    MOCK_METHOD2(sqlite3_get_auxdata, void*(sqlite3_context * ctx, int N));
    MOCK_METHOD0(sqlite3_initialize, int());
    MOCK_METHOD4(sqlite3_open_v2, int(char const* file, sqlite3 * *pdb, int flags, char const* zvfs));
    MOCK_METHOD5(sqlite3_prepare_v2, int(sqlite3 * db, char const* zsql, int size, sqlite3_stmt * *pstmt, char const** pztail));
    MOCK_METHOD1(sqlite3_reset, int(sqlite3_stmt * stmt));
    MOCK_METHOD1(sqlite3_result_null, void(sqlite3_context * ctx));
    MOCK_METHOD4(sqlite3_result_text, void(sqlite3_context * ctx, char const* value, int size, void (* d)(void*)));
    MOCK_METHOD4(sqlite3_set_auxdata, void(sqlite3_context * ctx, int N, void* data, void (* d)(void*)));
    MOCK_METHOD0(sqlite3_shutdown, int());
    MOCK_METHOD1(sqlite3_step, int(sqlite3_stmt * stmt));
    MOCK_METHOD1(sqlite3_value_blob, void const*(sqlite3_value * value));
    MOCK_METHOD1(sqlite3_soft_heap_limit64, int64_t(int64_t N));
    MOCK_METHOD1(sqlite3_value_bytes, int(sqlite3_value * value));
    MOCK_METHOD1(sqlite3_vfs_find, sqlite3_vfs * (char const* zVfsName));
};

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

}  // namespace testing

