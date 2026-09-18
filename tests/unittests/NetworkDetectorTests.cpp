// Copyright (c) Microsoft Corporation. All rights reserved.

#include "common/Common.hpp"

#ifdef _WIN32
#include "pal/desktop/NetworkDetector.hpp"

using namespace MAT;
using namespace testing;

TEST(NetworkDetectorTests, StartsReadsCostAndStopsWithoutNetworkListManager)
{
    ASSERT_EQ(GetModuleHandleW(L"netprofm.dll"), nullptr);

    MATW::NetworkDetector detector;
    ASSERT_TRUE(detector.Start());
    EXPECT_TRUE(detector.isUp());

    const auto cost = detector.GetCurrentNetworkCost();
    EXPECT_THAT(cost, AnyOf(
        Eq(NetworkCost_Unknown),
        Eq(NetworkCost_Unmetered),
        Eq(NetworkCost_Metered),
        Eq(NetworkCost_Roaming)));

    detector.Stop();
    EXPECT_FALSE(detector.isUp());
    EXPECT_EQ(GetModuleHandleW(L"netprofm.dll"), nullptr);
}
#endif
