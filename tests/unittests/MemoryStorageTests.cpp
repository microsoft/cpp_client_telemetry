//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "common/Common.hpp"
#include "utils/Utils.hpp"

#include "offline/MemoryStorage.hpp"
#include "config/RuntimeConfig_Default.hpp"
#include "NullObjects.hpp"

#include "common/MockIOfflineStorageObserver.hpp"

#include <stdio.h>
#include <regex>

#include "pal/PAL.hpp"

#include <set>
#include <memory>
#include <thread>
#include <atomic>

using namespace testing;
using namespace MAT;

/// <summary>
/// Test observer. Currently the callbacks mechanism is not implemented at MemoryStorage abstraction level.
/// </summary>
/// <seealso cref="IOfflineStorageObserver" />
class TestObserver : public IOfflineStorageObserver
{
    virtual void OnStorageOpened(std::string const & type) override
    {
        UNREFERENCED_PARAMETER(type);
    }
    virtual void OnStorageFailed(std::string const & reason) override
    {
        UNREFERENCED_PARAMETER(reason);
    }
    virtual void OnStorageOpenFailed(std::string const & reason) override
    {
        UNREFERENCED_PARAMETER(reason);
    }
    virtual void OnStorageTrimmed(std::map<std::string, size_t> const & numRecords) override
    {
        UNREFERENCED_PARAMETER(numRecords);
    }
    virtual void OnStorageRecordsDropped(std::map<std::string, size_t> const & numRecords) override
    {
        UNREFERENCED_PARAMETER(numRecords);
    }
    virtual void OnStorageRecordsRejected(std::map<std::string, size_t> const & numRecords) override
    {
        UNREFERENCED_PARAMETER(numRecords);
    }
    virtual void OnStorageRecordsSaved(size_t numRecords) override
    {
        UNREFERENCED_PARAMETER(numRecords);
    }
};

/// <summary>
/// No-op NULL object pattern log manager
/// </summary>
NullLogManager              testLogManager;

/// <summary>
/// Default test configuration
/// </summary>
RuntimeConfig_Default       testConfig(testLogManager.GetLogConfiguration());

/// <summary>
/// Storage observer
/// </summary>
TestObserver                testObserver;

// Run the list of all supported latencies
std::set<EventLatency> latencies =
{
    EventLatency_Normal,
    EventLatency_CostDeferred,
    EventLatency_RealTime,
    EventLatency_Max
};

constexpr size_t num_iterations = 10000;

/// <summary>
/// Adds events of various latencies to storage
/// </summary>
/// <param name="storage">The storage.</param>
/// <returns>Database size</returns>
size_t addEvents(MemoryStorage& storage)
{
    // Add events
    size_t total_db_size = 0;
    for (size_t i = 0; i < num_iterations; i++)
    {
        for (const EventLatency &lat : latencies)
        {
            StorageRecord record{ PAL::generateUuidString(), "token", lat, EventPersistence_Critical, INT64_MIN + 1, { 5, 4, 3, 2, 1 }, 77, INT64_MAX - 1 };
            total_db_size += record.blob.size() + sizeof(record);
            storage.StoreRecord(record);
        }
    }

    return total_db_size;
}

/// <summary>
/// Initialize, generate some records, save and retrieve these records back.
/// </summary>
/// <param name="">The .</param>
/// <param name="">The .</param>
/// <returns></returns>
TEST(MemoryStorageTests, StoreAndGetRecords)
{
    MemoryStorage storage(testLogManager, testConfig);

    // Observer callbacks are not currently implemented for this storage type
    storage.Initialize(testObserver);

    // Initial DB is empty
    EXPECT_THAT(storage.GetSize(), 0);
    
    // Check that EventLatency_Off doesn't get saved to ram queue
    StorageRecord record{ PAL::generateUuidString(), "token", EventLatency_Off, EventPersistence_Critical, INT64_MIN + 1, { 5, 4, 3, 2, 1 }, 77, INT64_MAX - 1 };
    EXPECT_THAT(storage.StoreRecord(record), false);
    EXPECT_THAT(storage.GetSize(), 0);

    // Read records in batches by latency
    {
        size_t total_db_size = addEvents(storage);
        // Verify that the DB is the right size
        EXPECT_THAT(storage.GetSize(), total_db_size);
        // Verify the contents
        size_t total_records = num_iterations * latencies.size();
        for (auto latency : { EventLatency_Max , EventLatency_RealTime , EventLatency_CostDeferred , EventLatency_Normal })
        {
            auto records = storage.GetRecords(false, latency, 0);
            EXPECT_THAT(records.size(), num_iterations);
            EXPECT_THAT(records.size(), storage.LastReadRecordCount());
            total_records -= records.size();
        }
        EXPECT_THAT(storage.GetSize(), 0);
        EXPECT_THAT(total_records, 0);
    }

    // Read all records at once in one batch
    {
        size_t total_db_size = addEvents(storage);
        // Verify that the DB is the right size
        EXPECT_THAT(storage.GetSize(), total_db_size);
        // Verify the contents
        size_t total_records = num_iterations * latencies.size();
        auto records = storage.GetRecords(false, EventLatency_Normal, 0);
        EXPECT_THAT(records.size(), total_records);
        EXPECT_THAT(records.size(), storage.LastReadRecordCount());
        EXPECT_THAT(storage.GetSize(), 0);
    }

    // Verify that the ram queue is empty after read
    EXPECT_THAT(storage.GetSize(), 0);
}

TEST(MemoryStorageTests, GetRecordsDeletesRecords)
{
    MemoryStorage storage(testLogManager, testConfig);

    std::vector<StorageRecordId> ids;

    // Add some events to storage
    auto total_db_size = addEvents(storage);
    EXPECT_THAT(storage.GetSize(), total_db_size);

    // Retrieve those into records
    auto records = storage.GetRecords();

    // Storage size is "zero" because all records are fetched
    EXPECT_THAT(storage.GetSize(), 0);
    EXPECT_THAT(storage.GetRecordCount(), 0);
    EXPECT_THAT(storage.GetReservedCount(), 0);
    EXPECT_THAT(records.size(), num_iterations * 4); // 4 latencies
}

TEST(MemoryStorageTests, DeleteAllRecords)
{
    MemoryStorage storage(testLogManager, testConfig);

    std::vector<StorageRecordId> ids;

    // Add some events to storage
    auto total_db_size = addEvents(storage);
    EXPECT_THAT(storage.GetSize(), total_db_size);

    // Retrieve those into records
    storage.DeleteAllRecords();

    // Storage size is "zero" because all records are fetched
    EXPECT_THAT(storage.GetSize(), 0);
    EXPECT_THAT(storage.GetRecordCount(), 0);
    EXPECT_THAT(storage.GetReservedCount(), 0);
}


TEST(MemoryStorageTests, ReleaseRecords)
{
    MemoryStorage storage(testLogManager, testConfig);

    std::vector<StorageRecordId> ids;
    HttpHeaders headers;
    bool fromMemory = true;

    // Add some events to storage
    auto total_db_size = addEvents(storage);
    EXPECT_THAT(storage.GetSize(), total_db_size);
    auto total_records = storage.GetRecordCount();

    // Retrieve those into records
    std::vector<StorageRecord> records;

    auto consumer = [&records](StorageRecord&& record) -> bool {
        records.push_back(std::move(record));
        return true; // want more
    };

    storage.GetAndReserveRecords(consumer, 1500);

    // Storage size is "zero" because all records are currently reserved (in-flight)
    EXPECT_THAT(storage.GetSize(), 0);
    EXPECT_THAT(storage.GetRecordCount(), 0);
    EXPECT_THAT(storage.GetReservedCount(), total_records);
    EXPECT_THAT(records.size(), total_records);

    // Track IDs of all reserved records
    for (auto &record : records)
    {
        ids.push_back(record.id);
    }

    // Restore all reserved records
    storage.ReleaseRecords(ids, false, headers, fromMemory);
    EXPECT_THAT(storage.GetSize(), total_db_size);
    EXPECT_THAT(storage.GetRecordCount(), total_records);
    EXPECT_THAT(storage.GetReservedCount(), 0);
}

TEST(MemoryStorageTests, GetAndReserveSome)
{
    MemoryStorage storage(testLogManager, testConfig);
    storage.Initialize(testObserver);
    addEvents(storage);
    auto totalCount = storage.GetRecordCount();
    constexpr size_t howMany = 32;
    std::vector<StorageRecord> someRecords;
    storage.GetAndReserveRecords(
        [&someRecords, howMany] (StorageRecord && record)->bool
        {
            if (someRecords.size() >= howMany) {
                return false;
            }
            someRecords.emplace_back(std::move(record));
            return true;
        },
        EventLatency_Normal
    );
    EXPECT_EQ(howMany, someRecords.size());
    EXPECT_EQ(howMany, storage.LastReadRecordCount());
    EXPECT_EQ(totalCount - howMany, storage.GetRecordCount());
}

// This method is not implemented for RAM storage
TEST(MemoryStorageTests, StoreSetting)
{
    MemoryStorage storage(testLogManager, testConfig);
    bool result = storage.StoreSetting("not_implemented", "not_implemented");
    EXPECT_THAT(result, false);
}

// This method is not implemented for RAM storage
TEST(MemoryStorageTests, GetSetting)
{
    MemoryStorage storage(testLogManager, testConfig);
    auto result = storage.GetSetting("not_implemented");
    EXPECT_THAT(result.empty(), true);
}

// This method is not implemented for RAM storage
TEST(MemoryStorageTests, ResizeDb)
{
    MemoryStorage storage(testLogManager, testConfig);
    EXPECT_THAT(storage.ResizeDb(), true);
}

constexpr size_t MAX_STRESS_THREADS = 20;

TEST(MemoryStorageTests, MultiThreadPerfTest)
{
    MemoryStorage storage(testLogManager, testConfig);

    std::vector<std::thread> workers;
    std::atomic<size_t> threadCount(0);

    // Add / Remove some events from several threads
    for (size_t i = 0; i < MAX_STRESS_THREADS; i++) {
        workers.push_back(std::thread([&storage, &threadCount]()
        {
            threadCount++;
            // Add some events
            addEvents(storage);

            // Retrieve them
            auto records = storage.GetRecords();

            // Acquire IDs
            std::vector<StorageRecordId> ids;
            for (const auto &record : records)
                ids.push_back(record.id);

            // Release records
            HttpHeaders headers;
            bool inMemory = true;
            storage.DeleteRecords(ids, headers, inMemory);
        }));
    }

    // The issue here is that clang-libc++ Mac OS X runtime can schedule
    // the main thread to continue running BEFORE propagating the spawned
    // thread(s) into joinable state, i.e. the thread is created, but not
    // assigned a native handle yet. What that in essence means that the
    // logic that waits for completion of all workers may run into race
    // condition with one of threads not being in joinable state YET...
    // Means rather than waiting for completion - we actually skip a thread
    // that is about to get scheduled!! Solution is to use the threadCount
    // to yield until each thread is guaranteed spawned, assigned a native
    // handle, after that we  can iterate and wait for completion.
    while (threadCount.load() != MAX_STRESS_THREADS)
    {
        std::this_thread::yield();
    }

    // Wait for completion of all worker threads
    std::for_each(workers.begin(), workers.end(), [](std::thread &t)
    {
        if (t.joinable())
            t.join();
    });

    // Once we're done - the storage has to be empty
    EXPECT_THAT(storage.GetRecordCount(), 0);
    EXPECT_THAT(storage.GetReservedCount(), 0);
    EXPECT_THAT(storage.GetSize(), 0);

}

