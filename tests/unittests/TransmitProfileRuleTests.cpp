// Copyright (c) Microsoft. All rights reserved.
#include "common/Common.hpp"
#include <TransmitProfiles.hpp>

using namespace testing;
using namespace MAT;

TEST(TransmitProfileRuleTests, DefaultConstructor_NetworkCostIsAny)
{
    TransmitProfileRule rule;
    ASSERT_EQ(rule.netCost, NetworkCost_Any);
}

TEST(TransmitProfileRuleTests, DefaultConstructor_PowerStateIsAny)
{
    TransmitProfileRule rule;
    ASSERT_EQ(rule.powerState, PowerSource_Any);
}

TEST(TransmitProfileRuleTests, DefaultConstructor_NetworkTypeIsAny)
{
    TransmitProfileRule rule;
    ASSERT_EQ(rule.netType, NetworkType_Any);
}

TEST(TransmitProfileRuleTests, DefaultConstructor_NetworkSpeedIsZero)
{
    TransmitProfileRule rule;
    ASSERT_EQ(rule.netSpeed, unsigned{0});
}

TEST(TransmitProfileRuleTests, DefaultConstructor_TimersSizeIsZero)
{
    TransmitProfileRule rule;
    ASSERT_EQ(rule.timers.size(), size_t{0});
}