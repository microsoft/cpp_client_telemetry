// Copyright (c) Microsoft Corporation. All rights reserved.

#include "common/Common.hpp"

#if defined(_WIN32) && defined(HAVE_MAT_NETDETECT)
#include "pal/desktop/NetworkDetector.hpp"

using namespace MAT;
using namespace testing;

TEST(NetworkDetectorTests, MapsWinRTNetworkCosts)
{
    EXPECT_EQ(MATW::MapNetworkCost(NetworkCostType_Unrestricted, false, false, false, false), NetworkCost_Unmetered);
    EXPECT_EQ(MATW::MapNetworkCost(NetworkCostType_Fixed, false, false, false, false), NetworkCost_Metered);
    EXPECT_EQ(MATW::MapNetworkCost(NetworkCostType_Variable, false, false, false, false), NetworkCost_Metered);
    EXPECT_EQ(MATW::MapNetworkCost(NetworkCostType_Unknown, false, false, false, false), NetworkCost_Unknown);
}

TEST(NetworkDetectorTests, MapsRestrictiveWinRTNetworkStates)
{
    EXPECT_EQ(MATW::MapNetworkCost(NetworkCostType_Unrestricted, true, false, false, false), NetworkCost_Roaming);
    EXPECT_EQ(MATW::MapNetworkCost(NetworkCostType_Unrestricted, false, true, false, false), NetworkCost_Roaming);
    EXPECT_EQ(MATW::MapNetworkCost(NetworkCostType_Unrestricted, false, false, true, false), NetworkCost_Roaming);
    EXPECT_EQ(MATW::MapNetworkCost(NetworkCostType_Unrestricted, false, false, false, true), NetworkCost_Roaming);
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
    EXPECT_FALSE(detector.QueueNetworkCostRefresh());
    EXPECT_EQ(GetModuleHandleW(L"netprofm.dll"), nullptr);
}

TEST(NetworkDetectorTests, QueuedNetworkCallbackRaceDoesNotOutliveStop)
{
    MATW::NetworkDetector detector;
    ASSERT_TRUE(detector.Start());

    std::atomic<bool> keepQueuing{true};
    std::thread callbackThread([&]()
                               {
        while (keepQueuing.load(std::memory_order_acquire)) {
            detector.QueueNetworkCostRefresh();
        } });

    detector.Stop();
    EXPECT_FALSE(detector.QueueNetworkCostRefresh());
    keepQueuing.store(false, std::memory_order_release);
    callbackThread.join();
}

TEST(NetworkDetectorTests, ConcurrentStopWaitsForStartupPublication)
{
    for (int iteration = 0; iteration < 20; ++iteration)
    {
        MATW::NetworkDetector detector;
        std::atomic<bool> startReturned{false};
        std::thread startThread([&]()
                                {
            detector.Start();
            startReturned.store(true, std::memory_order_release); });

        while (!detector.isUp() && !startReturned.load(std::memory_order_acquire))
        {
            std::this_thread::yield();
        }

        detector.Stop();
        startThread.join();
        EXPECT_FALSE(detector.isUp());
    }
}
#endif
