//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "tpm/DeviceStateHandler.hpp"

using namespace testing;
using namespace MAT;

class TestDeviceStateHandler : public DeviceStateHandler
{
public:
    using DeviceStateHandler::m_networkCost;
    using DeviceStateHandler::m_networkType;
    using DeviceStateHandler::m_powerSource;

    virtual void _UpdateDeviceCondition() override { }
};

class DeviceStateHandlerTests : public Test
{
public:
    TestDeviceStateHandler handler;
};

TEST_F(DeviceStateHandlerTests, Constructor_Default_NetworkCostIsUnmetered)
{
    ASSERT_EQ(NetworkCost_Unmetered, handler.m_networkCost);
}

TEST_F(DeviceStateHandlerTests, Constructor_Default_PowerSourceIsCharging)
{
    ASSERT_EQ(PowerSource_Charging, handler.m_powerSource);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsNetworkTypeValueIsNegativeOne_SetsNetworkTypeAny)
{
    handler.OnChanged(std::string{NETWORK_TYPE}, std::string{"-1"});
    ASSERT_EQ(NetworkType_Any, handler.m_networkType);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsNetworkTypeValueIsZero_SetsNetworkTypeUnknown)
{
    handler.OnChanged(std::string{NETWORK_TYPE}, std::string{"0"});
    ASSERT_EQ(NetworkType_Unknown, handler.m_networkType);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsNetworkTypeValueIsOne_SetsNetworkTypeWired)
{
    handler.OnChanged(std::string{NETWORK_TYPE}, std::string{"1"});
    ASSERT_EQ(NetworkType_Wired, handler.m_networkType);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsNetworkTypeValueIsTwo_SetsNetworkTypeWifi)
{
    handler.OnChanged(std::string{NETWORK_TYPE}, std::string{"2"});
    ASSERT_EQ(NetworkType_Wifi, handler.m_networkType);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsNetworkTypeValueIsThree_SetsNetworkTypeWWAN)
{
    handler.OnChanged(std::string{NETWORK_TYPE}, std::string{"3"});
    ASSERT_EQ(NetworkType_WWAN, handler.m_networkType);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsNetworkCostValueIsNegativeOne_SetsNetworkCostAny)
{
    handler.OnChanged(std::string{NETWORK_COST}, std::string{"-1"});
    ASSERT_EQ(NetworkCost_Any, handler.m_networkCost);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsNetworkCostValueIsZero_SetsNetworkCostUnknown)
{
    handler.OnChanged(std::string{NETWORK_COST}, std::string{"0"});
    ASSERT_EQ(NetworkCost_Unknown, handler.m_networkCost);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsNetworkCostValueIsOne_SetsNetworkCostUnmetered)
{
    handler.OnChanged(std::string{NETWORK_COST}, std::string{"1"});
    ASSERT_EQ(NetworkCost_Unmetered, handler.m_networkCost);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsNetworkCostValueIsTwo_SetsNetworkCostMetered)
{
    handler.OnChanged(std::string{NETWORK_COST}, std::string{"2"});
    ASSERT_EQ(NetworkCost_Metered, handler.m_networkCost);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsNetworkCostValueIsThree_SetsNetworkCostRoaming)
{
    handler.OnChanged(std::string{NETWORK_COST}, std::string{"3"});
    ASSERT_EQ(NetworkCost_Roaming, handler.m_networkCost);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsNetworkCostValueIsThree_SetsNetworkCostOverDataLimit)
{
    handler.OnChanged(std::string{NETWORK_COST}, std::string{"3"});
    ASSERT_EQ(NetworkCost_OverDataLimit, handler.m_networkCost);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsPowerSourceValueIsNegativeOne_SetsPowerSourceAny)
{
    handler.OnChanged(std::string{POWER_SOURCE}, std::string{"-1"});
    ASSERT_EQ(PowerSource_Any, handler.m_powerSource);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsPowerSourceValueIsZero_SetsPowerSourceUnknown)
{
    handler.OnChanged(std::string{POWER_SOURCE}, std::string{"0"});
    ASSERT_EQ(PowerSource_Unknown, handler.m_powerSource);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsPowerSourceValueIsOne_SetsPowerSourceBattery)
{
    handler.OnChanged(std::string{POWER_SOURCE}, std::string{"1"});
    ASSERT_EQ(PowerSource_Battery, handler.m_powerSource);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsPowerSourceValueIsTwo_SetsPowerSourceCharging)
{
    handler.OnChanged(std::string{POWER_SOURCE}, std::string{"2"});
    ASSERT_EQ(PowerSource_Charging, handler.m_powerSource);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsPowerSourceValueIsThree_SetsPowerSourceLowBattery)
{
    handler.OnChanged(std::string{POWER_SOURCE}, std::string{"3"});
    ASSERT_EQ(PowerSource_LowBattery, handler.m_powerSource);
}

TEST_F(DeviceStateHandlerTests, OnChanged_PropertyNameIsInvalid_DoesntSetAnything)
{
    handler.OnChanged(std::string{"Foo"}, std::string{"-1"});
    ASSERT_EQ(PowerSource_Charging, handler.m_powerSource);
    ASSERT_EQ(NetworkCost_Unmetered, handler.m_networkCost);
}
