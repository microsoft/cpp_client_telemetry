// Copyright (c) Microsoft Corporation. All rights reserved.

#include "common/Common.hpp"
#include "common/MockIOfflineStorage.hpp"
#include "common/MockIOfflineStorageObserver.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include "offline/OfflineStorageHandler.hpp"
#include "offline/IOfflineStorageProvider.hpp"
#include "offline/StorageObserver.hpp"
#include "offline/MemoryStorage.hpp"
#include "pal/TaskDispatcher_CAPI.hpp"
#include "NullObjects.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <vector>

using namespace testing;
using namespace MAT;

class OfflineStorageTests : public StrictMock<Test> {
protected:
    MockIOfflineStorage     offlineStorageMock;
    StorageObserver         offlineStorage;

    RouteSink<OfflineStorageTests, IncomingEventContextPtr const&>                             storeRecordFailed{ this, &OfflineStorageTests::resultStoreRecordFailed };
    RouteSink<OfflineStorageTests, EventsUploadContextPtr const&, StorageRecord const&, bool&> retrievedEvent{ this, &OfflineStorageTests::resultRetrievedEvent };
    RouteSink<OfflineStorageTests, EventsUploadContextPtr const&>                              retrievalFinished{ this, &OfflineStorageTests::resultRetrievalFinished };
    RouteSink<OfflineStorageTests, EventsUploadContextPtr const&>                              retrievalFailed{ this, &OfflineStorageTests::resultRetrievalFailed };

    RouteSink<OfflineStorageTests, StorageNotificationContext const*>                          opened{ this, &OfflineStorageTests::notifOpened };
    RouteSink<OfflineStorageTests, StorageNotificationContext const*>                          failed{ this, &OfflineStorageTests::notifFailed };
    RouteSink<OfflineStorageTests, StorageNotificationContext const*>                          trimmed{ this, &OfflineStorageTests::notifTrimmed };
    RouteSink<OfflineStorageTests, StorageNotificationContext const*>                          recordsDropped{ this, &OfflineStorageTests::notifRecordsDropped };

protected:
    OfflineStorageTests()
        : offlineStorage(testing::getSystem(), offlineStorageMock)
    {
        offlineStorage.storeRecordFailed >> storeRecordFailed;
        offlineStorage.retrievedEvent >> retrievedEvent;
        offlineStorage.retrievalFinished >> retrievalFinished;
        offlineStorage.retrievalFailed >> retrievalFailed;

        offlineStorage.opened >> opened;
        offlineStorage.failed >> failed;
        offlineStorage.trimmed >> trimmed;
        offlineStorage.recordsDropped >> recordsDropped;
    }

    MOCK_METHOD1(resultStoreRecordFailed, void(IncomingEventContextPtr const &));
    MOCK_METHOD3(resultRetrievedEvent, void(EventsUploadContextPtr const &, StorageRecord const &, bool&));
    MOCK_METHOD1(resultRetrievalFinished, void(EventsUploadContextPtr const &));
    MOCK_METHOD1(resultRetrievalFailed, void(EventsUploadContextPtr const &));

    MOCK_METHOD1(notifOpened, void(StorageNotificationContext const*));
    MOCK_METHOD1(notifFailed, void(StorageNotificationContext const*));
    MOCK_METHOD1(notifTrimmed, void(StorageNotificationContext const*));
    MOCK_METHOD1(notifRecordsDropped, void(StorageNotificationContext const*));
};


TEST_F(OfflineStorageTests, StartInitializes)
{
    EXPECT_CALL(offlineStorageMock, Initialize(Ref(offlineStorage)))
        .WillOnce(Return());
    EXPECT_THAT(offlineStorage.start(), true);
}

TEST_F(OfflineStorageTests, StopShutsDown)
{
    EXPECT_CALL(offlineStorageMock, Shutdown())
        .WillOnce(Return());
    EXPECT_THAT(offlineStorage.stop(), true);
}

TEST_F(OfflineStorageTests, StoreRecordIsForwarded)
{
    IncomingEventContext ctx;

    EXPECT_CALL(offlineStorageMock, StoreRecord(Ref(ctx.record)))
        .WillOnce(Return(true));
    EXPECT_THAT(offlineStorage.storeRecord(&ctx), true);
    EXPECT_THAT(ctx.record.timestamp, Near(PAL::getUtcSystemTimeMs(), 1000));

    EXPECT_CALL(offlineStorageMock, StoreRecord(Ref(ctx.record)))
        .WillOnce(Return(false));
    EXPECT_CALL(*this, resultStoreRecordFailed(&ctx))
        .WillOnce(Return());
    EXPECT_THAT(offlineStorage.storeRecord(&ctx), false);
}

TEST_F(OfflineStorageTests, RetrieveEventsPassesRecordsThrough)
{
    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->requestedMinLatency = EventLatency_Normal;
    ctx->requestedMaxCount = 6;

    StorageRecord record1("r1", "tenant1-token", EventLatency_Normal, EventPersistence_Normal, 1234567890, std::vector<uint8_t>{1, 127, 255});
    StorageRecord record2("r2", "tenant2-token", EventLatency_Normal, EventPersistence_Normal, 1234567891, std::vector<uint8_t>{2, 128, 0});
    EXPECT_CALL(offlineStorageMock, GetAndReserveRecords(_, Gt(1000u), ctx->requestedMinLatency, ctx->requestedMaxCount))
        .WillOnce(DoAll(
            Invoke([&record1, &record2](std::function<bool(StorageRecord&&)> const& consumer, unsigned, EventLatency, unsigned) {
        EXPECT_THAT(consumer(std::move(record1)), true);
        EXPECT_THAT(consumer(std::move(record2)), false);
    }),
            Return(true)))
        .RetiresOnSaturation();

    EXPECT_CALL(offlineStorageMock, IsLastReadFromMemory())
        .WillOnce(Return(false));

    EXPECT_CALL(*this, resultRetrievedEvent(ctx, Ref(record1), _))
        .WillOnce(SetArgReferee<2>(true))
        .RetiresOnSaturation();
    EXPECT_CALL(*this, resultRetrievedEvent(ctx, Ref(record2), _))
        .WillOnce(SetArgReferee<2>(false))
        .RetiresOnSaturation();
    EXPECT_CALL(*this, resultRetrievalFinished(ctx))
        .WillOnce(Return());



    offlineStorage.retrieveEvents(ctx);
}

TEST_F(OfflineStorageTests, RetrieveEventsFailureAborts)
{
    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->requestedMinLatency = EventLatency_Normal;
    ctx->requestedMaxCount = 6;

    EXPECT_CALL(offlineStorageMock, GetAndReserveRecords(_, Gt(1000u), ctx->requestedMinLatency, ctx->requestedMaxCount))
        .WillOnce(Return(false));
    EXPECT_CALL(offlineStorageMock, IsLastReadFromMemory())
        .WillOnce(Return(false));
    EXPECT_CALL(*this, resultRetrievalFailed(ctx))
        .WillOnce(Return());
    offlineStorage.retrieveEvents(ctx);
}

TEST_F(OfflineStorageTests, DeleteRecordsIsForwarded)
{
    auto ctx = std::make_shared<EventsUploadContext>();
    HttpHeaders test;
    bool fromMemory = false;
    std::vector<std::string> recordIds;
    for (const auto& element : ctx->recordIdsAndTenantIds)
    {
        recordIds.push_back(element.first);
    }
    ctx->fromMemory = fromMemory;
    EXPECT_CALL(offlineStorageMock, DeleteRecords(recordIds, test, fromMemory)).WillOnce(Return());
    EXPECT_THAT(offlineStorage.deleteRecords(ctx), true);
}

TEST_F(OfflineStorageTests, ReleaseRecordsIsForwarded)
{
    auto ctx = std::make_shared<EventsUploadContext>();
    HttpHeaders test;
    bool fromMemory = false;
    std::vector<std::string> recordIds;
    for (const auto& element : ctx->recordIdsAndTenantIds)
    {
        recordIds.push_back(element.first);
    }
    ctx->fromMemory = fromMemory;
    EXPECT_CALL(offlineStorageMock, ReleaseRecords(recordIds, false, test, fromMemory))
        .WillOnce(Return());
    EXPECT_THAT(offlineStorage.releaseRecords(ctx), true);
    fromMemory = false;
    EXPECT_CALL(offlineStorageMock, ReleaseRecords(recordIds, true, test, fromMemory))
        .WillOnce(Return());
    EXPECT_THAT(offlineStorage.releaseRecordsIncRetryCount(ctx), true);
}

namespace
{
    class ConfigurableLogManager : public NullLogManager
    {
    public:
        ILogConfiguration config;
        ILogConfiguration& GetLogConfiguration() override { return config; }
    };

    class CountingLogManager : public ConfigurableLogManager
    {
    public:
        bool StartActivity() override
        {
            ++activityCount;
            ++activeActivities;
            return true;
        }

        void EndActivity() override { --activeActivities; }

        std::atomic<unsigned> activityCount{0};
        std::atomic<int> activeActivities{0};
    };

    class PausedLogManager : public ConfigurableLogManager
    {
    public:
        bool StartActivity() override { return false; }
    };

    // Remove a SQLite db file along with its WAL-mode companion files
    // (-wal/-shm/-journal), which would otherwise accumulate in the temp dir.
    void RemoveDbFiles(const std::string& path)
    {
        std::remove(path.c_str());
        std::remove((path + "-wal").c_str());
        std::remove((path + "-shm").c_str());
        std::remove((path + "-journal").c_str());
    }

    // No-op dispatcher that owns queued tasks and frees them, so flushes only
    // run when invoked directly and scheduled tasks (if any) are not leaked.
    class NoopTaskDispatcher : public ITaskDispatcher
    {
    public:
        void Join() override { clear(); }
        void Queue(Task* task) override { m_tasks.push_back(task); }
        bool Cancel(Task* task, uint64_t waitTime = 0) override
        {
            UNREFERENCED_PARAMETER(waitTime);
            auto it = std::find(m_tasks.begin(), m_tasks.end(), task);
            if (it != m_tasks.end())
            {
                delete *it;
                m_tasks.erase(it);
                return true;
            }
            return false;
        }
        ~NoopTaskDispatcher() override { clear(); }

    private:
        void clear()
        {
            for (auto* t : m_tasks)
                delete t;
            m_tasks.clear();
        }
        std::vector<Task*> m_tasks;
    };

    class ManualTaskDispatcher : public ITaskDispatcher
    {
    public:
        enum class Behavior { Hold, Inline, Drop, ThrowOnce };

        void Join() override
        {
            Take();
        }

        void Queue(Task* task) override
        {
            std::unique_ptr<Task> owned(task);
            EXPECT_EQ(static_cast<int>(task->Type), static_cast<int>(Task::Call));
            const unsigned call = ++queueCalls;
            if (beforeQueue)
            {
                beforeQueue();
            }
            if (behavior == Behavior::Inline)
            {
                (*owned)();
                return;
            }
            if (behavior == Behavior::Drop)
            {
                return;
            }
#if HAVE_EXCEPTIONS
            if (behavior == Behavior::ThrowOnce && call == 1)
            {
                throw std::runtime_error("queue failed");
            }
#else
            UNREFERENCED_PARAMETER(call);
#endif
            std::lock_guard<std::mutex> lock(m_mutex);
            EXPECT_EQ(m_task, nullptr);
            m_task = std::move(owned);
        }

        bool Cancel(Task* task, uint64_t = 0) override
        {
            std::unique_ptr<Task> cancelled;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_task.get() != task)
                {
                    return false;
                }
                cancelled = std::move(m_task);
            }
            return true;
        }

        std::unique_ptr<Task> Take()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return std::move(m_task);
        }

        Behavior behavior = Behavior::Hold;
        std::atomic<unsigned> queueCalls{0};
        std::function<void()> beforeQueue;

    private:
        std::mutex m_mutex;
        std::unique_ptr<Task> m_task;
    };

    void EVTSDK_LIBABI_CDECL QueueInlineCapiTask(evt_task_t* task, task_callback_fn_t callback)
    {
        EXPECT_EQ(task->delayMs, 0u);
        callback(task->id);
    }

    bool EVTSDK_LIBABI_CDECL CancelCapiTask(const char*)
    {
        return true;
    }

    void EVTSDK_LIBABI_CDECL JoinCapiTasks() {}
}

namespace MAT_NS_BEGIN {

    class MockOfflineStorageProvider : public IOfflineStorageProvider
    {
    public:
        MockOfflineStorageProvider(
            std::shared_ptr<IOfflineStorage> memory,
            std::shared_ptr<IOfflineStorage> disk)
            : memory(std::move(memory)), disk(std::move(disk))
        {
        }

        std::shared_ptr<IOfflineStorage> CreateDiskStorage(
            ILogManager&, IRuntimeConfig&) override
        {
            return disk;
        }

        std::shared_ptr<IOfflineStorage> CreateMemoryStorage(
            ILogManager&, IRuntimeConfig&) override
        {
            return memory;
        }

    private:
        std::shared_ptr<IOfflineStorage> memory;
        std::shared_ptr<IOfflineStorage> disk;
    };

} MAT_NS_END

TEST_F(OfflineStorageTests, EmptyDiskReadPreservesFirstRamEventProvenance)
{
    NullLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    NoopTaskDispatcher dispatcher;
    NiceMock<MockIOfflineStorageObserver> observer;
    config[CFG_INT_RAM_QUEUE_SIZE] = 1024 * 1024;
    auto memory = std::make_shared<MemoryStorage>(logManager, config);
    auto disk = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto provider = std::make_shared<MockOfflineStorageProvider>(memory, disk);
    OfflineStorageHandler handler(logManager, config, dispatcher, provider);
    EXPECT_CALL(*disk, Initialize(Ref(handler)));
    handler.Initialize(observer);
    StorageObserver storage(testing::getSystem(), handler);
    storage.retrievedEvent >> retrievedEvent;
    storage.retrievalFinished >> retrievalFinished;

    StorageRecord record("oversized", "tenant-token", EventLatency_Normal,
                         EventPersistence_Normal, 1, std::vector<uint8_t>{'x'});
    ASSERT_TRUE(memory->StoreRecord(record));
    auto ctx = std::make_shared<EventsUploadContext>();
    EXPECT_CALL(*this, resultRetrievedEvent(ctx, _, _))
        .WillOnce(Invoke([](EventsUploadContextPtr const& upload, StorageRecord const& event, bool& wantMore)
                         {
                             // Packager includes an oversized first event but refuses subsequent events.
                             upload->recordIdsAndTenantIds.emplace(event.id, event.tenantToken);
                             wantMore = false; }));
    EXPECT_CALL(*disk, GetAndReserveRecords(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*disk, LastReadRecordCount()).WillOnce(Return(0));
    EXPECT_CALL(*this, resultRetrievalFinished(ctx));

    storage.retrieveEvents(ctx);
    EXPECT_TRUE(ctx->fromMemory);
    EXPECT_TRUE(handler.IsLastReadFromMemory());
    EXPECT_EQ(memory->LastReadRecordCount(), 0u);
    EXPECT_EQ(memory->GetRecordCount(), size_t{1});
    EXPECT_TRUE(storage.deleteRecords(ctx));
    EXPECT_EQ(memory->GetRecordCount(), size_t{0});
}

#if HAVE_EXCEPTIONS
class OfflineStorageRetrievalExceptionTests : public OfflineStorageTests,
                                              public WithParamInterface<std::tuple<bool, bool>>
{
};

TEST_P(OfflineStorageRetrievalExceptionTests, PartialRetrievalReleasesRecordsToTheirOriginalStorage)
{
    const bool fromMemory = std::get<0>(GetParam());
    const bool nonstandard = std::get<1>(GetParam());
    NullLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    NoopTaskDispatcher dispatcher;
    NiceMock<MockIOfflineStorageObserver> observer;
    config[CFG_INT_RAM_QUEUE_SIZE] = 1024 * 1024;
    auto memory = std::make_shared<MemoryStorage>(logManager, config);
    auto disk = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto provider = std::make_shared<MockOfflineStorageProvider>(memory, disk);
    OfflineStorageHandler handler(logManager, config, dispatcher, provider);
    EXPECT_CALL(*disk, Initialize(Ref(handler)));
    handler.Initialize(observer);
    StorageObserver storage(testing::getSystem(), handler);
    storage.retrievedEvent >> retrievedEvent;
    storage.retrievalFinished >> retrievalFinished;
    storage.retrievalFailed >> retrievalFailed;

    StorageRecord first("first", "tenant-token", EventLatency_Normal,
                        EventPersistence_Normal, 1, std::vector<uint8_t>{'x'});
    StorageRecord second("second", "tenant-token", EventLatency_Normal,
                         EventPersistence_Normal, 1, std::vector<uint8_t>{'y'});
    if (fromMemory)
    {
        ASSERT_TRUE(memory->StoreRecord(first));
        ASSERT_TRUE(memory->StoreRecord(second));
    }
    else
    {
        EXPECT_CALL(*disk, GetAndReserveRecords(_, _, _, _))
            .WillOnce(Invoke([&](std::function<bool(StorageRecord&&)> const& consumer, unsigned, EventLatency, unsigned)
                             {
                                 EXPECT_TRUE(consumer(std::move(first)));
                                 return consumer(std::move(second)); }));
    }

    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->fromMemory = !fromMemory;
    EXPECT_CALL(*this, resultRetrievedEvent(ctx, _, _))
        .WillOnce(Invoke([](EventsUploadContextPtr const& upload, StorageRecord const& record, bool&)
                         { upload->recordIdsAndTenantIds.emplace(record.id, record.tenantToken); }))
        .WillOnce(Invoke([&](EventsUploadContextPtr const&, StorageRecord const&, bool&)
                         {
                             if (nonstandard)
                             {
                                 throw 7;
                             }
                             throw std::runtime_error("mid-retrieval failure"); }));
    if (nonstandard)
    {
        EXPECT_THROW(storage.retrieveEvents(ctx), int);
    }
    else
    {
        EXPECT_THROW(storage.retrieveEvents(ctx), std::runtime_error);
    }
    ASSERT_EQ(ctx->recordIdsAndTenantIds.size(), size_t{1});
    EXPECT_EQ(ctx->fromMemory, fromMemory);
    EXPECT_EQ(handler.IsLastReadFromMemory(), fromMemory);
    if (fromMemory)
    {
        ASSERT_EQ(memory->GetReservedCount(), size_t{1});
    }
    else
    {
        EXPECT_CALL(*disk, ReleaseRecords(ElementsAre(ctx->recordIdsAndTenantIds.begin()->first), false, _, Eq(false)));
    }
    EXPECT_TRUE(storage.releaseRecords(ctx));
    if (fromMemory)
    {
        EXPECT_EQ(memory->GetReservedCount(), size_t{0});
        std::vector<std::string> retriedIds;
        EXPECT_TRUE(handler.GetAndReserveRecords([&](StorageRecord&& record)
                                                 {
                                                    retriedIds.push_back(record.id);
                                                    return true; }, 120000));
        EXPECT_THAT(retriedIds, UnorderedElementsAre("first", "second"));
        EXPECT_TRUE(handler.IsLastReadFromMemory());
    }
}

INSTANTIATE_TEST_SUITE_P(MemoryAndDisk, OfflineStorageRetrievalExceptionTests, Combine(Bool(), Bool()));
#endif

class OfflineStorageLifecycleTests : public Test
{
protected:
    void SetUp() override
    {
        config[CFG_INT_RAM_QUEUE_SIZE] = 1;
        config[CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH] = true;
        memory = std::make_shared<MemoryStorage>(logManager, config);
        disk = std::make_shared<StrictMock<MockIOfflineStorage>>();
        auto provider = std::make_shared<MockOfflineStorageProvider>(memory, disk);
        handler.reset(new OfflineStorageHandler(logManager, config, dispatcher, provider));
        EXPECT_CALL(*disk, Initialize(Ref(*handler)));
        handler->Initialize(observer);
    }

    StorageRecord Record(const char* id)
    {
        return StorageRecord(id, "tenant-token", EventLatency_Normal,
                             EventPersistence_Normal, 1, std::vector<uint8_t>{'x'});
    }

    bool StoreTwo()
    {
        return handler->StoreRecord(Record("first")) && handler->StoreRecord(Record("second"));
    }

    void ExpectSaved(size_t count)
    {
        EXPECT_CALL(*disk, StoreRecords(_))
            .WillOnce(Invoke([count](std::vector<StorageRecord>& records)
            {
                EXPECT_EQ(records.size(), count);
                return records.size();
            }));
        EXPECT_CALL(observer, OnStorageRecordsSaved(count));
    }

    CountingLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    ManualTaskDispatcher dispatcher;
    StrictMock<MockIOfflineStorageObserver> observer;
    std::shared_ptr<MemoryStorage> memory;
    std::shared_ptr<StrictMock<MockIOfflineStorage>> disk;
    std::unique_ptr<OfflineStorageHandler> handler;
};

TEST_F(OfflineStorageLifecycleTests, ShutdownCancelsQueuedFlushAndPersistsRecords)
{
    ASSERT_TRUE(StoreTwo());
    ExpectSaved(2);
    EXPECT_CALL(*disk, Shutdown());

    std::promise<void> finished;
    auto done = finished.get_future();
    std::thread shutdown([&] {
        handler->Shutdown();
        finished.set_value();
    });
    const auto status = done.wait_for(std::chrono::seconds(5));
    if (status != std::future_status::ready)
    {
        auto task = dispatcher.Take();
        if (task)
        {
            (*task)();
        }
    }
    shutdown.join();
    EXPECT_EQ(status, std::future_status::ready);
    EXPECT_EQ(dispatcher.Take(), nullptr);
    EXPECT_FALSE(handler->StoreRecord(Record("after-shutdown")));
    const unsigned activitiesAfterShutdown = logManager.activityCount.load();
    handler->Flush();
    EXPECT_EQ(logManager.activityCount.load(), activitiesAfterShutdown);
}

TEST_F(OfflineStorageLifecycleTests, ShutdownWaitsForDirectFlushObserver)
{
    ASSERT_TRUE(StoreTwo());
    auto task = dispatcher.Take();
    ASSERT_NE(task, nullptr);
    std::promise<void> entered;
    auto enteredFuture = entered.get_future();
    std::promise<void> release;
    auto releaseFuture = release.get_future().share();
    std::atomic<unsigned> shutdownCalls{0};
    EXPECT_CALL(*disk, StoreRecords(_))
        .WillOnce(Invoke([](std::vector<StorageRecord>& records) { return records.size(); }));
    EXPECT_CALL(observer, OnStorageRecordsSaved(2))
        .WillOnce(Invoke([&](size_t) {
            entered.set_value();
            releaseFuture.wait();
        }));
    EXPECT_CALL(*disk, Shutdown()).WillOnce(Invoke([&] { ++shutdownCalls; }));

    std::thread flush([&] { handler->Flush(); });
    if (enteredFuture.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    {
        release.set_value();
        flush.join();
        FAIL() << "Direct flush did not notify its observer";
    }
    std::promise<void> started;
    auto startedFuture = started.get_future();
    std::promise<void> finished;
    auto done = finished.get_future();
    std::thread shutdown([&] {
        started.set_value();
        handler->Shutdown();
        finished.set_value();
    });
    startedFuture.wait();
    EXPECT_EQ(done.wait_for(std::chrono::milliseconds(200)), std::future_status::timeout);
    EXPECT_EQ(shutdownCalls.load(), 0u);
    release.set_value();
    flush.join();
    shutdown.join();
    EXPECT_EQ(done.wait_for(std::chrono::seconds(0)), std::future_status::ready);
    const unsigned activitiesAfterShutdown = logManager.activityCount.load();
    (*task)();
    EXPECT_EQ(logManager.activityCount.load(), activitiesAfterShutdown);
}

TEST_F(OfflineStorageLifecycleTests, ConcurrentShutdownRunsStorageShutdownOnce)
{
    std::promise<void> entered;
    auto enteredFuture = entered.get_future();
    std::promise<void> release;
    auto releaseFuture = release.get_future().share();
    EXPECT_CALL(*disk, Shutdown()).WillOnce(Invoke([&] {
        entered.set_value();
        releaseFuture.wait();
    }));

    std::thread first([&] { handler->Shutdown(); });
    if (enteredFuture.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    {
        release.set_value();
        first.join();
        FAIL() << "First shutdown did not reach storage";
    }
    std::promise<void> started;
    auto startedFuture = started.get_future();
    std::promise<void> finished;
    auto done = finished.get_future();
    std::thread second([&] {
        started.set_value();
        handler->Shutdown();
        finished.set_value();
    });
    startedFuture.wait();
    EXPECT_EQ(done.wait_for(std::chrono::milliseconds(200)), std::future_status::timeout);
    release.set_value();
    first.join();
    second.join();
    EXPECT_EQ(done.wait_for(std::chrono::seconds(0)), std::future_status::ready);
    handler->Shutdown();
}

TEST_F(OfflineStorageLifecycleTests, ShutdownWaitsForRunningFlush)
{
    ASSERT_TRUE(StoreTwo());
    auto task = dispatcher.Take();
    ASSERT_NE(task, nullptr);
    std::promise<void> entered;
    auto enteredFuture = entered.get_future();
    std::promise<void> release;
    auto releaseFuture = release.get_future().share();
    std::atomic<unsigned> batchCalls{0};
    std::atomic<unsigned> directlyStored{0};
    EXPECT_CALL(*disk, StoreRecords(_))
        .WillRepeatedly(Invoke([&](std::vector<StorageRecord>& records) {
            if (++batchCalls == 1)
            {
                entered.set_value();
                releaseFuture.wait();
            }
            return records.size();
        }));
    EXPECT_CALL(*disk, StoreRecord(_))
        .WillRepeatedly(Invoke([&](StorageRecord const&) {
            ++directlyStored;
            return true;
        }));
    EXPECT_CALL(observer, OnStorageRecordsSaved(_)).Times(AtLeast(1));
    EXPECT_CALL(*disk, Shutdown());

    std::thread worker([task = std::move(task)] { (*task)(); });
    if (enteredFuture.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    {
        release.set_value();
        worker.join();
        FAIL() << "Scheduled flush did not enter storage";
    }
    std::promise<void> started;
    auto startedFuture = started.get_future();
    std::promise<void> finished;
    auto done = finished.get_future();
    std::thread shutdown([&] {
        started.set_value();
        handler->Shutdown();
        finished.set_value();
    });
    startedFuture.wait();
    for (unsigned i = 0; i < 500 && directlyStored.load() == 0; ++i)
    {
        auto id = "probe-" + std::to_string(i);
        handler->StoreRecord(Record(id.c_str()));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(directlyStored.load(), 1u);
    EXPECT_EQ(done.wait_for(std::chrono::seconds(0)), std::future_status::timeout);
    release.set_value();
    worker.join();
    shutdown.join();
    EXPECT_EQ(done.wait_for(std::chrono::seconds(0)), std::future_status::ready);
}

TEST_F(OfflineStorageLifecycleTests, StoresDuringDrainReachDiskBeforeShutdown)
{
    ASSERT_TRUE(handler->StoreRecord(Record("first")));
    std::promise<void> entered;
    auto enteredFuture = entered.get_future();
    std::promise<void> release;
    auto releaseFuture = release.get_future().share();
    dispatcher.beforeQueue = [&] {
        entered.set_value();
        releaseFuture.wait();
    };
    std::atomic<size_t> directlyStored{0};
    size_t batched = 0;
    EXPECT_CALL(*disk, StoreRecord(_))
        .WillRepeatedly(Invoke([&](StorageRecord const&) {
            ++directlyStored;
            return true;
        }));
    EXPECT_CALL(*disk, StoreRecords(_))
        .WillOnce(Invoke([&](std::vector<StorageRecord>& records) {
            batched = records.size();
            return records.size();
        }));
    EXPECT_CALL(observer, OnStorageRecordsSaved(_));
    EXPECT_CALL(*disk, Shutdown());

    std::thread producer([&] { EXPECT_TRUE(handler->StoreRecord(Record("second"))); });
    if (enteredFuture.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    {
        release.set_value();
        producer.join();
        FAIL() << "Store did not reach task submission";
    }
    std::promise<void> started;
    auto startedFuture = started.get_future();
    std::promise<void> finished;
    auto done = finished.get_future();
    std::thread shutdown([&] {
        started.set_value();
        handler->Shutdown();
        finished.set_value();
    });
    startedFuture.wait();
    size_t accepted = 2;
    for (unsigned i = 0; i < 500 && directlyStored.load() == 0; ++i)
    {
        auto id = "during-drain-" + std::to_string(i);
        if (handler->StoreRecord(Record(id.c_str())))
        {
            ++accepted;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(directlyStored.load(), 1u);
    EXPECT_EQ(done.wait_for(std::chrono::seconds(0)), std::future_status::timeout);
    release.set_value();
    producer.join();
    shutdown.join();
    EXPECT_EQ(batched + directlyStored.load(), accepted);
    EXPECT_EQ(dispatcher.Take(), nullptr);
}

TEST_F(OfflineStorageLifecycleTests, DroppedTaskAllowsAnotherFlushToBeScheduled)
{
    dispatcher.behavior = ManualTaskDispatcher::Behavior::Drop;
    ASSERT_TRUE(StoreTwo());
    ASSERT_TRUE(handler->StoreRecord(Record("third")));
    EXPECT_EQ(dispatcher.queueCalls.load(), 2u);
    ExpectSaved(3);
    EXPECT_CALL(*disk, Shutdown());
    handler->Shutdown();
}

TEST_F(OfflineStorageLifecycleTests, InlineDispatcherRunsFlushWithoutDeadlock)
{
    dispatcher.behavior = ManualTaskDispatcher::Behavior::Inline;
    ExpectSaved(2);
    ASSERT_TRUE(StoreTwo());
    EXPECT_EQ(dispatcher.queueCalls.load(), 1u);
    EXPECT_CALL(*disk, Shutdown());
    handler->Shutdown();
}

#if HAVE_EXCEPTIONS
TEST_F(OfflineStorageLifecycleTests, FailedQueueDoesNotBlockLaterScheduling)
{
    dispatcher.behavior = ManualTaskDispatcher::Behavior::ThrowOnce;
    ASSERT_TRUE(handler->StoreRecord(Record("first")));
    EXPECT_THROW(handler->StoreRecord(Record("second")), std::runtime_error);
    ASSERT_TRUE(handler->StoreRecord(Record("third")));
    EXPECT_EQ(dispatcher.queueCalls.load(), 2u);
    ExpectSaved(3);
    EXPECT_CALL(*disk, Shutdown());
    handler->Shutdown();
}

TEST_F(OfflineStorageLifecycleTests, FailedDirectFlushReleasesActivityAndCanBeRetried)
{
    ASSERT_TRUE(StoreTwo());
    EXPECT_CALL(*disk, StoreRecords(_))
        .WillOnce(Invoke([](std::vector<StorageRecord>& records) -> size_t {
            records.clear();
            throw std::runtime_error("disk write failed");
        }))
        .WillOnce(Invoke([](std::vector<StorageRecord>& records) { return records.size(); }));
    EXPECT_CALL(observer, OnStorageRecordsSaved(2));
    EXPECT_THROW(handler->Flush(), std::runtime_error);
    EXPECT_EQ(logManager.activeActivities.load(), 0);
    EXPECT_EQ(memory->GetRecordCount(), 2u);
    handler->Flush();
    EXPECT_EQ(logManager.activeActivities.load(), 0);
    EXPECT_EQ(memory->GetRecordCount(), 0u);
    EXPECT_CALL(*disk, Shutdown());
    handler->Shutdown();
}

TEST_F(OfflineStorageLifecycleTests, ConcurrentShutdownRetriesFailedAttempt)
{
    std::promise<void> entered;
    auto enteredFuture = entered.get_future();
    std::promise<void> release;
    auto releaseFuture = release.get_future().share();
    EXPECT_CALL(*disk, Shutdown())
        .WillOnce(Invoke([&] {
            entered.set_value();
            releaseFuture.wait();
            throw std::runtime_error("shutdown failed");
        }))
        .WillOnce(Return());

    std::thread first([&] { EXPECT_THROW(handler->Shutdown(), std::runtime_error); });
    if (enteredFuture.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    {
        release.set_value();
        first.join();
        FAIL() << "First shutdown did not reach storage";
    }
    std::promise<void> started;
    auto startedFuture = started.get_future();
    std::promise<void> finished;
    auto done = finished.get_future();
    std::thread second([&] {
        started.set_value();
        EXPECT_NO_THROW(handler->Shutdown());
        finished.set_value();
    });
    startedFuture.wait();
    EXPECT_EQ(done.wait_for(std::chrono::milliseconds(200)), std::future_status::timeout);
    release.set_value();
    first.join();
    second.join();
    EXPECT_EQ(done.wait_for(std::chrono::seconds(0)), std::future_status::ready);
    handler->Shutdown();
}

TEST_F(OfflineStorageLifecycleTests, FailedShutdownFlushCanBeRetried)
{
    ASSERT_TRUE(StoreTwo());
    EXPECT_CALL(*disk, StoreRecords(_))
        .WillOnce(Throw(std::runtime_error("disk write failed")))
        .WillOnce(Invoke([](std::vector<StorageRecord>& records) { return records.size(); }));
    EXPECT_CALL(observer, OnStorageRecordsSaved(2));
    EXPECT_CALL(*disk, Shutdown());
    EXPECT_THROW(handler->Shutdown(), std::runtime_error);
    EXPECT_EQ(memory->GetRecordCount(), 2u);
    handler->Shutdown();
    EXPECT_EQ(memory->GetRecordCount(), 0u);
}

TEST_F(OfflineStorageLifecycleTests, ReinitializeRetriesFailedShutdownBeforeReplacingStorage)
{
    ASSERT_TRUE(StoreTwo());
    EXPECT_CALL(*disk, StoreRecords(_))
        .WillOnce(Throw(std::runtime_error("disk write failed")))
        .WillOnce(Invoke([](std::vector<StorageRecord>& records) { return records.size(); }));
    EXPECT_CALL(observer, OnStorageRecordsSaved(2));
    EXPECT_CALL(*disk, Shutdown()).Times(2);
    EXPECT_THROW(handler->Shutdown(), std::runtime_error);
    EXPECT_CALL(*disk, Initialize(Ref(*handler)));
    handler->Initialize(observer);
    EXPECT_EQ(memory->GetRecordCount(), 0u);
    handler->Shutdown();
}

TEST(OfflineStorageHandlerFlushTests, RetryingDiskShutdownDoesNotRepeatMemoryShutdown)
{
    ConfigurableLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    ManualTaskDispatcher dispatcher;
    StrictMock<MockIOfflineStorageObserver> observer;
    config[CFG_INT_RAM_QUEUE_SIZE] = 1024 * 1024;
    config[CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH] = true;
    auto memory = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto disk = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto provider = std::make_shared<MockOfflineStorageProvider>(memory, disk);
    OfflineStorageHandler handler(logManager, config, dispatcher, provider);
    EXPECT_CALL(*disk, Initialize(Ref(handler)));
    EXPECT_CALL(*memory, Initialize(Ref(handler)));
    handler.Initialize(observer);
    EXPECT_CALL(*memory, GetSize()).WillOnce(Return(0));
    EXPECT_CALL(*memory, Shutdown()).Times(1);
    EXPECT_CALL(*disk, Shutdown())
        .WillOnce(Throw(std::runtime_error("shutdown failed")))
        .WillOnce(Return());
    EXPECT_THROW(handler.Shutdown(), std::runtime_error);
    handler.Shutdown();
}
#endif

TEST_F(OfflineStorageLifecycleTests, PartialBatchMutationRetainsRecordsForRetry)
{
    ASSERT_TRUE(StoreTwo());
    EXPECT_CALL(*disk, StoreRecords(_))
        .WillOnce(Invoke([](std::vector<StorageRecord>& records) -> size_t {
            records.clear();
            return 1;
        }))
        .WillOnce(Invoke([](std::vector<StorageRecord>& records) {
            EXPECT_EQ(records.size(), 2u);
            return records.size();
        }));
    EXPECT_CALL(observer, OnStorageRecordsSaved(1));
    EXPECT_CALL(observer, OnStorageRecordsSaved(2));
    handler->Flush();
    EXPECT_EQ(memory->GetRecordCount(), 2u);
    handler->Flush();
    EXPECT_EQ(memory->GetRecordCount(), 0u);
    EXPECT_CALL(*disk, Shutdown());
    handler->Shutdown();
}

TEST_F(OfflineStorageLifecycleTests, SavedObserverCanWaitForAnotherFlush)
{
    ASSERT_TRUE(StoreTwo());
    EXPECT_CALL(*disk, StoreRecords(_))
        .WillOnce(Invoke([](std::vector<StorageRecord>& records) { return records.size(); }));
    std::promise<void> finished;
    auto done = finished.get_future();
    std::thread nested;
    EXPECT_CALL(observer, OnStorageRecordsSaved(2))
        .WillOnce(Invoke([&](size_t) {
            nested = std::thread([&] {
                handler->Flush();
                finished.set_value();
            });
            EXPECT_EQ(done.wait_for(std::chrono::seconds(5)), std::future_status::ready);
        }));
    handler->Flush();
    if (nested.joinable())
    {
        nested.join();
    }
    EXPECT_CALL(*disk, Shutdown());
    handler->Shutdown();
}

TEST_F(OfflineStorageLifecycleTests, ReinitializeWithoutMemoryCacheStoresDirectlyToDisk)
{
    config[CFG_INT_RAM_QUEUE_SIZE] = 0;
    EXPECT_CALL(*disk, Shutdown());
    EXPECT_CALL(*disk, Initialize(Ref(*handler)));
    handler->Initialize(observer);

    EXPECT_CALL(*disk, StoreRecord(_)).WillOnce(Return(true));
    EXPECT_TRUE(handler->StoreRecord(Record("direct")));
    EXPECT_EQ(memory->GetRecordCount(), 0u);

    EXPECT_CALL(*disk, Shutdown());
    handler->Shutdown();
}

TEST_F(OfflineStorageLifecycleTests, OldDequeuedTaskStaysInertAfterReinitialize)
{
    ASSERT_TRUE(StoreTwo());
    auto oldTask = dispatcher.Take();
    ASSERT_NE(oldTask, nullptr);
    ExpectSaved(2);
    EXPECT_CALL(*disk, Shutdown());
    handler->Shutdown();

    EXPECT_CALL(*disk, Initialize(Ref(*handler)));
    handler->Initialize(observer);
    ASSERT_TRUE(StoreTwo());
    const unsigned activitiesBeforeTask = logManager.activityCount.load();
    (*oldTask)();
    oldTask.reset();
    EXPECT_EQ(logManager.activityCount.load(), activitiesBeforeTask);
    auto currentTask = dispatcher.Take();
    ASSERT_NE(currentTask, nullptr);
    ExpectSaved(2);
    (*currentTask)();
    EXPECT_GT(logManager.activityCount.load(), activitiesBeforeTask);
    EXPECT_EQ(memory->GetRecordCount(), 0u);
    EXPECT_CALL(*disk, Shutdown());
    handler->Shutdown();
}

TEST_F(OfflineStorageLifecycleTests, ReinitializeWhileAcceptingClosesOldGeneration)
{
    ASSERT_TRUE(StoreTwo());
    auto oldTask = dispatcher.Take();
    ASSERT_NE(oldTask, nullptr);
    ExpectSaved(2);
    EXPECT_CALL(*disk, Shutdown());
    EXPECT_CALL(*disk, Initialize(Ref(*handler)));
    handler->Initialize(observer);

    ASSERT_TRUE(StoreTwo());
    const unsigned activitiesBeforeTask = logManager.activityCount.load();
    (*oldTask)();
    oldTask.reset();
    EXPECT_EQ(logManager.activityCount.load(), activitiesBeforeTask);
    auto currentTask = dispatcher.Take();
    ASSERT_NE(currentTask, nullptr);
    ExpectSaved(2);
    (*currentTask)();
    EXPECT_EQ(memory->GetRecordCount(), 0u);
    EXPECT_CALL(*disk, Shutdown());
    handler->Shutdown();
}

TEST_F(OfflineStorageLifecycleTests, OldCompletionDoesNotClearNewerQueuedFlush)
{
    ASSERT_TRUE(StoreTwo());
    auto oldTask = dispatcher.Take();
    ASSERT_NE(oldTask, nullptr);
    std::promise<void> entered;
    auto enteredFuture = entered.get_future();
    std::promise<void> release;
    auto releaseFuture = release.get_future().share();
    EXPECT_CALL(*disk, StoreRecords(_))
        .WillOnce(Invoke([&](std::vector<StorageRecord>& records) {
            entered.set_value();
            releaseFuture.wait();
            return records.size();
        }))
        .WillOnce(Invoke([](std::vector<StorageRecord>& records) { return records.size(); }));
    EXPECT_CALL(observer, OnStorageRecordsSaved(2)).Times(2);
    std::thread worker([task = std::move(oldTask)] { (*task)(); });
    if (enteredFuture.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    {
        release.set_value();
        worker.join();
        FAIL() << "Scheduled flush did not enter storage";
    }
    EXPECT_TRUE(handler->StoreRecord(Record("third")));
    EXPECT_TRUE(handler->StoreRecord(Record("fourth")));
    release.set_value();
    worker.join();
    auto newTask = dispatcher.Take();
    ASSERT_NE(newTask, nullptr);
    (*newTask)();
    EXPECT_EQ(memory->GetRecordCount(), 0u);
    EXPECT_CALL(*disk, Shutdown());
    handler->Shutdown();
}

TEST(OfflineStorageHandlerFlushTests, ShutdownFlushesWhenActivitiesArePaused)
{
    PausedLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    ManualTaskDispatcher dispatcher;
    StrictMock<MockIOfflineStorageObserver> observer;
    config[CFG_INT_RAM_QUEUE_SIZE] = 1024 * 1024;
    config[CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH] = true;
    auto memory = std::make_shared<MemoryStorage>(logManager, config);
    auto disk = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto provider = std::make_shared<MockOfflineStorageProvider>(memory, disk);
    OfflineStorageHandler handler(logManager, config, dispatcher, provider);
    EXPECT_CALL(*disk, Initialize(Ref(handler)));
    handler.Initialize(observer);
    StorageRecord record("event", "tenant-token", EventLatency_Normal,
                         EventPersistence_Normal, 1, std::vector<uint8_t>{'x'});
    ASSERT_TRUE(handler.StoreRecord(record));
    handler.Flush();
    EXPECT_CALL(*disk, StoreRecords(_)).WillOnce(Return(1));
    EXPECT_CALL(observer, OnStorageRecordsSaved(1));
    EXPECT_CALL(*disk, Shutdown());
    handler.Shutdown();
}

TEST(OfflineStorageHandlerFlushTests, CapiDispatcherReceivesImmediateFlush)
{
    ConfigurableLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    PAL::TaskDispatcher_CAPI dispatcher(&QueueInlineCapiTask, &CancelCapiTask, &JoinCapiTasks);
    StrictMock<MockIOfflineStorageObserver> observer;
    config[CFG_INT_RAM_QUEUE_SIZE] = 1;
    config[CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH] = true;
    auto memory = std::make_shared<MemoryStorage>(logManager, config);
    auto disk = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto provider = std::make_shared<MockOfflineStorageProvider>(memory, disk);
    OfflineStorageHandler handler(logManager, config, dispatcher, provider);
    EXPECT_CALL(*disk, Initialize(Ref(handler)));
    handler.Initialize(observer);
    EXPECT_CALL(*disk, StoreRecords(_)).WillOnce(Return(2));
    EXPECT_CALL(observer, OnStorageRecordsSaved(2));
    StorageRecord first("first", "tenant-token", EventLatency_Normal,
                        EventPersistence_Normal, 1, std::vector<uint8_t>{'x'});
    StorageRecord second("second", "tenant-token", EventLatency_Normal,
                         EventPersistence_Normal, 1, std::vector<uint8_t>{'y'});
    ASSERT_TRUE(handler.StoreRecord(first));
    ASSERT_TRUE(handler.StoreRecord(second));
    EXPECT_CALL(*disk, Shutdown());
    handler.Shutdown();
}

TEST(OfflineStorageHandlerFlushTests, DequeuedFlushCannotRunAfterShutdown)
{
    CountingLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    ManualTaskDispatcher dispatcher;
    StrictMock<MockIOfflineStorageObserver> observer;
    config[CFG_INT_RAM_QUEUE_SIZE] = 1;
    config[CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH] = true;
    auto memory = std::make_shared<MemoryStorage>(logManager, config);
    auto disk = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto provider = std::make_shared<MockOfflineStorageProvider>(memory, disk);
    OfflineStorageHandler handler(logManager, config, dispatcher, provider);
    EXPECT_CALL(*disk, Initialize(Ref(handler)));
    handler.Initialize(observer);

    StorageRecord first("first", "tenant-token", EventLatency_Normal,
                        EventPersistence_Normal, 1, std::vector<uint8_t>{'x'});
    StorageRecord second("second", "tenant-token", EventLatency_Normal,
                         EventPersistence_Normal, 1, std::vector<uint8_t>{'y'});
    ASSERT_TRUE(handler.StoreRecord(first));
    ASSERT_TRUE(handler.StoreRecord(second));
    auto task = dispatcher.Take();
    ASSERT_NE(task, nullptr);

    EXPECT_CALL(*disk, StoreRecords(_))
        .WillOnce(Invoke([](std::vector<StorageRecord>& records) { return records.size(); }));
    EXPECT_CALL(observer, OnStorageRecordsSaved(2));
    handler.Flush();

    EXPECT_CALL(*disk, Shutdown());
    handler.Shutdown();
    const unsigned activitiesBeforeTask = logManager.activityCount.load();
    (*task)();
    EXPECT_EQ(logManager.activityCount.load(), activitiesBeforeTask);
}

TEST(OfflineStorageHandlerFlushTests, FailedMemoryRequeueIsReportedAndDropped)
{
    NullLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    NoopTaskDispatcher dispatcher;
    StrictMock<MockIOfflineStorageObserver> observer;

    auto memory = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto disk = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto provider = std::make_shared<MockOfflineStorageProvider>(memory, disk);
    OfflineStorageHandler handler(logManager, config, dispatcher, provider);
    config[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;
    config[CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH] = true;
    EXPECT_CALL(*memory, Initialize(Ref(handler))).WillOnce(Return());
    EXPECT_CALL(*disk, Initialize(Ref(handler))).WillOnce(Return());
    handler.Initialize(observer);

    std::vector<StorageRecord> records;
    records.push_back(StorageRecord("retry-ok", "tenant-one-token",
        EventLatency_Normal, EventPersistence_Normal, /*timestamp*/ 1,
        std::vector<uint8_t>{ 'x' }));
    records.push_back(StorageRecord("retry-drop", "tenant-two-token",
        EventLatency_Normal, EventPersistence_Normal, /*timestamp*/ 1,
        std::vector<uint8_t>{ 'y' }));

    EXPECT_CALL(*memory, GetSize())
        .WillOnce(Return(records.size()))
        .WillOnce(Return(records.size()));
    EXPECT_CALL(*memory, GetRecordCount(EventLatency_Unspecified))
        .WillOnce(Return(records.size()));
    EXPECT_CALL(*memory, GetRecords(false, EventLatency_Unspecified, 2000))
        .WillOnce(Return(records));
    EXPECT_CALL(*disk, StoreRecords(_)).WillOnce(Return(0));
    EXPECT_CALL(*memory, StoreRecord(_))
        .WillOnce(Return(true))
        .WillOnce(Return(false));
    EXPECT_CALL(observer, OnStorageRecordsSaved(0));
    EXPECT_CALL(observer, OnStorageRecordsDropped(_))
        .WillOnce(Invoke([](std::map<std::string, size_t> const& dropped) {
            auto found = dropped.find("tenant-two-token");
            ASSERT_NE(found, dropped.end());
            EXPECT_EQ(found->second, static_cast<size_t>(1));
        }));

    handler.Flush();
}

TEST(OfflineStorageHandlerFlushTests, BatchingOptOutUsesPerRecordDiskStores)
{
    NullLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    NoopTaskDispatcher dispatcher;
    StrictMock<MockIOfflineStorageObserver> observer;

    config[CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH] = false;
    config[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;

    auto memory = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto disk = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto provider = std::make_shared<MockOfflineStorageProvider>(memory, disk);
    OfflineStorageHandler handler(logManager, config, dispatcher, provider);
    EXPECT_CALL(*memory, Initialize(Ref(handler))).WillOnce(Return());
    EXPECT_CALL(*disk, Initialize(Ref(handler))).WillOnce(Return());
    handler.Initialize(observer);

    std::vector<StorageRecord> records;
    records.push_back(StorageRecord("per-record-1", "tenant-one-token",
        EventLatency_Normal, EventPersistence_Normal, /*timestamp*/ 1,
        std::vector<uint8_t>{ 'x' }));
    records.push_back(StorageRecord("per-record-2", "tenant-two-token",
        EventLatency_Normal, EventPersistence_Normal, /*timestamp*/ 1,
        std::vector<uint8_t>{ 'y' }));

    EXPECT_CALL(*memory, GetSize())
        .WillOnce(Return(static_cast<size_t>(records.size())))
        .WillOnce(Return(static_cast<size_t>(0)));
    EXPECT_CALL(*memory, GetRecords(false, EventLatency_Unspecified, 0))
        .WillOnce(Return(records));
    EXPECT_CALL(*disk, StoreRecords(_)).Times(0);
    EXPECT_CALL(*disk, StoreRecord(_))
        .Times(static_cast<int>(records.size()))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(observer, OnStorageRecordsSaved(records.size()));

    handler.Flush();
}

TEST(OfflineStorageHandlerFlushTests, BatchedFlushKeepsMemoryOnlyRecordsOffDisk)
{
    NullLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    NoopTaskDispatcher dispatcher;
    StrictMock<MockIOfflineStorageObserver> observer;

    config[CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH] = true;
    config[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;

    auto memory = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto disk = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto provider = std::make_shared<MockOfflineStorageProvider>(memory, disk);
    OfflineStorageHandler handler(logManager, config, dispatcher, provider);
    EXPECT_CALL(*memory, Initialize(Ref(handler))).WillOnce(Return());
    EXPECT_CALL(*disk, Initialize(Ref(handler))).WillOnce(Return());
    handler.Initialize(observer);

    std::vector<StorageRecord> records;
    records.push_back(StorageRecord("persisted", "tenant-token",
                                    EventLatency_Normal, EventPersistence_Normal, 1, std::vector<uint8_t>{'x'}));
    records.push_back(StorageRecord("memory-only", "tenant-token",
                                    EventLatency_Normal, EventPersistence_DoNotStoreOnDisk, 1, std::vector<uint8_t>{'y'}));

    EXPECT_CALL(*memory, GetSize())
        .WillOnce(Return(records.size()))
        .WillOnce(Return(static_cast<size_t>(1)));
    EXPECT_CALL(*memory, GetRecordCount(EventLatency_Unspecified))
        .WillOnce(Return(records.size()));
    EXPECT_CALL(*memory, GetRecords(false, EventLatency_Unspecified, 2000))
        .WillOnce(Return(records));
    EXPECT_CALL(*memory, StoreRecord(Field(&StorageRecord::id, "memory-only")))
        .WillOnce(Return(true));
    EXPECT_CALL(*disk, StoreRecord(_)).Times(0);
    EXPECT_CALL(*disk, StoreRecords(_))
        .WillOnce(Invoke([](std::vector<StorageRecord>& batch) -> size_t {
            EXPECT_THAT(batch, ElementsAre(Field(&StorageRecord::id, "persisted")));
            batch.clear();
            return 1;
        }));
    EXPECT_CALL(observer, OnStorageRecordsSaved(1));

    handler.Flush();
}

TEST(OfflineStorageHandlerFlushTests, PerRecordFlushKeepsMemoryOnlyRecordsOffDisk)
{
    NullLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    NoopTaskDispatcher dispatcher;
    StrictMock<MockIOfflineStorageObserver> observer;

    config[CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH] = false;
    config[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;

    auto memory = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto disk = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto provider = std::make_shared<MockOfflineStorageProvider>(memory, disk);
    OfflineStorageHandler handler(logManager, config, dispatcher, provider);
    EXPECT_CALL(*memory, Initialize(Ref(handler))).WillOnce(Return());
    EXPECT_CALL(*disk, Initialize(Ref(handler))).WillOnce(Return());
    handler.Initialize(observer);

    std::vector<StorageRecord> records;
    records.push_back(StorageRecord("persisted", "tenant-token",
                                    EventLatency_Normal, EventPersistence_Normal, 1, std::vector<uint8_t>{'x'}));
    records.push_back(StorageRecord("memory-only", "tenant-token",
                                    EventLatency_Normal, EventPersistence_DoNotStoreOnDisk, 1, std::vector<uint8_t>{'y'}));

    EXPECT_CALL(*memory, GetSize())
        .WillOnce(Return(records.size()))
        .WillOnce(Return(static_cast<size_t>(1)));
    EXPECT_CALL(*memory, GetRecords(false, EventLatency_Unspecified, 0))
        .WillOnce(Return(records));
    EXPECT_CALL(*memory, StoreRecord(Field(&StorageRecord::id, "memory-only")))
        .WillOnce(Return(true));
    EXPECT_CALL(*disk, StoreRecords(_)).Times(0);
    EXPECT_CALL(*disk, StoreRecord(Field(&StorageRecord::id, "persisted")))
        .WillOnce(Return(true));
    EXPECT_CALL(observer, OnStorageRecordsSaved(1));

    handler.Flush();
}

TEST(OfflineStorageHandlerFlushTests, BatchedFlushLimitsEachDiskWrite)
{
    NullLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    NoopTaskDispatcher dispatcher;
    StrictMock<MockIOfflineStorageObserver> observer;

    config[CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH] = true;
    config[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;

    auto memory = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto disk = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto provider = std::make_shared<MockOfflineStorageProvider>(memory, disk);
    OfflineStorageHandler handler(logManager, config, dispatcher, provider);
    EXPECT_CALL(*memory, Initialize(Ref(handler))).WillOnce(Return());
    EXPECT_CALL(*disk, Initialize(Ref(handler))).WillOnce(Return());
    handler.Initialize(observer);

    std::vector<StorageRecord> firstBatch;
    std::vector<StorageRecord> secondBatch;
    std::vector<StorageRecord> finalBatch;
    for (size_t i = 0; i < 4005; ++i)
    {
        StorageRecord record("batch-" + std::to_string(i), "tenant-token",
            EventLatency_Normal, EventPersistence_Normal, /*timestamp*/ 1,
            std::vector<uint8_t>{ 'x' });
        if (i < 2000)
        {
            firstBatch.push_back(record);
        }
        else if (i < 4000)
        {
            secondBatch.push_back(record);
        }
        else
        {
            finalBatch.push_back(record);
        }
    }

    EXPECT_CALL(*memory, GetSize())
        .WillOnce(Return(static_cast<size_t>(4005)))
        .WillOnce(Return(static_cast<size_t>(0)));
    EXPECT_CALL(*memory, GetRecordCount(EventLatency_Unspecified))
        .WillOnce(Return(static_cast<size_t>(4005)));
    EXPECT_CALL(*memory, GetRecords(false, EventLatency_Unspecified, 2000))
        .WillOnce(Return(firstBatch))
        .WillOnce(Return(secondBatch))
        .WillOnce(Return(finalBatch));
    EXPECT_CALL(*disk, StoreRecords(_))
        .WillOnce(Invoke([](std::vector<StorageRecord>& records) {
            EXPECT_EQ(records.size(), static_cast<size_t>(2000));
            return records.size();
        }))
        .WillOnce(Invoke([](std::vector<StorageRecord>& records) {
            EXPECT_EQ(records.size(), static_cast<size_t>(2000));
            return records.size();
        }))
        .WillOnce(Invoke([](std::vector<StorageRecord>& records) {
            EXPECT_EQ(records.size(), static_cast<size_t>(5));
            return records.size();
        }));
    EXPECT_CALL(observer, OnStorageRecordsSaved(4005));

    handler.Flush();
}

TEST(OfflineStorageHandlerFlushTests, BatchedFlushDefersMemoryOnlyRequeueUntilSnapshotIsDrained)
{
    NullLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    NoopTaskDispatcher dispatcher;
    StrictMock<MockIOfflineStorageObserver> observer;

    config[CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH] = true;
    config[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 5000;

    auto memory = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto disk = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto provider = std::make_shared<MockOfflineStorageProvider>(memory, disk);
    OfflineStorageHandler handler(logManager, config, dispatcher, provider);
    EXPECT_CALL(*memory, Initialize(Ref(handler))).WillOnce(Return());
    EXPECT_CALL(*disk, Initialize(Ref(handler))).WillOnce(Return());
    handler.Initialize(observer);

    std::vector<StorageRecord> firstBatch;
    std::vector<StorageRecord> secondBatch;
    for (size_t i = 0; i < 4000; ++i)
    {
        StorageRecord record(
            "memory-only-" + std::to_string(i), "tenant-token",
            EventLatency_Normal, EventPersistence_DoNotStoreOnDisk, 1,
            std::vector<uint8_t>{'x'});
        (i < 2000 ? firstBatch : secondBatch).push_back(std::move(record));
    }
    std::vector<StorageRecord> finalBatch {
        StorageRecord(
            "persistent", "tenant-token", EventLatency_Normal,
            EventPersistence_Normal, 1, std::vector<uint8_t>{'y'})
    };

    EXPECT_CALL(*memory, GetSize())
        .WillOnce(Return(static_cast<size_t>(4001)))
        .WillOnce(Return(static_cast<size_t>(4000)));
    EXPECT_CALL(*memory, GetRecordCount(EventLatency_Unspecified))
        .WillOnce(Return(static_cast<size_t>(4001)));

    {
        InSequence sequence;
        EXPECT_CALL(*memory, GetRecords(false, EventLatency_Unspecified, 2000))
            .WillOnce(Return(firstBatch));
        EXPECT_CALL(*memory, GetRecords(false, EventLatency_Unspecified, 2000))
            .WillOnce(Return(secondBatch));
        EXPECT_CALL(*memory, GetRecords(false, EventLatency_Unspecified, 2000))
            .WillOnce(Return(finalBatch));
        EXPECT_CALL(*disk, StoreRecords(_)).WillOnce(Return(1));
        EXPECT_CALL(*memory, StoreRecord(_))
            .Times(4000)
            .WillRepeatedly(Return(true));
    }
    EXPECT_CALL(observer, OnStorageRecordsSaved(1));

    handler.Flush();
}

TEST(OfflineStorageHandlerFlushTests, BatchedFlushRecoversMemoryOnlyRecordsWhenLaterBatchThrows)
{
    NullLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    NoopTaskDispatcher dispatcher;
    StrictMock<MockIOfflineStorageObserver> observer;

    config[CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH] = true;
    config[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 3000;

    auto memory = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto disk = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto provider = std::make_shared<MockOfflineStorageProvider>(memory, disk);
    OfflineStorageHandler handler(logManager, config, dispatcher, provider);
    EXPECT_CALL(*memory, Initialize(Ref(handler))).WillOnce(Return());
    EXPECT_CALL(*disk, Initialize(Ref(handler))).WillOnce(Return());
    handler.Initialize(observer);

    std::vector<StorageRecord> firstBatch;
    for (size_t i = 0; i < 2000; ++i)
    {
        firstBatch.emplace_back(
            "memory-only-" + std::to_string(i), "tenant-token",
            EventLatency_Normal, EventPersistence_DoNotStoreOnDisk, 1,
            std::vector<uint8_t>{'x'});
    }
    std::vector<StorageRecord> throwingBatch {
        StorageRecord(
            "persistent", "tenant-token", EventLatency_Normal,
            EventPersistence_Normal, 1, std::vector<uint8_t>{'y'})
    };

    EXPECT_CALL(*memory, GetSize()).WillOnce(Return(static_cast<size_t>(2001)));
    EXPECT_CALL(*memory, GetRecordCount(EventLatency_Unspecified))
        .WillOnce(Return(static_cast<size_t>(2001)));
    EXPECT_CALL(*memory, GetRecords(false, EventLatency_Unspecified, 2000))
        .WillOnce(Return(firstBatch))
        .WillOnce(Return(throwingBatch));
    EXPECT_CALL(*disk, StoreRecords(_))
        .WillOnce(Throw(std::runtime_error("disk write failed")));
    EXPECT_CALL(*memory, StoreRecord(_))
        .Times(2001)
        .WillRepeatedly(Return(true));

    EXPECT_THROW(handler.Flush(), std::runtime_error);
}

TEST(OfflineStorageHandlerFlushTests, FailedBatchRequeuesOnlyThatBatch)
{
    NullLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    NoopTaskDispatcher dispatcher;
    StrictMock<MockIOfflineStorageObserver> observer;

    config[CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH] = true;
    config[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;

    auto memory = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto disk = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto provider = std::make_shared<MockOfflineStorageProvider>(memory, disk);
    OfflineStorageHandler handler(logManager, config, dispatcher, provider);
    EXPECT_CALL(*memory, Initialize(Ref(handler))).WillOnce(Return());
    EXPECT_CALL(*disk, Initialize(Ref(handler))).WillOnce(Return());
    handler.Initialize(observer);

    std::vector<StorageRecord> firstBatch;
    std::vector<StorageRecord> failedBatch;
    for (size_t i = 0; i < 4000; ++i)
    {
        StorageRecord record("failed-batch-" + std::to_string(i), "tenant-token",
            EventLatency_Normal, EventPersistence_Normal, /*timestamp*/ 1,
            std::vector<uint8_t>{ 'x' });
        (i < 2000 ? firstBatch : failedBatch).push_back(record);
    }

    EXPECT_CALL(*memory, GetSize())
        .WillOnce(Return(static_cast<size_t>(4000)))
        .WillOnce(Return(static_cast<size_t>(4000)));
    EXPECT_CALL(*memory, GetRecordCount(EventLatency_Unspecified))
        .WillOnce(Return(static_cast<size_t>(4000)));
    EXPECT_CALL(*memory, GetRecords(false, EventLatency_Unspecified, 2000))
        .WillOnce(Return(firstBatch))
        .WillOnce(Return(failedBatch));
    EXPECT_CALL(*memory, StoreRecord(_))
        .Times(2000)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*disk, StoreRecords(_))
        .WillOnce(Return(static_cast<size_t>(2000)))
        .WillOnce(Return(static_cast<size_t>(0)));
    EXPECT_CALL(observer, OnStorageRecordsSaved(2000));

    handler.Flush();
}

TEST(OfflineStorageHandlerFlushTests, CustomStorageUsesPerRecordWrites)
{
    ConfigurableLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    NoopTaskDispatcher dispatcher;
    StrictMock<MockIOfflineStorageObserver> observer;
    config[CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH] = true;
    config[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;

    auto disk = std::make_shared<StrictMock<MockIOfflineStorage>>();
    logManager.config.AddModule(CFG_MODULE_OFFLINE_STORAGE, disk);

    auto memory = std::make_shared<StrictMock<MockIOfflineStorage>>();
    auto provider = std::make_shared<MockOfflineStorageProvider>(memory, disk);
    OfflineStorageHandler handler(logManager, config, dispatcher, provider);
    EXPECT_CALL(*memory, Initialize(Ref(handler))).WillOnce(Return());
    EXPECT_CALL(*disk, Initialize(Ref(handler))).WillOnce(Return());
    handler.Initialize(observer);

    std::vector<StorageRecord> records;
    records.push_back(StorageRecord("custom-1", "tenant-token",
        EventLatency_Normal, EventPersistence_Normal, 1, std::vector<uint8_t>{ 'x' }));
    records.push_back(StorageRecord("custom-2", "tenant-token",
        EventLatency_Normal, EventPersistence_Normal, 1, std::vector<uint8_t>{ 'y' }));

    EXPECT_CALL(*memory, GetSize())
        .WillOnce(Return(records.size()))
        .WillOnce(Return(static_cast<size_t>(0)));
    EXPECT_CALL(*memory, GetRecords(false, EventLatency_Unspecified, 0))
        .WillOnce(Return(records));
    EXPECT_CALL(*disk, StoreRecords(_)).Times(0);
    EXPECT_CALL(*disk, StoreRecord(_)).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(observer, OnStorageRecordsSaved(records.size()));

    handler.Flush();
}

// Regression test: when valid records drained from the in-memory queue fail to
// be persisted by the disk backend during Flush() (a transient failure -- here
// an unopenable database), they must be returned to the queue rather than lost.
TEST(OfflineStorageHandlerFlushTests, FailedDiskStoreDuringFlushReturnsRecordsToMemory)
{
    NullLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    NoopTaskDispatcher dispatcher;
    NiceMock<MockIOfflineStorageObserver> observer;

    ON_CALL(config, GetOfflineStorageMaximumSizeBytes()).WillByDefault(Return(32 * 4096));
    ON_CALL(config, GetMaximumRetryCount()).WillByDefault(Return(5));

    // A path inside a non-existent directory cannot be opened by SQLite (it does
    // not create parent directories), so every disk StoreRecords() returns 0 --
    // a transient failure with otherwise-valid records.
    std::ostringstream dbPath;
    dbPath << GetTempDirectory() << "no_such_dir_" << PAL::getUtcSystemTimeMs()
        << "/FlushReserveTest.db";
    config[CFG_STR_CACHE_FILE_PATH] = dbPath.str();
    config[CFG_INT_RAM_QUEUE_SIZE] = 1024 * 1024;  // enable the in-memory queue

    OfflineStorageHandler handler(logManager, config, dispatcher);
    handler.Initialize(observer);

    const size_t kCount = 5;
    for (size_t i = 0; i < kCount; i++)
    {
        StorageRecord r("flush-id-" + std::to_string(i), "tenant-token",
            EventLatency_Normal, EventPersistence_Normal, /*timestamp*/ 1,
            std::vector<uint8_t>{ 'x' });
        handler.StoreRecord(r);
    }
    EXPECT_EQ(handler.GetRecordCount(), kCount);

    handler.Flush();

    // The disk could not persist the batch; with the fix the valid records are
    // returned to the in-memory queue rather than silently dropped.
    EXPECT_EQ(handler.GetRecordCount(), kCount);

    handler.Shutdown();
}

TEST(OfflineStorageHandlerFlushTests, EventLatencyOffIsDroppedWithoutReportingStoreFailure)
{
    NullLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    NoopTaskDispatcher dispatcher;
    NiceMock<MockIOfflineStorageObserver> observer;

    ON_CALL(config, GetOfflineStorageMaximumSizeBytes()).WillByDefault(Return(32 * 4096));

    config[CFG_STR_CACHE_FILE_PATH] = ":memory:";
    config[CFG_INT_RAM_QUEUE_SIZE] = 1024 * 1024;  // enable the in-memory queue

    OfflineStorageHandler handler(logManager, config, dispatcher);
    handler.Initialize(observer);

    StorageRecord record("latency-off", "tenant-token",
        EventLatency_Off, EventPersistence_Normal, /*timestamp*/ 1,
        std::vector<uint8_t>{ 'x' });

    EXPECT_TRUE(handler.StoreRecord(record));
    EXPECT_EQ(handler.GetRecordCount(), static_cast<size_t>(0));

    handler.Shutdown();
}

// Regression test: a permanently-invalid record (rejected by the disk backend's
// validation) must be dropped on Flush(), not returned to the queue -- otherwise
// one poison record would be re-drained and re-rejected on every flush, wedging
// the queue and blocking every valid record behind it.
TEST(OfflineStorageHandlerFlushTests, FlushDropsInvalidRecordsInsteadOfWedging)
{
    NullLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    NoopTaskDispatcher dispatcher;
    NiceMock<MockIOfflineStorageObserver> observer;

    ON_CALL(config, GetOfflineStorageMaximumSizeBytes()).WillByDefault(Return(32 * 4096));
    ON_CALL(config, GetMaximumRetryCount()).WillByDefault(Return(5));

    std::ostringstream dbPath;
    dbPath << GetTempDirectory() << "FlushDropInvalid-" << PAL::getUtcSystemTimeMs() << ".db";
    RemoveDbFiles(dbPath.str());
    config[CFG_STR_CACHE_FILE_PATH] = dbPath.str();
    config[CFG_INT_RAM_QUEUE_SIZE] = 1024 * 1024;  // enable the in-memory queue

    OfflineStorageHandler handler(logManager, config, dispatcher);
    handler.Initialize(observer);

    // A timestamp <= 0 is accepted by the in-memory queue but permanently rejected
    // by the SQLite disk store's validation, so it can never be persisted.
    const size_t kCount = 5;
    for (size_t i = 0; i < kCount; i++)
    {
        StorageRecord r("bad-id-" + std::to_string(i), "tenant-token",
            EventLatency_Normal, EventPersistence_Normal, /*timestamp*/ 0,
            std::vector<uint8_t>{ 'x' });
        handler.StoreRecord(r);
    }
    EXPECT_EQ(handler.GetRecordCount(), kCount);

    handler.Flush();

    // The invalid records are dropped, not returned to the queue, so the queue
    // drains and is not wedged.
    EXPECT_EQ(handler.GetRecordCount(), static_cast<size_t>(0));

    handler.Shutdown();
    RemoveDbFiles(dbPath.str());
}

TEST(OfflineStorageHandlerFlushTests, FlushOptOutDropsInvalidRecordsInsteadOfWedging)
{
    NullLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    NoopTaskDispatcher dispatcher;
    NiceMock<MockIOfflineStorageObserver> observer;

    ON_CALL(config, GetOfflineStorageMaximumSizeBytes()).WillByDefault(Return(32 * 4096));
    ON_CALL(config, GetMaximumRetryCount()).WillByDefault(Return(5));

    std::ostringstream dbPath;
    dbPath << GetTempDirectory() << "FlushOptOutDropInvalid-" << PAL::getUtcSystemTimeMs() << ".db";
    RemoveDbFiles(dbPath.str());
    config[CFG_STR_CACHE_FILE_PATH] = dbPath.str();
    config[CFG_INT_RAM_QUEUE_SIZE] = 1024 * 1024;
    config[CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH] = false;

    OfflineStorageHandler handler(logManager, config, dispatcher);
    handler.Initialize(observer);

    const size_t kCount = 3;
    for (size_t i = 0; i < kCount; i++)
    {
        StorageRecord r("bad-opt-out-id-" + std::to_string(i), "tenant-token",
            EventLatency_Normal, EventPersistence_Normal, /*timestamp*/ 0,
            std::vector<uint8_t>{ 'x' });
        handler.StoreRecord(r);
    }
    EXPECT_EQ(handler.GetRecordCount(), kCount);

    handler.Flush();

    EXPECT_EQ(handler.GetRecordCount(), static_cast<size_t>(0));

    handler.Shutdown();
    RemoveDbFiles(dbPath.str());
}
