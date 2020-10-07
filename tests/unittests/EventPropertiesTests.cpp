//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "api/ContextFieldsProvider.hpp"

using namespace testing;
using namespace MAT;


TEST(EventPropertiesTests, Construction)
{
    EventProperties ep("test");
    EXPECT_THAT(ep.GetName(), Eq("test"));
    EXPECT_THAT(ep.GetLatency(), EventLatency_Normal);
    EXPECT_THAT(ep.GetTimestamp(), 0ll);
    EXPECT_THAT(ep.GetProperties(), SizeIs(1));
    EXPECT_THAT(ep.GetPiiProperties(), IsEmpty());
    EXPECT_TRUE(std::get<0>(ep.TryGetLevel()));
    EXPECT_THAT(std::get<1>(ep.TryGetLevel()), DIAG_LEVEL_OPTIONAL);
}

TEST(EventPropertiesTests, Name)
{
    EventProperties ep("");
    EXPECT_THAT(ep.GetName(), Eq("undefined"));

    ep.SetName("Abcde123");
    EXPECT_THAT(ep.GetName(), Eq("Abcde123"));

    ep.SetName("");
    EXPECT_THAT(ep.GetName(), Eq("Abcde123"));

    ep.SetName("Weird. Characters_are_weird");
    EXPECT_THAT(ep.GetName(), Eq("Abcde123"));

    ep.SetName("My.Event.Name");
    EXPECT_THAT(ep.GetName(), Eq("My.Event.Name"));
}

TEST(EventPropertiesTests, DiagnosticLevel)
{
    EventProperties epOptional("Optional", DIAG_LEVEL_OPTIONAL);
    EXPECT_THAT(epOptional.GetName(), Eq("Optional"));
    EXPECT_TRUE(std::get<0>(epOptional.TryGetLevel()));
    EXPECT_THAT(std::get<1>(epOptional.TryGetLevel()), DIAG_LEVEL_OPTIONAL);

    epOptional.SetLevel(DIAG_LEVEL_REQUIRED);
    EXPECT_TRUE(std::get<0>(epOptional.TryGetLevel()));
    EXPECT_THAT(std::get<1>(epOptional.TryGetLevel()), DIAG_LEVEL_REQUIRED);

    EventProperties epRequired("Required", DIAG_LEVEL_REQUIRED);
    EXPECT_THAT(epRequired.GetName(), Eq("Required"));
    EXPECT_TRUE(std::get<0>(epRequired.TryGetLevel()));
    EXPECT_THAT(std::get<1>(epRequired.TryGetLevel()), DIAG_LEVEL_REQUIRED);

    EventProperties epCustom("Custom", 55);
    EXPECT_THAT(epCustom.GetName(), Eq("Custom"));
    EXPECT_TRUE(std::get<0>(epCustom.TryGetLevel()));
    EXPECT_THAT(std::get<1>(epCustom.TryGetLevel()), 55);
}

TEST(EventPropertiesTests, Timestamp)
{
    EventProperties ep("test");
    EXPECT_THAT(ep.GetTimestamp(), 0ll);

    ep.SetTimestamp(1234567890123456789ll);
    EXPECT_THAT(ep.GetTimestamp(), 1234567890123456789ll);

    ep.SetTimestamp(0);
    EXPECT_THAT(ep.GetTimestamp(), 0ll);

    ep.SetTimestamp(-1);
    EXPECT_THAT(ep.GetTimestamp(), -1);
}

TEST(EventPropertiesTests, Latency)
{
    EventProperties ep("test");
    EXPECT_THAT(ep.GetLatency(), EventLatency_Normal);

    ep.SetPriority(EventPriority_Off);
    EXPECT_THAT(ep.GetLatency(), EventLatency_Off);

    ep.SetPriority(EventPriority_Low);
    EXPECT_THAT(ep.GetLatency(), EventLatency_Normal);

    ep.SetPriority(EventPriority_Normal);
    EXPECT_THAT(ep.GetLatency(), EventLatency_Normal);

    ep.SetPriority(EventPriority_High);
    EXPECT_THAT(ep.GetLatency(), EventLatency_RealTime);

    ep.SetPriority(EventPriority_Immediate);
    EXPECT_THAT(ep.GetLatency(), EventLatency_RealTime);

}

TEST(EventPropertiesTests, Properties)
{
    EventProperties ep("test");
    EXPECT_THAT(ep.GetProperties(), SizeIs(1));

    ep.SetProperty("one",                          std::string("two"));
    ep.SetProperty("Weird.Characters_are_weird", std::string("value #2"));
    ep.SetProperty("public",                       "value #3", PiiKind_None);
    EXPECT_THAT(ep.GetProperties(), SizeIs(4));
    EXPECT_THAT(ep.GetProperties(), Contains(Pair("one", EventProperty("two"))));
    EXPECT_THAT(ep.GetProperties(), Contains(Pair("Weird.Characters_are_weird", EventProperty("value #2"))));
    EXPECT_THAT(ep.GetProperties(), Contains(Pair("public", EventProperty("value #3"))));
    EXPECT_THAT(ep.GetPiiProperties(), IsEmpty());
}

TEST(EventPropertiesTests, NumericProperties)
{
    EventProperties ep("test");
    EXPECT_THAT(ep.GetProperties(), SizeIs(1));

    ep.SetProperty("char",     static_cast<signed char>(-123));
    ep.SetProperty("int",      static_cast<int>(-123123));
    ep.SetProperty("int64_t",  static_cast<int64_t>(-123123123));
    ep.SetProperty("uint8",    static_cast<uint8_t>(255));
    ep.SetProperty("unsigned", static_cast<unsigned>(999999999));
    ep.SetProperty("ull",      (uint64_t)(9999999999999999999ull));
    ep.SetProperty("float",    static_cast<float>(1234.5f));
    ep.SetProperty("double",   static_cast<double>(-9876.543));
//    ep.SetProperty("lodouble", static_cast<long double>(-98769876.5435431));
    ep.SetProperty("true",     true);
    ep.SetProperty("false",    false);
    EXPECT_THAT(ep.GetProperties(), SizeIs(11));
    EXPECT_THAT(ep.GetProperties(), Contains(Pair("char", EventProperty(-123))));
    EXPECT_THAT(ep.GetProperties(), Contains(Pair("int",  EventProperty(-123123))));
    EXPECT_THAT(ep.GetProperties(), Contains(Pair("int64_t", EventProperty(-123123123))));
    EXPECT_THAT(ep.GetProperties(), Contains(Pair("uint8", EventProperty(255))));
    EXPECT_THAT(ep.GetProperties(), Contains(Pair("unsigned", 999999999)));
    EXPECT_THAT(ep.GetProperties(), Contains(Pair("ull", EventProperty((uint64_t)9999999999999999999u))));
    EXPECT_THAT(ep.GetProperties(), Contains(Pair("float", EventProperty(1234.500000))));
    EXPECT_THAT(ep.GetProperties(), Contains(Pair("double", EventProperty(-9876.543000))));
//    EXPECT_THAT(ep.GetProperties(), Contains(Pair("lodouble", EventProperty("-98769876.543543"))));
    EXPECT_THAT(ep.GetProperties(), Contains(Pair("true", EventProperty(true))));
    EXPECT_THAT(ep.GetProperties(), Contains(Pair("false", EventProperty(false))));
    EXPECT_THAT(ep.GetPiiProperties(), IsEmpty());
}

TEST(EventPropertiesTests, PiiProperties)
{
    EventProperties ep("test");
    EXPECT_THAT(ep.GetPiiProperties(), IsEmpty());

    ep.SetProperty("secret",                       "dn=value",    PiiKind_DistinguishedName);
    ep.SetProperty("Weird.Characters_are_weird", "12.34.56.78", PiiKind_IPv4Address);
    EXPECT_THAT(ep.GetPiiProperties(), SizeIs(2));
    EXPECT_THAT(ep.GetPiiProperties(), Contains(Pair("secret",                       Pair("dn=value",    PiiKind_DistinguishedName))));
    EXPECT_THAT(ep.GetPiiProperties(), Contains(Pair("Weird.Characters_are_weird", Pair("12.34.56.78", PiiKind_IPv4Address))));
}

TEST(EventPropertiesTests, CustomerContentProperties)
{
    EventProperties ep("test");
    EXPECT_THAT(ep.GetPiiProperties(), IsEmpty());

    ep.SetProperty("customerData", "value", CustomerContentKind_GenericData);
    EXPECT_THAT(ep.GetPiiProperties(), SizeIs(1));
    EXPECT_THAT(ep.GetPiiProperties(), Contains(Pair("customerData", Pair("value", CustomerContentKind_GenericData))));
}

TEST(EventPropertiesTests, SetLevel_AddsProperty)
{
    EventProperties properties;
    properties.SetLevel(42);
    EXPECT_THAT(properties.GetProperties(), SizeIs(1));
}

TEST(EventPropertiesTests, SetLevel_SetsPropertyName)
{
    EventProperties properties;
    properties.SetLevel(42);
    EXPECT_TRUE(properties.GetProperties().find(COMMONFIELDS_EVENT_LEVEL) != properties.GetProperties().cend());
}

TEST(EventPropertiesTests, SetLevel_SetsPropertyTypeInt64)
{
    EventProperties properties;
    properties.SetLevel(42);
    const auto& property = properties.GetProperties().find(COMMONFIELDS_EVENT_LEVEL)->second;
    EXPECT_EQ(property.type, TYPE_INT64);
}

TEST(EventPropertiesTests, SetLevel_SetsValueCorrectly)
{
    EventProperties properties;
    properties.SetLevel(42);
    const auto& property = properties.GetProperties().find(COMMONFIELDS_EVENT_LEVEL)->second;
    EXPECT_EQ(property.as_int64, 42);
}

TEST(EventPropertiesTests, TryGetLevel_NotTheRightType_ReturnsFalseAndZero)
{
    EventProperties properties;
    properties.SetProperty(COMMONFIELDS_EVENT_LEVEL, "Not a number");
    auto result = properties.TryGetLevel();
    EXPECT_FALSE(std::get<0>(result));
    EXPECT_EQ(std::get<1>(result), 0);
}

TEST(EventPropertiesTests, TryGetLevel_ValueLargerThanUint8_ReturnsFalseAndZero)
{
    EventProperties properties;
    properties.SetProperty(COMMONFIELDS_EVENT_LEVEL, 257);
    auto result = properties.TryGetLevel();
    EXPECT_FALSE(std::get<0>(result));
    EXPECT_EQ(std::get<1>(result), 0);
}

TEST(EventPropertiesTests, TryGetLevel_ValueLessThanZero_ReturnsFalseAndZero)
{
    EventProperties properties;
    properties.SetProperty(COMMONFIELDS_EVENT_LEVEL, -1);
    auto result = properties.TryGetLevel();
    EXPECT_FALSE(std::get<0>(result));
    EXPECT_EQ(std::get<1>(result), 0);
}

TEST(EventPropertiesTests, TryGetLevel_ValidValue_ReturnsTrueAndCorrectValue)
{
    EventProperties properties;
    properties.SetProperty(COMMONFIELDS_EVENT_LEVEL, 42);
    auto result = properties.TryGetLevel();
    EXPECT_TRUE(std::get<0>(result));
    EXPECT_EQ(std::get<1>(result), 42);
}

TEST(EventPropertiesTests, TryGetLevel_ValueSetBySetLevel_ReturnsTrueAndCorrectValue)
{
    EventProperties properties;
    properties.SetLevel(42);
    auto result = properties.TryGetLevel();
    EXPECT_TRUE(std::get<0>(result));
    EXPECT_EQ(std::get<1>(result), 42);
}
