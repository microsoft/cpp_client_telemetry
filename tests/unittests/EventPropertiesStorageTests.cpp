#include "system/EventPropertiesStorage.hpp"
#include "common/Common.hpp"

using namespace testing;
using namespace ARIASDK_NS;

TEST(EventPropertiesStorageTests, DefaultConstructor)
{
	EventPropertiesStorage storage;
	EXPECT_THAT(storage.EventName, Eq(""));
	EXPECT_THAT(storage.EventType, Eq(""));
	EXPECT_THAT(storage.EventLatency, EventLatency_Normal);
	EXPECT_THAT(storage.EventPersistence, EventPersistence_Normal);
	EXPECT_THAT(storage.EventPopSample, 100);
	EXPECT_THAT(storage.EventPolicyBitflags, 0);
	EXPECT_THAT(storage.TimestampInMillis, 0);
}

static EventPropertiesStorage ConstructNonDefaultStorage() noexcept
{
	EventPropertiesStorage storage;
	storage.EventName = "Fred";
	storage.EventType = "Rogers";
	storage.EventLatency = EventLatency::EventLatency_CostDeferred;
	storage.EventPersistence = EventPersistence::EventPersistence_Critical;
	storage.EventPopSample = 3.14;
	storage.EventPolicyBitflags = 42;
	storage.TimestampInMillis = 123;
	return storage;
}

TEST(EventPropertiesStorageTests, CopyConstructor)
{
	EventPropertiesStorage storage = ConstructNonDefaultStorage();
	EventPropertiesStorage storageCopy { storage };

	EXPECT_THAT(storageCopy.EventName, storage.EventName);
	EXPECT_THAT(storageCopy.EventType, storage.EventType);
	EXPECT_THAT(storageCopy.EventLatency, storage.EventLatency);
	EXPECT_THAT(storageCopy.EventPersistence, storage.EventPersistence);
	EXPECT_THAT(storageCopy.EventPopSample, storage.EventPopSample);
	EXPECT_THAT(storageCopy.EventPolicyBitflags, storage.EventPolicyBitflags);
	EXPECT_THAT(storageCopy.TimestampInMillis, storage.TimestampInMillis);
}

TEST(EventPropertiesStorageTests, MoveConstructor)
{
	EventPropertiesStorage storage = ConstructNonDefaultStorage();
	EventPropertiesStorage storageCopy { std::move(storage) };

	EXPECT_THAT(storageCopy.EventName, "Fred");
	EXPECT_THAT(storageCopy.EventType, "Rogers");
	EXPECT_THAT(storageCopy.EventLatency, EventLatency::EventLatency_CostDeferred);
	EXPECT_THAT(storageCopy.EventPersistence, EventPersistence::EventPersistence_Critical);
	EXPECT_THAT(storageCopy.EventPopSample, 3.14);
	EXPECT_THAT(storageCopy.EventPolicyBitflags, 42);
	EXPECT_THAT(storageCopy.TimestampInMillis, 123);
}

TEST(EventPropertiesStorageTests, AssignmentOperator)
{
	EventPropertiesStorage storage = ConstructNonDefaultStorage();
	EventPropertiesStorage secondStorage;
	secondStorage.EventName = "Mister";
	secondStorage = storage;

	EXPECT_THAT(secondStorage.EventName, storage.EventName);
	EXPECT_THAT(secondStorage.EventType, storage.EventType);
	EXPECT_THAT(secondStorage.EventLatency, storage.EventLatency);
	EXPECT_THAT(secondStorage.EventPersistence, storage.EventPersistence);
	EXPECT_THAT(secondStorage.EventPopSample, storage.EventPopSample);
	EXPECT_THAT(secondStorage.EventPolicyBitflags, storage.EventPolicyBitflags);
	EXPECT_THAT(secondStorage.TimestampInMillis, storage.TimestampInMillis);
}