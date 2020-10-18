//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "system/EventPropertiesStorage.hpp"
#include "common/Common.hpp"

using namespace testing;
using namespace MAT;

TEST(EventPropertiesStorageTests, DefaultConstructor)
{
    EventPropertiesStorage storage;
    EXPECT_THAT(storage.eventName, Eq(""));
    EXPECT_THAT(storage.eventType, Eq(""));
    EXPECT_THAT(storage.eventLatency, EventLatency_Normal);
    EXPECT_THAT(storage.eventPersistence, EventPersistence_Normal);
    EXPECT_THAT(storage.eventPopSample, 100);
    EXPECT_THAT(storage.eventPolicyBitflags, 0);
    EXPECT_THAT(storage.timestampInMillis, 0);
}

static EventPropertiesStorage ConstructNonDefaultStorage() noexcept
{
    EventPropertiesStorage storage;
    storage.eventName = "Fred";
    storage.eventType = "Rogers";
    storage.eventLatency = EventLatency::EventLatency_CostDeferred;
    storage.eventPersistence = EventPersistence::EventPersistence_Critical;
    storage.eventPopSample = 3.14;
    storage.eventPolicyBitflags = 42;
    storage.timestampInMillis = 123;
    return storage;
}

TEST(EventPropertiesStorageTests, CopyConstructor)
{
    EventPropertiesStorage storage = ConstructNonDefaultStorage();
    EventPropertiesStorage storageCopy{ storage };

    EXPECT_THAT(storageCopy.eventName, storage.eventName);
    EXPECT_THAT(storageCopy.eventType, storage.eventType);
    EXPECT_THAT(storageCopy.eventLatency, storage.eventLatency);
    EXPECT_THAT(storageCopy.eventPersistence, storage.eventPersistence);
    EXPECT_THAT(storageCopy.eventPopSample, storage.eventPopSample);
    EXPECT_THAT(storageCopy.eventPolicyBitflags, storage.eventPolicyBitflags);
    EXPECT_THAT(storageCopy.timestampInMillis, storage.timestampInMillis);
}

TEST(EventPropertiesStorageTests, MoveConstructor)
{
    EventPropertiesStorage storage = ConstructNonDefaultStorage();
    EventPropertiesStorage storageCopy{ std::move(storage) };

    EXPECT_THAT(storageCopy.eventName, "Fred");
    EXPECT_THAT(storageCopy.eventType, "Rogers");
    EXPECT_THAT(storageCopy.eventLatency, EventLatency::EventLatency_CostDeferred);
    EXPECT_THAT(storageCopy.eventPersistence, EventPersistence::EventPersistence_Critical);
    EXPECT_THAT(storageCopy.eventPopSample, 3.14);
    EXPECT_THAT(storageCopy.eventPolicyBitflags, 42);
    EXPECT_THAT(storageCopy.timestampInMillis, 123);
}

TEST(EventPropertiesStorageTests, AssignmentOperator)
{
    EventPropertiesStorage storage = ConstructNonDefaultStorage();
    EventPropertiesStorage secondStorage;
    secondStorage.eventName = "Mister";
    secondStorage = storage;

    EXPECT_THAT(secondStorage.eventName, storage.eventName);
    EXPECT_THAT(secondStorage.eventType, storage.eventType);
    EXPECT_THAT(secondStorage.eventLatency, storage.eventLatency);
    EXPECT_THAT(secondStorage.eventPersistence, storage.eventPersistence);
    EXPECT_THAT(secondStorage.eventPopSample, storage.eventPopSample);
    EXPECT_THAT(secondStorage.eventPolicyBitflags, storage.eventPolicyBitflags);
    EXPECT_THAT(secondStorage.timestampInMillis, storage.timestampInMillis);
}
