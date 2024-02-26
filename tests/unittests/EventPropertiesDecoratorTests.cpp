//
// Copyright (c) Microsoft Corporation. All rights reserved.
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
    EventProperties props {"TestEvent"};
    props.SetTimestamp(1234567);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_THAT(static_cast<unsigned long long>(record.time), Eq(1234567ULL*10000 + 0x89F7FF5F7B58000ULL));
}

TEST(EventPropertiesDecoratorTests, Decorate_PopSample)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    record.popSample = 1;
    EventProperties props {"TestEvent"};
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
    EventProperties props {"TestEvent"};
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_TRUE(record.flags & NormalEventPersistence);
}

TEST(EventPropertiesDecoratorTests, Decorate_EventPersistence_Critical)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
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
    EventProperties props {"TestEvent"};
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_TRUE(record.flags & NormalEventLatency);
}

TEST(EventPropertiesDecoratorTests, Decorate_EventLatency_CostDeferred)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    EventLatency latency = EventLatency::EventLatency_CostDeferred;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_TRUE(record.flags & CostDeferredEventLatency);
}

TEST(EventPropertiesDecoratorTests, Decorate_EventLatency_Realtime)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    EventLatency latency = EventLatency::EventLatency_RealTime;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_TRUE(record.flags & RealtimeEventLatency);
}

TEST(EventPropertiesDecoratorTests, Decorate_EventLatency_Max_FallbackToRealtime)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    EventLatency latency = EventLatency::EventLatency_Max;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_TRUE(record.flags & RealtimeEventLatency);
}

TEST(EventPropertiesDecoratorTests, Decorate_EventTag_MarkPii)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetPolicyBitFlags(MICROSOFT_EVENTTAG_MARK_PII);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));
    EXPECT_TRUE(record.flags & RECORD_FLAGS_EVENTTAG_MARK_PII);
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_StringProperty)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("StringProp", "StringValue");
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("StringProp");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.type, Eq(ValueKind::ValueString));
    EXPECT_THAT(dataField->second.stringValue, Eq("StringValue"));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_Int64Property)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.type, Eq(ValueKind::ValueInt64));
    EXPECT_THAT(dataField->second.longValue, Eq(12345));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_DoubleProperty)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("DoubleProp", 12345.0);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("DoubleProp");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.type, Eq(ValueKind::ValueDouble));
    EXPECT_THAT(dataField->second.doubleValue, Eq(12345.0));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_BooleanProperty)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("BooleanProp", true);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("BooleanProp");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.type, Eq(ValueKind::ValueBool));
    EXPECT_THAT(dataField->second.longValue, Eq(1));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_TimeTicksProperty)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("TimeTicksProp", time_ticks_t {12345} );
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("TimeTicksProp");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.type, Eq(ValueKind::ValueDateTime));
    EXPECT_THAT(dataField->second.longValue, Eq(12345));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_GuidProperty)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    GUID_t guid {"01234567-89ab-cdef-0123-456789abcdef"};
    props.SetProperty("GuidProp", guid);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("GuidProp");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.type, Eq(ValueKind::ValueGuid));
    EXPECT_THAT(dataField->second.guidValue[0], SizeIs(16));

    uint8_t guidBytes[16] = {0};
    guid.to_bytes(guidBytes);
    auto guidByteVector = std::vector<uint8_t>(guidBytes, guidBytes + sizeof(guidBytes) / sizeof(guidBytes[0]));
    EXPECT_THAT(dataField->second.guidValue[0], Eq(guidByteVector));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_PiiKind_CustomerContentKind_GenericData)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345, PiiKind::CustomerContentKind_GenericData);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.attributes[0].customerContent[0].Kind, Eq(CustomerContentKind::GenericContent));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_PiiKind_DistinguishedName)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345, PiiKind::PiiKind_DistinguishedName);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.attributes[0].pii[0].Kind, Eq(PIIKind::DistinguishedName));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_PiiKind_Fqdn)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345, PiiKind::PiiKind_Fqdn);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.attributes[0].pii[0].Kind, Eq(PIIKind::Fqdn));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_PiiKind_GenericData)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345, PiiKind::PiiKind_GenericData);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.attributes[0].pii[0].Kind, Eq(PIIKind::GenericData));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_PiiKind_Identity)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345, PiiKind::PiiKind_Identity);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.attributes[0].pii[0].Kind, Eq(PIIKind::Identity));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_PiiKind_IPv4Address)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345, PiiKind::PiiKind_IPv4Address);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.attributes[0].pii[0].Kind, Eq(PIIKind::IPV4Address));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_PiiKind_IPv4AddressLegacy)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345, PiiKind::PiiKind_IPv4AddressLegacy);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.attributes[0].pii[0].Kind, Eq(PIIKind::IPV4AddressLegacy));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_PiiKind_IPv6Address)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345, PiiKind::PiiKind_IPv6ScrubLastHextets);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.attributes[0].pii[0].Kind, Eq(PIIKind::IPv6ScrubLastHextets));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_PiiKind_DropValue)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345, PiiKind::PiiKind_DropValue);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.attributes[0].pii[0].Kind, Eq(PIIKind::PiiKind_DropValue));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_PiiKind_IPv6Address)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345, PiiKind::PiiKind_IPv6Address);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.attributes[0].pii[0].Kind, Eq(PIIKind::IPv6Address));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_PiiKind_MailSubject)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345, PiiKind::PiiKind_MailSubject);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.attributes[0].pii[0].Kind, Eq(PIIKind::MailSubject));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_PiiKind_PhoneNumber)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345, PiiKind::PiiKind_PhoneNumber);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.attributes[0].pii[0].Kind, Eq(PIIKind::PhoneNumber));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_PiiKind_QueryString)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345, PiiKind::PiiKind_QueryString);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.attributes[0].pii[0].Kind, Eq(PIIKind::QueryString));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_PiiKind_SipAddress)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345, PiiKind::PiiKind_SipAddress);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.attributes[0].pii[0].Kind, Eq(PIIKind::SipAddress));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_PiiKind_SmtpAddress)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345, PiiKind::PiiKind_SmtpAddress);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.attributes[0].pii[0].Kind, Eq(PIIKind::SmtpAddress));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventProperties_PiiKind_Uri)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    Record record;
    EventProperties props {"TestEvent"};
    props.SetProperty("Int64Prop", 12345, PiiKind::PiiKind_Uri);
    EventLatency latency = EventLatency::EventLatency_Normal;

    EXPECT_TRUE(decorator.decorate(record, latency, props));

    auto dataField = record.data[0].properties.find("Int64Prop");
    EXPECT_THAT(dataField, Ne(record.data[0].properties.end()));
    EXPECT_THAT(dataField->second.attributes[0].pii[0].Kind, Eq(PIIKind::Uri));
}

TEST(EventPropertiesDecoratorTests, Decorate_EventTag_DropPii)
{
    NullLogManager logManager;
    EventPropertiesDecorator decorator(logManager);
    auto record = PopulateRecordForDropPii();
    EventProperties props {"TestEvent"};
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

    EXPECT_THAT(record->extProtocol[0].ticketKeys, SizeIs(0));
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
