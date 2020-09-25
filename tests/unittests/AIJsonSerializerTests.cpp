// Copyright (c) Microsoft. All rights reserved .

#if defined __has_include
#if __has_include("modules/azmon/AIJsonSerializer.hpp")
#include "modules/azmon/AIJsonSerializer.hpp"
#else
/* Compiling without Azure Monitor */
#undef HAVE_MAT_AI
#endif
#endif

#ifdef HAVE_MAT_AI

#include "common/Common.hpp"
#include "json.hpp"

#define TEST_TOKEN "12345678-1234-1234-1234-123456789abc"

std::unique_ptr<::CsProtocol::Record> createTestRecord(
    std::string name,
    int seq,
    ::CsProtocol::App app,
    ::CsProtocol::Device device,
    ::CsProtocol::Protocol proto,
    ::CsProtocol::Os os,
    ::CsProtocol::User user,
    ::CsProtocol::Data data
)
{
    std::unique_ptr<::CsProtocol::Record> record = std::make_unique<::CsProtocol::Record>();
    record->name = name;
    record->baseType = EVENTRECORD_TYPE_CUSTOM_EVENT;
    ::CsProtocol::Sdk sdk;
    sdk.seq = seq;
    record->extSdk.push_back(sdk);
    record->extApp.push_back(app);
    record->extDevice.push_back(device);
    record->extProtocol.push_back(proto);
    record->extOs.push_back(os);
    record->extUser.push_back(user);
    record->data.push_back(data);
    return record;
}

std::unique_ptr<::CsProtocol::Record> createInvalidTestRecord()
{
    std::string smallString(256 + 4, 's');
    std::string mediumString(1024 + 4, 'm');
    std::string longString(8192 + 4, 'l');

    ::CsProtocol::App app;
    app.ver = mediumString;
    app.locale = smallString;

    ::CsProtocol::Device device;
    device.deviceClass = smallString;
    device.localId = mediumString;

    ::CsProtocol::Protocol proto;
    proto.devMake = smallString;
    proto.devModel = smallString;

    ::CsProtocol::Os os;
    os.name = smallString;
    os.ver = smallString;

    ::CsProtocol::User user;
    user.localId = smallString;

    ::CsProtocol::Data data;
    ::CsProtocol::Value prop;
    prop.stringValue = longString;
    data.properties[smallString] = prop;

    return createTestRecord(smallString, 1, app, device, proto, os, user, data);
}

TEST(AIJsonSerializerTests, correctJsonStructure)
{
    std::unique_ptr<AIJsonSerializer> aiSerializer = std::make_unique<AIJsonSerializer>();

    ::CsProtocol::App app;
    app.ver = "appVer";
    app.locale = "appLocale";

    ::CsProtocol::Device device;
    device.localId = "deviceId";
    device.deviceClass = "deviceClass";

    ::CsProtocol::Protocol proto;
    proto.devMake = "protoDevMake";
    proto.devModel = "protoDevModel";

    ::CsProtocol::Os os;
    os.name = "osName";
    os.ver = "osVer";

    ::CsProtocol::User user;
    user.localId = "userId";

    ::CsProtocol::Data data;
    ::CsProtocol::Value prop1;
    prop1.stringValue = "prop_value1";
    data.properties["prop_key1"] = prop1;

    std::unique_ptr<::CsProtocol::Record> record1 = createTestRecord(
        "event1", 1, app, device, proto, os, user, data
    );
    IncomingEventContext context1(PAL::generateUuidString(), TEST_TOKEN, EventLatency_Unspecified, EventPersistence_Normal, record1.get());

    aiSerializer->serialize(&context1);
    ::nlohmann::json result1 = nlohmann::json::parse(context1.record.blob.begin(), context1.record.blob.end());

    auto sessionId = result1["tags"]["ai.session.id"].get<std::string>();
    auto isFirst = result1["tags"]["ai.session.isFirst"].get<bool>();

    EXPECT_TRUE(isFirst);
    EXPECT_TRUE(sessionId.size() >= 36 && sessionId.size() <= 64);
    EXPECT_EQ(1, result1["seq"].get<int>());
    EXPECT_EQ("appVer", result1["tags"]["ai.application.ver"].get<std::string>());
    EXPECT_EQ("deviceId", result1["tags"]["ai.device.id"].get<std::string>());
    EXPECT_EQ("appLocale", result1["tags"]["ai.device.locale"].get<std::string>());
    EXPECT_EQ("protoDevModel", result1["tags"]["ai.device.model"].get<std::string>());
    EXPECT_EQ("protoDevMake", result1["tags"]["ai.device.oemName"].get<std::string>());
    EXPECT_EQ("osName", result1["tags"]["ai.device.os"].get<std::string>());
    EXPECT_EQ("osVer", result1["tags"]["ai.device.osVersion"].get<std::string>());
    EXPECT_EQ("deviceClass", result1["tags"]["ai.device.type"].get<std::string>());
    EXPECT_EQ("userId", result1["tags"]["ai.user.id"].get<std::string>());
    EXPECT_EQ("event1", result1["data"]["baseData"]["name"].get<std::string>());
    EXPECT_EQ("prop_value1", result1["data"]["baseData"]["properties"]["prop_key1"].get<std::string>());

    ::CsProtocol::Data data2;
    ::CsProtocol::Value prop2;
    prop2.stringValue = "prop_value2";
    data2.properties["prop_key2"] = prop2;

    std::unique_ptr<::CsProtocol::Record> record2 = createTestRecord(
        "event2", 2, app, device, proto, os, user, data2
    );
    IncomingEventContext context2(PAL::generateUuidString(), TEST_TOKEN, EventLatency_Unspecified, EventPersistence_Normal, record2.get());

    aiSerializer->serialize(&context2);
    ::nlohmann::json result2 = nlohmann::json::parse(context2.record.blob.begin(), context2.record.blob.end());

    auto isFirst2 = result2["tags"]["ai.session.isFirst"].get<bool>();
    EXPECT_FALSE(isFirst2);
    EXPECT_EQ(sessionId, result2["tags"]["ai.session.id"].get<std::string>());
    EXPECT_EQ(2, result2["seq"].get<int>());
    EXPECT_EQ("appVer", result2["tags"]["ai.application.ver"].get<std::string>());
    EXPECT_EQ("event2", result2["data"]["baseData"]["name"].get<std::string>());
    EXPECT_EQ("prop_value2", result2["data"]["baseData"]["properties"]["prop_key2"].get<std::string>());
}

TEST(AIJsonSerializerTests, valuesSanitized)
{
    std::unique_ptr<::CsProtocol::Record> record = createInvalidTestRecord();

    IncomingEventContext context(PAL::generateUuidString(), TEST_TOKEN, EventLatency_Unspecified, EventPersistence_Normal, record.get());
    std::unique_ptr<AIJsonSerializer> aiSerializer = std::make_unique<AIJsonSerializer>();

    aiSerializer->serialize(&context);
    auto blob = context.record.blob;
    ::nlohmann::json result = nlohmann::json::parse(blob.begin(), blob.end());
    
    std::string expectSmall64(64, 's');
    std::string expectSmall150(150, 's');
    std::string expectSmall256(256, 's');
    std::string expectSmall128(128, 's');
    std::string expectMedium(1024, 'm');
    std::string expectLarge(8192, 'l');

    EXPECT_EQ(expectMedium,   result["tags"]["ai.application.ver"].get<std::string>());
    EXPECT_EQ(expectMedium,   result["tags"]["ai.device.id"].get<std::string>());
    EXPECT_EQ(expectSmall64,  result["tags"]["ai.device.locale"].get<std::string>());
    EXPECT_EQ(expectSmall256, result["tags"]["ai.device.model"].get<std::string>());
    EXPECT_EQ(expectSmall256, result["tags"]["ai.device.oemName"].get<std::string>());
    EXPECT_EQ(expectSmall256, result["tags"]["ai.device.os"].get<std::string>());
    EXPECT_EQ(expectSmall256, result["tags"]["ai.device.osVersion"].get<std::string>());
    EXPECT_EQ(expectSmall64,  result["tags"]["ai.device.type"].get<std::string>());
    EXPECT_EQ(expectSmall128, result["tags"]["ai.user.id"].get<std::string>());
    EXPECT_EQ(expectSmall150, result["data"]["baseData"]["name"].get<std::string>());
    EXPECT_EQ(expectLarge, result["data"]["baseData"]["properties"][expectSmall150].get<std::string>());
}

#endif  // HAVE_MAT_AI