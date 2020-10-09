//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
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

TEST(TransmitProfileRuleTests, TimerConstructor_TimersSizeThree_TimersSet)
{
    TransmitProfileRule rule{{1, 2, 3}};
    ASSERT_EQ(rule.timers.size(), size_t{3});
    ASSERT_EQ(rule.timers[0], 1);
    ASSERT_EQ(rule.timers[1], 2);
    ASSERT_EQ(rule.timers[2], 3);
}

TEST(TransmitProfileRuleTests, TimerConstructor_TimersSizeThree_OtherValuesUnmodified)
{
    TransmitProfileRule rule{{1, 2, 3}};
    ASSERT_EQ(rule.netCost, NetworkCost::NetworkCost_Any);
    ASSERT_EQ(rule.powerState, PowerSource::PowerSource_Any);
    ASSERT_EQ(rule.netType, NetworkType::NetworkType_Any);
    ASSERT_EQ(rule.netSpeed, unsigned{0});
}

TEST(TransmitProfileRuleTests, NetCostAndTimerConstructor_NetCostLowTimersSizeThree_NetCostAndTimersSet)
{
    TransmitProfileRule rule{NetworkCost::NetworkCost_Unmetered, {1, 2, 3}};
    ASSERT_EQ(rule.netCost, NetworkCost::NetworkCost_Unmetered);
    ASSERT_EQ(rule.timers.size(), size_t{3});
    ASSERT_EQ(rule.timers[0], 1);
    ASSERT_EQ(rule.timers[1], 2);
    ASSERT_EQ(rule.timers[2], 3);
}

TEST(TransmitProfileRuleTests, NetCostAndTimerConstructor_NetCostLowTimersSizeThree_OtherValuesUnmodified)
{
    TransmitProfileRule rule{NetworkCost::NetworkCost_Unmetered, {1, 2, 3}};
    ASSERT_EQ(rule.powerState, PowerSource::PowerSource_Any);
    ASSERT_EQ(rule.netType, NetworkType::NetworkType_Any);
    ASSERT_EQ(rule.netSpeed, unsigned{0});
}

TEST(TransmitProfileRuleTests, NetCostPowerSourceAndTimerConstructor_NetCostLowPowerSourceBatteryTimersSizeThree_NetCostPowerSourceAndTimersSet)
{
    TransmitProfileRule rule{NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Battery, {1, 2, 3}};
    ASSERT_EQ(rule.netCost, NetworkCost::NetworkCost_Unmetered);
    ASSERT_EQ(rule.powerState, PowerSource::PowerSource_Battery);
    ASSERT_EQ(rule.timers.size(), size_t{3});
    ASSERT_EQ(rule.timers[0], 1);
    ASSERT_EQ(rule.timers[1], 2);
    ASSERT_EQ(rule.timers[2], 3);
}

TEST(TransmitProfileRuleTests, NetCostPowerSourceAndTimerConstructor_NetCostLowPowerSourceBatteryTimersSizeThree_OtherValuesUnmodified)
{
    TransmitProfileRule rule{NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Battery, {1, 2, 3}};
    ASSERT_EQ(rule.netType, NetworkType::NetworkType_Any);
    ASSERT_EQ(rule.netSpeed, unsigned{0});
}
