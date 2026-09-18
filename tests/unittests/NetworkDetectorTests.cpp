// Copyright (c) Microsoft Corporation. All rights reserved.

#include "common/Common.hpp"

#if defined(_WIN32) && defined(HAVE_MAT_NETDETECT)
#include "pal/desktop/NetworkDetector.hpp"

using namespace MAT;
using namespace testing;

TEST(NetworkDetectorTests, MapsWinRTNetworkCosts)
{
    EXPECT_EQ(MATW::MapNetworkCost(NetworkCostType_Unrestricted, false, false, false), NetworkCost_Unmetered);
    EXPECT_EQ(MATW::MapNetworkCost(NetworkCostType_Fixed, false, false, false), NetworkCost_Metered);
    EXPECT_EQ(MATW::MapNetworkCost(NetworkCostType_Variable, false, false, false), NetworkCost_Metered);
    EXPECT_EQ(MATW::MapNetworkCost(NetworkCostType_Unknown, false, false, false), NetworkCost_Unknown);
}

TEST(NetworkDetectorTests, MapsRestrictiveWinRTNetworkStates)
{
    EXPECT_EQ(MATW::MapNetworkCost(NetworkCostType_Unrestricted, true, false, false), NetworkCost_Roaming);
    EXPECT_EQ(MATW::MapNetworkCost(NetworkCostType_Unrestricted, false, true, false), NetworkCost_Roaming);
    EXPECT_EQ(MATW::MapNetworkCost(NetworkCostType_Unrestricted, false, false, true), NetworkCost_Roaming);
}

TEST(NetworkDetectorTests, StartsReadsCostAndStopsWithoutNetworkListManager)
{
    ASSERT_EQ(GetModuleHandleW(L"netprofm.dll"), nullptr);

    MATW::NetworkDetector detector;
    ASSERT_TRUE(detector.Start());
    EXPECT_TRUE(detector.isUp());
    EXPECT_EQ(GetModuleHandleW(L"netprofm.dll"), nullptr);

    const auto cost = detector.GetCurrentNetworkCost();
    EXPECT_THAT(cost, AnyOf(
        Eq(NetworkCost_Unknown),
        Eq(NetworkCost_Unmetered),
        Eq(NetworkCost_Metered),
        Eq(NetworkCost_Roaming)));
    EXPECT_EQ(detector.GetNetworkCost(), cost);

    detector.Stop();
    EXPECT_FALSE(detector.isUp());
    EXPECT_EQ(GetModuleHandleW(L"netprofm.dll"), nullptr);
}
#endif
