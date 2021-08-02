//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "common/Common.hpp"
#include "decorators/EventPropertiesDecorator.hpp"
#include "NullObjects.hpp"

using namespace testing;
using namespace MAT;
using namespace CsProtocol;

constexpr int16_t NormalEventLatency = 0x100;
constexpr int16_t RealtimeEventLatency = 0x200;
constexpr int16_t CostDeferredEventLatency = 0x300;
constexpr int8_t NormalEventPersistence = 0x1;
constexpr int8_t CriticalEventPersistence = 0x2;

class TestEventPropertiesDecorator : public EventPropertiesDecorator
{
public:
    TestEventPropertiesDecorator(ILogManager& logManager) noexcept :
        EventPropertiesDecorator(logManager) {}

    const std::string& GetRandomLocalId() const noexcept
    {
        return randomLocalId;
    }
};

static std::unique_ptr<Record> PopulateRecordForDropPii()
{
    auto record = std::unique_ptr<Record>(new Record{});
    record->extProtocol.push_back(CsProtocol::Protocol{});
    record->extDevice.push_back(CsProtocol::Device{});
    record->extUser.push_back(CsProtocol::User{});
    record->extSdk.push_back(CsProtocol::Sdk{});

    record->extProtocol[0].ticketKeys.push_back(std::vector<std::string>{"Ticket"});
    record->extDevice[0].localId = "localId";
    record->extDevice[0].authId = "authId";
    record->extDevice[0].authSecId = "authSecId";
    record->extDevice[0].id = "id";
    record->extUser[0].localId = "localId";
    record->extUser[0].authId = "authId";
    record->extUser[0].id = "id";
    record->extSdk[0].seq = 12345;
    record->extSdk[0].epoch = "epoch";
    record->extSdk[0].installId = "installId";
    record->cV = "cV";

    return record;
}

TEST(EventPropertiesDecoratorTests, Decorate_ZeroValueDoesNotAlterRecordTime)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    record.time = 1234;
    EventProperties props {"TestEvent"};
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_THAT(record.time, Eq(1234));
}

TEST(EventPropertiesDecoratorTests, Decorate_SetToEventPropertiesTimestamp)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    record.time = 1234;
    EventProperties props{"TestEvent"};
    props.SetTimestamp(1234567);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_THAT(record.time, Eq(1234567));
}

TEST(EventPropertiesDecoratorTests, Decorate_PopSample)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    record.popSample = 1;
    EventProperties props{"TestEvent"};
    props.SetPopsample(0.02);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_THAT(record.popSample, Eq(0.02));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventPersistence_Default)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props{"TestEvent"};
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_TRUE(record.flags & NormalEventPersistence);
}

TEST(EventPropertiesDecoratorTests, Decorate_EventPersistence_Critical)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props{"TestEvent"};
    props.SetPersistence(EventPersistence::EventPersistence_Critical);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_TRUE(record.flags & CriticalEventPersistence);
}

TEST(EventPropertiesDecoratorTests, Decorate_EventLatency_Normal)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props{"TestEvent"};
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_TRUE(record.flags & NormalEventLatency);
}

TEST(EventPropertiesDecoratorTests, Decorate_EventLatency_CostDeferred)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props{"TestEvent"};
    EventLatency latency = EventLatency::EventLatency_CostDeferred;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_TRUE(record.flags & CostDeferredEventLatency);
}

TEST(EventPropertiesDecoratorTests, Decorate_EventLatency_Realtime)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props{"TestEvent"};
    EventLatency latency = EventLatency::EventLatency_RealTime;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_TRUE(record.flags & RealtimeEventLatency);
}

TEST(EventPropertiesDecoratorTests, Decorate_EventLatency_Max_FallbackToRealtime)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props{"TestEvent"};
    EventLatency latency = EventLatency::EventLatency_Max;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_TRUE(record.flags & RealtimeEventLatency);
}

TEST(EventPropertiesDecoratorTests, Decorate_EventTag_MarkPii)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props{"TestEvent"};
    props.SetPolicyBitFlags(MICROSOFT_EVENTTAG_MARK_PII);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_TRUE(record.flags & RECORD_FLAGS_EVENTTAG_MARK_PII);
}

TEST(EventPropertiesDecoratorTests, Decorate_EventTag_DropPii)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    auto record = PopulateRecordForDropPii();
    EventProperties props{"TestEvent"};
    props.SetPolicyBitFlags(MICROSOFT_EVENTTAG_DROP_PII);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(*record, latency, props));
    EXPECT_TRUE(record->flags & RECORD_FLAGS_EVENTTAG_DROP_PII);
}

TEST(EventPropertiesDecoratorTests, DropPiiPartA_StripsValues)
{
    NullLogManager logManager;
    TestEventPropertiesDecorator decorator(logManager);
    auto record = PopulateRecordForDropPii();

    decorator.dropPiiPartA(*record);

    EXPECT_TRUE(record->extProtocol[0].ticketKeys.size() == 0);
    EXPECT_THAT(record->extDevice[0].localId, Eq(decorator.GetRandomLocalId()));
    EXPECT_THAT(record->extDevice[0].authId, Eq(""));
    EXPECT_THAT(record->extDevice[0].authSecId, Eq(""));
    EXPECT_THAT(record->extDevice[0].id, Eq(""));
    EXPECT_THAT(record->extUser[0].localId, Eq(""));
    EXPECT_THAT(record->extUser[0].authId, Eq(""));
    EXPECT_THAT(record->extUser[0].id, Eq(""));
    EXPECT_THAT(record->extSdk[0].seq, Eq(0));
    EXPECT_THAT(record->extSdk[0].epoch, Eq(""));
    EXPECT_THAT(record->extSdk[0].installId, Eq(""));
    EXPECT_THAT(record->cV, Eq(""));
}
