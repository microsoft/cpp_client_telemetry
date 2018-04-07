#if 0
// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include "controlplane/TenantDataSerializer.hpp"
#include "json.hpp"

using namespace testing;
using namespace ARIASDK_NS::ControlPlane;

TEST(TenantDataSerializerTests, SerializeTenantData_InputIsNull_ReturnsEmptyString)
{
    TenantDataSerializer serializer;
    std::string expectedOutput;

    ASSERT_EQ(expectedOutput, serializer.SerializeTenantData(nullptr));
}

TEST(TenantDataSerializerTests, SerializeTenantData_InputHasNoSettings_ReturnsEmptyString)
{
    TenantDataSerializer serializer;
    std::string expectedOutput;
    TenantData tenantData;

    ASSERT_EQ(expectedOutput, serializer.SerializeTenantData(&tenantData));
}

TEST(TenantDataSerializerTests, SerializeTenantData_InputHasSettings_ReturnsExpectedString)
{
    TenantDataSerializer serializer;
    std::string expectedOutput("{\"BoolMap\":{\"key1\":false,\"key2\":true},\"LongMap\":{\"key3\":99,\"key4\":-5},\"StringMap\":{\"key5\":\"red\",\"key6\":\"blue\",\"key7\":\"{With \\\\ \\\" Escaped chars}\"}}");
    TenantData tenantData;

    tenantData.m_boolMap["key1"] = false;
    tenantData.m_boolMap["key2"] = true;
    tenantData.m_longMap["key3"] = 99;
    tenantData.m_longMap["key4"] = -5;
    tenantData.m_stringMap["key5"] = std::string("red");
    tenantData.m_stringMap["key6"] = std::string("blue");
    tenantData.m_stringMap["key7"] = std::string("{With \\ \" Escaped chars}");
    ASSERT_EQ(expectedOutput, serializer.SerializeTenantData(&tenantData));
}

TEST(TenantDataSerializerTests, DeserializeTenantData_InputIsEmpty_ReturnsNullptr)
{
    TenantDataSerializer serializer;
    std::string input;

    ASSERT_EQ(nullptr, serializer.DeserializeTenantData(input));
}

TEST(TenantDataSerializerTests, DeserializeTenantData_InputIsInvalid_ReturnsNullptr)
{
    TenantDataSerializer serializer;
    std::string input("This is invalid JSON!");

    ASSERT_EQ(nullptr, serializer.DeserializeTenantData(input));
}

TEST(TenantDataSerializerTests, DeserializeTenantData_InputIsValid_ContainsNoSettings_ReturnsEmptyNonDummyTenantDataPtr)
{
    TenantDataSerializer serializer;
    std::string input("{\"NoDataHere\":{\"key1\":false,\"key2\":true}}");

    TenantDataPtr tenantData = serializer.DeserializeTenantData(input);
    ASSERT_NE(nullptr, tenantData);
    ASSERT_FALSE(tenantData->m_isDummy);
    ASSERT_TRUE(tenantData->m_boolMap.empty());
    ASSERT_TRUE(tenantData->m_longMap.empty());
    ASSERT_TRUE(tenantData->m_stringMap.empty());
}

TEST(TenantDataSerializerTests, DeserializeTenantData_InputIsValid_ContainsSettings_ReturnsPopulatedNonDummyTenantDataPtr)
{
    TenantDataSerializer serializer;
    std::string input("{\"BoolMap\":{\"key1\":false,\"key2\":true},\"LongMap\":{\"key3\":99,\"key4\":-5},\"StringMap\":{\"key5\":\"red\",\"key6\":\"blue\",\"key7\":\"{With \\\\ \\\" Escaped chars}\"}}");

    TenantDataPtr tenantData = serializer.DeserializeTenantData(input);
    ASSERT_NE(nullptr, tenantData);
    ASSERT_FALSE(tenantData->m_isDummy);
    ASSERT_EQ(tenantData->m_boolMap["key1"], false);
    ASSERT_EQ(tenantData->m_boolMap["key2"], true);
    ASSERT_EQ(tenantData->m_longMap["key3"], 99);
    ASSERT_EQ(tenantData->m_longMap["key4"], -5);
    ASSERT_EQ(tenantData->m_stringMap["key5"], std::string("red"));
    ASSERT_EQ(tenantData->m_stringMap["key6"], std::string("blue"));
    ASSERT_EQ(tenantData->m_stringMap["key7"], std::string("{With \\ \" Escaped chars}"));
}

TEST(TenantDataSerializerTests, DeserializeTenantData_InputIsValid_ContainsSettingsAndExtraData_ReturnsPopulatedNonDummyTenantDataPtr)
{
    TenantDataSerializer serializer;
    std::string input("{\"BoolMap\":{\"key1\":false,\"key2\":true},\"UnknownStuff\":{\"key1\":false,\"key2\":true},\"LongMap\":{\"key3\":99,\"key4\":-5},\"StringMap\":{\"key5\":\"red\",\"key6\":\"blue\",\"key7\":\"{With \\\\ \\\" Escaped chars}\"}}");

    TenantDataPtr tenantData = serializer.DeserializeTenantData(input);
    ASSERT_NE(nullptr, tenantData);
    ASSERT_FALSE(tenantData->m_isDummy);
    ASSERT_EQ(tenantData->m_boolMap["key1"], false);
    ASSERT_EQ(tenantData->m_boolMap["key2"], true);
    ASSERT_EQ(tenantData->m_longMap["key3"], 99);
    ASSERT_EQ(tenantData->m_longMap["key4"], -5);
    ASSERT_EQ(tenantData->m_stringMap["key5"], std::string("red"));
    ASSERT_EQ(tenantData->m_stringMap["key6"], std::string("blue"));
    ASSERT_EQ(tenantData->m_stringMap["key7"], std::string("{With \\ \" Escaped chars}"));
}

TEST(TenantDataSerializerTests, SerializeDeserialize_RoundTripReturnsIdenticalData)
{
    TenantDataSerializer serializer;

    TenantData original;
    original.m_boolMap["abc"] = false;
    original.m_boolMap["def"] = true;
    original.m_longMap["ghi"] = 0xFFFFFFFF;
    original.m_longMap["jkl"] = 0;
    original.m_stringMap["abc"] = std::string("Julius");
    original.m_stringMap["def"] = std::string("Augustus");
    original.m_stringMap["ghi"] = std::string("Nero");
    original.m_stringMap["jkl"] = std::string();

    TenantDataPtr roundTrip = serializer.DeserializeTenantData(serializer.SerializeTenantData(&original));

    ASSERT_EQ(original.m_boolMap.size(), roundTrip->m_boolMap.size());
    ASSERT_EQ(original.m_longMap.size(), roundTrip->m_longMap.size());
    ASSERT_EQ(original.m_stringMap.size(), roundTrip->m_stringMap.size());

    for (auto pair : original.m_boolMap)
        ASSERT_EQ(pair.second, roundTrip->m_boolMap[pair.first]);
    for (auto pair : original.m_longMap)
        ASSERT_EQ(pair.second, roundTrip->m_longMap[pair.first]);
    for (auto pair : original.m_stringMap)
        ASSERT_EQ(pair.second, roundTrip->m_stringMap[pair.first]);
}
#endif
