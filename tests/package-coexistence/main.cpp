#include "LogManager.hpp"
#include "sqlite3.h"
#include <cstdio>

#if defined(sqlite3_open) || defined(Z_PREFIX) || defined(MATSDK_DISABLE_EXCEPTIONS)
#error SDK-private compile policy leaked to a consumer
#endif

LOGMANAGER_INSTANCE

#ifdef MATSDK_TEST_PRIVATE_SQLITE
extern "C"
{
    int matsdk_sqlite3_open(const char*, sqlite3**);
    int matsdk_sqlite3_close(sqlite3*);
    int matsdk_sqlite3_exec(sqlite3*, const char*, int (*)(void*, int, char**, char**), void*, char**);
    int matsdk_sqlite3_compileoption_used(const char*);
}
#endif

int main()
{
    sqlite3* external = nullptr;
    if (sqlite3_open(":memory:", &external) != SQLITE_OK)
    {
        std::fprintf(stderr, "External SQLite failed to open\n");
        return 1;
    }
    if (sqlite3_compileoption_used("OMIT_JSON") ||
        sqlite3_exec(external, "CREATE TABLE external_data(value); INSERT INTO external_data VALUES(42);",
                     nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        std::fprintf(stderr, "External SQLite was replaced by the SDK's stripped copy\n");
        sqlite3_close(external);
        return 1;
    }
#ifdef MATSDK_TEST_PRIVATE_SQLITE
    sqlite3* internal = nullptr;
    if (matsdk_sqlite3_open(":memory:", &internal) != SQLITE_OK ||
        matsdk_sqlite3_exec(internal, "CREATE TABLE sdk_data(value); INSERT INTO sdk_data VALUES(7);",
                            nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        std::fprintf(stderr, "SDK-private SQLite failed\n");
        if (internal)
        {
            matsdk_sqlite3_close(internal);
        }
        sqlite3_close(external);
        return 1;
    }
#ifdef MATSDK_TEST_MINIMAL_SQLITE
    if (!matsdk_sqlite3_compileoption_used("OMIT_JSON"))
    {
        std::fprintf(stderr, "SDK-private SQLite was replaced by the external copy\n");
        matsdk_sqlite3_close(internal);
        sqlite3_close(external);
        return 1;
    }
#endif
    if (matsdk_sqlite3_close(internal) != SQLITE_OK)
    {
        std::fprintf(stderr, "SDK-private SQLite failed to close\n");
        sqlite3_close(external);
        return 1;
    }
#endif
    MAT::ILogger* logger = MAT::LogManager::Initialize("sqlite-coexistence-test");
    if (!logger)
    {
        std::fprintf(stderr, "SDK initialization failed with external SQLite linked\n");
        sqlite3_close(external);
        return 1;
    }
    MAT::LogManager::FlushAndTeardown();
    if (sqlite3_close(external) != SQLITE_OK)
    {
        std::fprintf(stderr, "External SQLite failed to close\n");
        return 1;
    }
    return 0;
}
