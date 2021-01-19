//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
//
// Created by maharrim on 5/18/2020.
//
#ifdef ANDROID
#include <android/log.h>
#endif
#include "common/Common.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include "common/MockIOfflineStorageObserver.hpp"
#include "offline/MemoryStorage.hpp"
#ifdef ANDROID
#include "offline/OfflineStorage_Room.hpp"
#endif
#include "offline/OfflineStorage_SQLite.hpp"
#include "NullObjects.hpp"
#include <functional>
#include <string>
#include <fstream>
#ifdef ANDROID
#include <http/HttpClient_Android.hpp>
#endif

namespace MAE = ::Microsoft::Applications::Events;

using namespace testing;
enum class StorageImplementation {
    Room,
    SQLite,
    Memory
};

std::ostream & operator<<(std::ostream &o, StorageImplementation i) {
    switch (i) {
        case StorageImplementation::Room:
            return o << "Room";
        case StorageImplementation::SQLite:
            return o << "SQLite";
        case StorageImplementation ::Memory:
            return o << "Memory";
        default:
            return o << static_cast<int>(i);
    }
}

class OfflineStorageTestsRoom : public TestWithParam<StorageImplementation> {

public:
    StrictMock<MockIRuntimeConfig>                      configMock;
    StrictMock<MockIOfflineStorageObserver>             observerMock;
    ILogManager * const                                 logManager;
    std::unique_ptr<MAE::IOfflineStorage>               offlineStorage;
    NullLogManager                                      nullLogManager;
    StorageImplementation                               implementation;

    OfflineStorageTestsRoom() : logManager(&nullLogManager)
    {
        EXPECT_CALL(configMock, GetOfflineStorageMaximumSizeBytes()).WillRepeatedly(
                Return(32 * 4096));
        EXPECT_CALL(configMock, GetMaximumRetryCount()).WillRepeatedly(
                Return(5));
        std::ostringstream name;
        implementation = GetParam();
        switch (implementation) {
#ifdef ANDROID
            case StorageImplementation::Room:
                configMock[CFG_STR_CACHE_FILE_PATH] = "OfflineStorageTestsRoom.db";
                offlineStorage = std::make_unique<MAE::OfflineStorage_Room>(nullLogManager, configMock);
                EXPECT_CALL(observerMock, OnStorageOpened("Room/Init"))
                        .RetiresOnSaturation();
                break;
#endif
            case StorageImplementation::SQLite:
                name << MAE::GetTempDirectory() << "OfflineStorageTestsSQLite.db";
                configMock[CFG_STR_CACHE_FILE_PATH] = name.str();
                offlineStorage = std::make_unique<MAE::OfflineStorage_SQLite>(nullLogManager, configMock);
                EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Default"))
                        .RetiresOnSaturation();
                break;
            case StorageImplementation::Memory:
                offlineStorage = std::make_unique<MAE::MemoryStorage>(nullLogManager, configMock);
                break;
        }

        offlineStorage->Initialize(observerMock);
    }

    ~OfflineStorageTestsRoom()
    {
        offlineStorage->Shutdown();
    }

    void DeleteAllRecords() {
        auto records = offlineStorage->GetRecords(true, EventLatency_Unspecified, 0);
        if (records.empty()) {
            return;
        }
        std::vector<std::string> ids;
        ids.reserve(records.size());
        for (auto &record : records) {
            ids.emplace_back(std::move(record.id));
        }
        HttpHeaders h;
        bool fromMemory = false;
        offlineStorage->DeleteRecords(ids, h, fromMemory);
        EXPECT_EQ(0, offlineStorage->GetRecordCount());
    }

    void SetUp() override {
        DeleteAllRecords();
    }

    void TearDown() override {
        DeleteAllRecords();
    }

    void PopulateRecords() {
        auto now = PAL::getUtcSystemTimeMs();
        for (EventLatency latency : {EventLatency_Normal, EventLatency_RealTime}) {
            StorageRecordVector records;

            for (size_t i = 0; i < 10; ++i) {
                std::ostringstream id_stream;
                id_stream << "Fred-" << i << "-" << latency;
                std::string id = id_stream.str();
                records.emplace_back(
                        id,
                        id,
                        latency,
                        EventPersistence_Normal,
                        now,
                        StorageBlob{1, 2, 3});
            }
            offlineStorage->StoreRecords(records);
        }
        EXPECT_EQ(20, offlineStorage->GetRecordCount(EventLatency_Unspecified));
    }

    void VerifyBlob(StorageBlob const & blob)
    {
        EXPECT_EQ(3, blob.size());
        for (size_t i = 0; i < blob.size(); ++i) {
            EXPECT_EQ(i+1, blob[i]);
        }
    }
};

TEST_P(OfflineStorageTestsRoom, TestBadFile)
{
    auto path = GetTempDirectory();

    switch (implementation) {
        case StorageImplementation::Memory:
            return;
        case StorageImplementation::Room:
            path = path.substr(0, path.length() - 6) + "databases/BadDatabase.db";
            break;
        case StorageImplementation::SQLite:
            path = path + "BadDatabase.db";
            break;
    }
    auto badFile = std::ofstream(path);
    badFile << "this is a BAD database" << std::endl;
    badFile.close();

    std::unique_ptr<MAE::IOfflineStorage> badStorage;
    switch (implementation) {
#ifdef ANDROID
        case StorageImplementation::Room:
            configMock[CFG_STR_CACHE_FILE_PATH] = "BadDatabase.db";
            badStorage = std::make_unique<MAE::OfflineStorage_Room>(nullLogManager, configMock);
            EXPECT_CALL(observerMock, OnStorageOpened("Room/Init"))
                .RetiresOnSaturation();
            break;
#endif
        case StorageImplementation::SQLite:
            configMock[CFG_STR_CACHE_FILE_PATH] = path.c_str();
            badStorage = std::make_unique<MAE::OfflineStorage_SQLite>(nullLogManager, configMock);
            EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Clean"))
                .RetiresOnSaturation();
            EXPECT_CALL(observerMock, OnStorageFailed("1")).RetiresOnSaturation();
            break;
        default:
            return;
    }
    badStorage->Initialize(observerMock);
    std::atomic<size_t> found(0);
    EXPECT_FALSE(badStorage->GetAndReserveRecords( [&found](StorageRecord && record)->bool {
      found += 1;
      return true;
    }, 5));
    badStorage->Shutdown();
}

TEST_P(OfflineStorageTestsRoom, TestStoreRecords)
{
    auto now = PAL::getUtcSystemTimeMs();
    StorageRecordVector records;
    for (size_t i = 0; i < 10; ++i) {
        std::ostringstream id_stream;
        id_stream << "Fred-" << i;
        std::string id = id_stream.str();
        records.emplace_back(
                id,
                id,
                EventLatency_Normal,
                EventPersistence_Normal,
                now,
                StorageBlob {1, 2, 3});
    }
    offlineStorage->StoreRecords(records);
    EXPECT_EQ(10, offlineStorage->GetRecordCount(EventLatency_Normal));
    EXPECT_EQ(10, offlineStorage->GetRecordCount(EventLatency_Unspecified));
    auto found = offlineStorage->GetRecords(true, EventLatency_Unspecified, 0);
    EXPECT_EQ(10, found.size());
    for (auto const & record : found) {
        VerifyBlob(record.blob);
        EXPECT_EQ(EventLatency_Normal, record.latency);
        EXPECT_EQ(EventPersistence_Normal, record.persistence);
        EXPECT_EQ(now, record.timestamp);
        EXPECT_EQ(0, record.reservedUntil);
    }
}

std::ostream & operator<<(std::ostream &os, EventLatency const &latency)
{
    switch (latency) {
        case EventLatency_Normal:
            os << "Normal";
            break;
        case EventLatency_RealTime:
            os << "Real-Time";
            break;
        default:
            os << "Other-" << static_cast<int>(latency);
            break;
    }
    return os;
}

TEST_P(OfflineStorageTestsRoom, TestGetAndReserveManyAcceptSome) {
    StorageRecordVector records;
    auto now = PAL::getUtcSystemTimeMs();
    for (size_t i = 0; i < 500000; ++i) {
        std::ostringstream id_stream;
        id_stream << "Fred-" << i;
        std::string id = id_stream.str();
        records.emplace_back(
                id,
                id,
                EventLatency_Normal,
                EventPersistence_Normal,
                now,
                StorageBlob{1, 2, 3});
        if (records.size() >= 256) {
            offlineStorage->StoreRecords(records);
            records.clear();
        }
    }
    if (!records.empty()) {
        offlineStorage->StoreRecords(records);
        records.clear();
    }
    EXPECT_TRUE(offlineStorage->GetAndReserveRecords(
            [&records](StorageRecord &&record) -> bool {
                if (records.size() >= 256) {
                    return false;
                }
                records.push_back(record);
                return true;
            },
            5
    ));
}

TEST_P(OfflineStorageTestsRoom, TestGetAndReserveAcceptAll)
{
    PopulateRecords();
    StorageRecordVector found;
    EXPECT_TRUE(offlineStorage->GetAndReserveRecords( [&found](StorageRecord && record)->bool {
        found.push_back(record);
        return true;
    }, 5));
    EXPECT_EQ(20, found.size());
    ASSERT_EQ(20, offlineStorage->LastReadRecordCount());
    ASSERT_EQ(
            implementation == StorageImplementation::Memory,
            offlineStorage->IsLastReadFromMemory());
    for (size_t i = 0; i < 10; ++i) {
        EXPECT_EQ(EventLatency_RealTime, found[i].latency);
    }
    for (size_t i = 10; i < 20; ++i) {
        EXPECT_EQ(EventLatency_Normal, found[i].latency);
    }
    for (auto const & record : found) {
        VerifyBlob(record.blob);
    }
}

TEST_P(OfflineStorageTestsRoom, TestAcceptFunctor) {
    PopulateRecords();
    StorageRecordVector found;
    size_t calls = 0u;
    EXPECT_TRUE(offlineStorage->GetAndReserveRecords(
            [&found, &calls](StorageRecord && record)->bool {
                ++calls;
                if (record.latency == EventLatency_RealTime) {
                    found.push_back(record);
                    return true;
                }
                return false;
            }
            , 5));
    ASSERT_EQ(10, found.size());
    ASSERT_EQ(11, calls);

}

TEST_P(OfflineStorageTestsRoom, TestSettings) {
    if (implementation == StorageImplementation::Memory) {
        return;
    }
    for (size_t i = 0; i < 10; ++i) {
        std::ostringstream nameStream;
        nameStream << "Fred" << i;
        offlineStorage->StoreSetting(nameStream.str(), nameStream.str());
        EXPECT_EQ(nameStream.str(), offlineStorage->GetSetting(nameStream.str()));
    }
    offlineStorage->StoreSetting("Fred3", "another value");
    for (size_t i = 0; i < 10; ++i) {
        std::ostringstream nameStream;
        nameStream << "Fred" << i;
        if (i == 3) {
            EXPECT_EQ(std::string("another value"), offlineStorage->GetSetting(nameStream.str()));
        }
        else {
            EXPECT_EQ(nameStream.str(), offlineStorage->GetSetting(nameStream.str()));
        }
    }
    EXPECT_EQ("", offlineStorage->GetSetting(std::string("something")));
    for (size_t i = 0; i < 10; ++i) {
        std::ostringstream nameStream;
        nameStream << "Fred" << i;
        offlineStorage->StoreSetting(nameStream.str(), "");
    }
    for (size_t i = 0; i < 10; ++i) {
        std::ostringstream nameStream;
        nameStream << "Fred" << i;
        EXPECT_EQ("", offlineStorage->GetSetting(nameStream.str()));
    }
}

TEST_P(OfflineStorageTestsRoom, TestGetRecords) {
    if (implementation == StorageImplementation::Memory) {
        // For MemoryStorage, GetRecords() returns very
        // different results.
        return;
    }
    auto now = PAL::getUtcSystemTimeMs();
    StorageRecordVector records;
    StorageRecord x;
    for (size_t i = 0; i < 20; ++i) {
        std::ostringstream s;
        s << "Fred-" << i;
        records.emplace_back(
                s.str(),
                s.str(),
                i < 10 ? EventLatency_Normal : EventLatency_RealTime,
                EventPersistence_Normal,
                now,
                StorageBlob {1, 2, 3}
                );
    }
    offlineStorage->StoreRecords(records);
    auto found = offlineStorage->GetRecords(false, EventLatency_Normal, 0);
    ASSERT_EQ(10, found.size());
    for (StorageRecord record : found) {
        ASSERT_EQ(EventLatency_Normal, record.latency);
    }
    auto shutdown_found = offlineStorage->GetRecords(true, EventLatency_Normal, 0);
    ASSERT_EQ(20, shutdown_found.size());
    for (size_t i = 0; i < 10; ++i) {
        ASSERT_EQ(EventLatency_RealTime, shutdown_found[i].latency);
    }
    for (size_t i = 10; i < 20; ++i) {
        ASSERT_EQ(EventLatency_Normal, shutdown_found[i].latency);
    }
}

TEST_P(OfflineStorageTestsRoom, TestManyExpiredRecords) {
    size_t count = 5000;
    auto now = PAL::getUtcSystemTimeMs();
    auto retries = configMock.GetMaximumRetryCount() + 1;
    std::vector<StorageRecord> manyRecords;
    manyRecords.reserve(count);
    EXPECT_EQ(0, offlineStorage->GetRecordCount(EventLatency_Normal));
    for (size_t i = 0; i < count; ++i) {
        std::string thing = std::to_string(i);
        manyRecords.emplace_back(
            thing, // id std::string const& tenantToken, EventLatency latency, EventPersistence persistence)
            thing, // token
            EventLatency_Normal,
            EventPersistence_Normal,
            now,
            StorageBlob {1, 2, 3}
        );
    }
    offlineStorage->StoreRecords(manyRecords);
    std::vector<StorageRecordId> manyIds;
    manyIds.reserve(count);
    if (implementation != StorageImplementation::Memory) {
        EXPECT_CALL(observerMock, OnStorageRecordsDropped(SizeIs(count))).
                WillOnce(Return());
    }
    for (size_t retry = 0; retry < retries; ++retry) {
        manyRecords.clear();
        manyIds.clear();
        offlineStorage->GetAndReserveRecords(
                [&manyRecords](StorageRecord &&record) -> bool {
                    manyRecords.emplace_back(record);
                    return true;
                },
                5000u);
        EXPECT_EQ(count, manyRecords.size());
        EXPECT_THAT(manyRecords, Each(Field(&StorageRecord::retryCount, Eq(retry))));
        for (auto const & record : manyRecords) {
            manyIds.emplace_back(std::move(record.id));
        }
        bool fromMemory;
        offlineStorage->ReleaseRecords(manyIds, true, HttpHeaders(), fromMemory);
    }
    size_t remainingRecords = 0;
    if (implementation == StorageImplementation::Memory) {
        remainingRecords = count;
    }
    EXPECT_EQ(remainingRecords, offlineStorage->GetRecordCount(EventLatency_Normal));
}

TEST_P(OfflineStorageTestsRoom, LastReadRecordCount) {
    size_t count = 5000;
    size_t consume = 315;
    std::hash<size_t> id_hash;
    StorageRecordVector records;
    records.reserve(count);
    auto now = PAL::getUtcSystemTimeMs();
    for (size_t i = 0; i < count; i++) {
        auto id = id_hash(i);
        auto id_string = std::to_string(id);
        records.emplace_back(
                id_string,
                id_string,
                EventLatency_Normal,
                EventPersistence_Normal,
                now,
                StorageBlob {3, 1, 4, 1, 5, 9}
                );
    }
    offlineStorage->StoreRecords(records);
    records.clear();
    offlineStorage->GetAndReserveRecords(
            [&records, consume](StorageRecord && record)->bool
            {
                if (records.size() >= consume) {
                    return false;
                }
                records.emplace_back(record);
                return true;
            },
            5000
            );
    EXPECT_EQ(consume, offlineStorage->LastReadRecordCount());
}

TEST_P(OfflineStorageTestsRoom, ReleaseActuallyReleases) {
    auto now = PAL::getUtcSystemTimeMs();
    StorageRecord r(
            "Fred",
            "George",
            EventLatency_Normal,
            EventPersistence_Normal,
            now,
            StorageBlob {1, 2, 3}
            );
    offlineStorage->StoreRecord(r);
    offlineStorage->GetAndReserveRecords(
            [](StorageRecord && record)->bool
            {
                return false;
            },
            5000
            );
    EXPECT_EQ(0, offlineStorage->LastReadRecordCount());
    StorageRecordVector records;
    offlineStorage->GetAndReserveRecords(
            [&records] (StorageRecord && record)->bool
            {
                records.emplace_back(std::move(record));
                return true;
            }, 5000
            );
    EXPECT_EQ(1, offlineStorage->LastReadRecordCount());
    EXPECT_EQ(1, records.size());
    offlineStorage->GetAndReserveRecords(
            [] (StorageRecord && record)->bool
            {
                ADD_FAILURE();
                return false;
            },
            5000
            );
}

TEST_P(OfflineStorageTestsRoom, DeleteByToken)
{
    StorageRecordVector records;
    auto now = PAL::getUtcSystemTimeMs();
    for (size_t i = 0; i < 1000; ++i) {
        auto id = std::to_string(i);
        auto tenantToken = std::to_string(i % 5);
        records.emplace_back(
                id,
                tenantToken,
                EventLatency_Normal,
                EventPersistence_Normal,
                now,
                StorageBlob {1, 2, static_cast<unsigned char>(i), 4, 5}
        );
    }
    offlineStorage->StoreRecords(records);
    EXPECT_EQ(1000, offlineStorage->GetRecordCount());
    offlineStorage->DeleteRecords({{ "tenant_token", "0"}});
    EXPECT_EQ(800, offlineStorage->GetRecordCount());
}

TEST_P(OfflineStorageTestsRoom, ResizeDB)
{
    if (implementation == StorageImplementation::Memory) {
        return;
    }

    auto now = PAL::getUtcSystemTimeMs();

    StorageRecord record(
            "",
            "TenantFred",
            EventLatency_Normal,
            EventPersistence_Normal,
            now,
            StorageBlob {1, 2, 3, 4}
            );
    size_t index = 1;
    while (offlineStorage->GetSize() <= configMock.GetOfflineStorageMaximumSizeBytes()) {
        record.id = std::to_string(index);
        offlineStorage->StoreRecord(record);
        index += 1;
    }
    auto preCount = offlineStorage->GetRecordCount();
    offlineStorage->ResizeDb();
    auto postCount = offlineStorage->GetRecordCount();
    EXPECT_GT(preCount, postCount);
}

TEST_P(OfflineStorageTestsRoom, StoreManyRecords)
{
    constexpr size_t targetSize = 2 * 1024 * 1024;
    constexpr size_t blobSize = 512;
    constexpr size_t blockSize = 1024;

    std::random_device rd;   // non-deterministic generator
    std::mt19937_64 gen(rd());  // to seed mersenne twister.
    std::uniform_int_distribution<> randomByte(0,255);
    std::uniform_int_distribution<uint64_t> randomWord(0, UINT64_MAX);
    auto now = PAL::getUtcSystemTimeMs();

    StorageBlob masterBlob;
    masterBlob.reserve(blobSize);
    while (masterBlob.size() < blobSize) {
        masterBlob.push_back(randomByte(gen));
    }
    size_t blocks = 0;
    StorageRecordVector records;
    records.reserve(blockSize);
    while (records.size() < blockSize) {
        records.emplace_back(
                "",
                "Fred-Doom-Token23",
                EventLatency_Normal,
                EventPersistence_Normal,
                now,
                StorageBlob(masterBlob)
        );
    }

    while (offlineStorage->GetSize() < targetSize) {
        for (auto & record : records) {
            record.id = std::to_string(randomWord(gen));
        }
        offlineStorage->StoreRecords(records);
        ++blocks;
    }
    EXPECT_EQ(blocks * blockSize, offlineStorage->GetRecordCount());
}

#ifdef ANDROID
auto values = Values(StorageImplementation::Room, StorageImplementation::SQLite, StorageImplementation::Memory);
#else
auto values = Values(StorageImplementation::SQLite, StorageImplementation::Memory);
#endif

INSTANTIATE_TEST_CASE_P(Storage,
        OfflineStorageTestsRoom,
        values,
        [](const testing::TestParamInfo<OfflineStorageTestsRoom::ParamType>& info)->std::string {
    std::ostringstream s;
    s << info.param;
    return s.str();
});

