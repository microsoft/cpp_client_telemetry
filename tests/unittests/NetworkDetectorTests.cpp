// Copyright (c) Microsoft Corporation. All rights reserved.

#include "common/Common.hpp"

#if defined(_WIN32) && defined(HAVE_MAT_NETDETECT)
#include "api/LogManagerFactory.hpp"
#include "pal/desktop/NetworkDetector.hpp"
#include "common/network-detector-test-access.hpp"

#include <future>

using namespace MAT;
using namespace testing;

class StopDetectorOnNetworkChange : public DebugEventListener
{
   public:
    explicit StopDetectorOnNetworkChange(MATW::NetworkDetector& detector) :
        detector(detector)
    {
    }

    void OnDebugEvent(DebugEvent& event) override
    {
        if (event.type == EVT_NET_CHANGED && !handled.exchange(true))
        {
            detector.Stop();
            stopped.set_value();
        }
    }

    std::future<void> GetStoppedFuture()
    {
        return stopped.get_future();
    }

   private:
    MATW::NetworkDetector& detector;
    std::atomic<bool> handled{false};
    std::promise<void> stopped;
};

TEST(NetworkDetectorTests, MapsConnectivityHintNetworkCosts)
{
    EXPECT_EQ(MATW::MapNetworkCost(NetworkConnectivityCostHintUnrestricted, false, false, false), NetworkCost_Unmetered);
    EXPECT_EQ(MATW::MapNetworkCost(NetworkConnectivityCostHintFixed, false, false, false), NetworkCost_Metered);
    EXPECT_EQ(MATW::MapNetworkCost(NetworkConnectivityCostHintVariable, false, false, false), NetworkCost_Metered);
    EXPECT_EQ(MATW::MapNetworkCost(NetworkConnectivityCostHintUnknown, false, false, false), NetworkCost_Unknown);
}

TEST(NetworkDetectorTests, MapsRestrictiveConnectivityHints)
{
    EXPECT_EQ(MATW::MapNetworkCost(NetworkConnectivityCostHintUnrestricted, true, false, false), NetworkCost_Roaming);
    EXPECT_EQ(MATW::MapNetworkCost(NetworkConnectivityCostHintUnrestricted, false, true, false), NetworkCost_Roaming);
    EXPECT_EQ(MATW::MapNetworkCost(NetworkConnectivityCostHintUnrestricted, false, false, true), NetworkCost_Roaming);
}

TEST(NetworkDetectorTests, MapsLegacyCostFlagsIncludingCombinedRestrictions)
{
    EXPECT_EQ(MATW::MapLegacyNetworkCost(NLM_CONNECTION_COST_UNKNOWN), NetworkCost_Unknown);
    EXPECT_EQ(MATW::MapLegacyNetworkCost(NLM_CONNECTION_COST_UNRESTRICTED), NetworkCost_Unmetered);
    EXPECT_EQ(MATW::MapLegacyNetworkCost(NLM_CONNECTION_COST_FIXED), NetworkCost_Metered);
    EXPECT_EQ(MATW::MapLegacyNetworkCost(NLM_CONNECTION_COST_VARIABLE), NetworkCost_Metered);
    for (const DWORD flag : { NLM_CONNECTION_COST_ROAMING, NLM_CONNECTION_COST_OVERDATALIMIT,
                             NLM_CONNECTION_COST_APPROACHINGDATALIMIT, NLM_CONNECTION_COST_CONGESTED })
    {
        EXPECT_EQ(MATW::MapLegacyNetworkCost(NLM_CONNECTION_COST_UNRESTRICTED | flag), NetworkCost_Roaming);
        EXPECT_EQ(MATW::MapLegacyNetworkCost(NLM_CONNECTION_COST_FIXED | flag), NetworkCost_Roaming);
    }
}

TEST(NetworkDetectorTests, StartsReadsCostAndStopsWithoutNetworkListManager)
{
    const auto moduleBefore = GetModuleHandleW(L"netprofm.dll");

    MATW::NetworkDetector detector;
    if (!MATW::NetworkDetectorTestAccess::HasNativeBackend(detector))
    {
        GTEST_SKIP() << "IP Helper connectivity hints are unavailable on this Windows release.";
    }
    ASSERT_TRUE(detector.Start());
    EXPECT_TRUE(detector.isUp());
    EXPECT_EQ(GetModuleHandleW(L"netprofm.dll"), moduleBefore);

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
    EXPECT_EQ(GetModuleHandleW(L"netprofm.dll"), moduleBefore);
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

TEST(NetworkDetectorTests, FailedSubscriptionCleansUpAndCanRetry)
{
    MATW::NetworkDetector detector;
    MATW::NetworkDetectorTestAccess::FailNativeSubscription(detector);
    for (unsigned iteration = 0; iteration < 3; ++iteration)
    {
        EXPECT_FALSE(detector.Start());
        EXPECT_FALSE(detector.isUp());
        EXPECT_FALSE(detector.QueueNetworkCostRefresh());
        detector.Stop();
    }
    MATW::NetworkDetectorTestAccess::UseLegacyBackend(detector);
    EXPECT_TRUE(detector.Start());
    detector.Stop();
    EXPECT_FALSE(detector.isUp());
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

TEST(NetworkDetectorTests, NetworkChangeListenerCanStopDetector)
{
    ILogConfiguration configuration;
    configuration[CFG_BOOL_ENABLE_NET_DETECT] = false;
    ILogManager* logManager = LogManagerFactory::Create(configuration);
    ASSERT_NE(logManager, nullptr);
    MATW::NetworkDetector detector;
    StopDetectorOnNetworkChange listener(detector);
    auto stopped = listener.GetStoppedFuture();
    logManager->AddEventListener(EVT_NET_CHANGED, listener);

    ASSERT_TRUE(detector.Start());
    ASSERT_EQ(stopped.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    EXPECT_FALSE(detector.isUp());
    detector.Stop();

    logManager->RemoveEventListener(EVT_NET_CHANGED, listener);
    EXPECT_EQ(LogManagerFactory::Destroy(logManager), STATUS_SUCCESS);
}

enum class NetworkDetectorBackend
{
    Native,
    Legacy,
    LegacyWithoutCost
};

class NetworkDetectorBackendTests : public TestWithParam<NetworkDetectorBackend>
{
protected:
    void SelectBackend(MATW::NetworkDetector& detector)
    {
        if (GetParam() == NetworkDetectorBackend::Legacy)
        {
            MATW::NetworkDetectorTestAccess::UseLegacyBackend(detector);
        }
        else if (GetParam() == NetworkDetectorBackend::LegacyWithoutCost)
        {
            MATW::NetworkDetectorTestAccess::UseLegacyBackendWithoutCost(detector);
        }
    }
};

class BlockingStopDetectorOnNetworkChange : public DebugEventListener
{
public:
    BlockingStopDetectorOnNetworkChange(MATW::NetworkDetector& detector, bool stopBeforeRelease) :
        detector(detector), stopBeforeRelease(stopBeforeRelease), released(release.get_future())
    {
    }

    void OnDebugEvent(DebugEvent& event) override
    {
        if (event.type != EVT_NET_CHANGED || handled.exchange(true))
        {
            return;
        }
        if (stopBeforeRelease)
        {
            detector.Stop();
        }
        entered.set_value();
        released.wait();
        if (!stopBeforeRelease)
        {
            detector.Stop();
        }
    }

    std::future<void> GetEnteredFuture() { return entered.get_future(); }
    void Release() { release.set_value(); }

private:
    MATW::NetworkDetector& detector;
    bool stopBeforeRelease;
    std::atomic<bool> handled{false};
    std::promise<void> entered;
    std::promise<void> release;
    std::future<void> released;
};

TEST_P(NetworkDetectorBackendTests, ConcurrentExternalAndReentrantStopsDrainCallback)
{
    ILogConfiguration configuration;
    configuration[CFG_BOOL_ENABLE_NET_DETECT] = false;
    ILogManager* manager = LogManagerFactory::Create(configuration);
    ASSERT_NE(manager, nullptr);
    MATW::NetworkDetector detector;
    SelectBackend(detector);
    BlockingStopDetectorOnNetworkChange listener(detector, false);
    auto entered = listener.GetEnteredFuture();
    manager->AddEventListener(EVT_NET_CHANGED, listener);
    EXPECT_TRUE(detector.Start());
    EXPECT_EQ(entered.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    auto firstStop = std::async(std::launch::async, [&] { detector.Stop(); });
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (detector.isUp() && std::chrono::steady_clock::now() < deadline)
    {
        std::this_thread::yield();
    }
    EXPECT_FALSE(detector.isUp());
    auto secondStop = std::async(std::launch::async, [&] { detector.Stop(); });
    EXPECT_EQ(firstStop.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);
    EXPECT_EQ(secondStop.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);
    listener.Release();
    EXPECT_EQ(firstStop.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    EXPECT_EQ(secondStop.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    firstStop.get();
    secondStop.get();
    manager->RemoveEventListener(EVT_NET_CHANGED, listener);
    EXPECT_EQ(LogManagerFactory::Destroy(manager), STATUS_SUCCESS);
}

TEST_P(NetworkDetectorBackendTests, ExternalStopDrainsCallbackAfterReentrantStop)
{
    ILogConfiguration configuration;
    configuration[CFG_BOOL_ENABLE_NET_DETECT] = false;
    ILogManager* manager = LogManagerFactory::Create(configuration);
    ASSERT_NE(manager, nullptr);
    MATW::NetworkDetector detector;
    SelectBackend(detector);
    BlockingStopDetectorOnNetworkChange listener(detector, true);
    auto entered = listener.GetEnteredFuture();
    manager->AddEventListener(EVT_NET_CHANGED, listener);
    EXPECT_TRUE(detector.Start());
    EXPECT_EQ(entered.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    EXPECT_FALSE(detector.isUp());
    auto stopped = std::async(std::launch::async, [&] { detector.Stop(); });
    EXPECT_EQ(stopped.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);
    listener.Release();
    EXPECT_EQ(stopped.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    stopped.get();
    manager->RemoveEventListener(EVT_NET_CHANGED, listener);
    EXPECT_EQ(LogManagerFactory::Destroy(manager), STATUS_SUCCESS);
}

TEST_P(NetworkDetectorBackendTests, RepeatedStartReadAndStop)
{
    MATW::NetworkDetector detector;
    SelectBackend(detector);
    for (unsigned iteration = 0; iteration < 10; ++iteration)
    {
        ASSERT_TRUE(detector.Start());
        if (GetParam() != NetworkDetectorBackend::Native)
        {
            EXPECT_EQ(MATW::NetworkDetectorTestAccess::LegacySubscriptionCount(detector), 3u);
        }
        if (GetParam() == NetworkDetectorBackend::LegacyWithoutCost)
        {
            EXPECT_FALSE(MATW::NetworkDetectorTestAccess::HasLegacyCost(detector));
            EXPECT_EQ(detector.GetNetworkCost(), NetworkCost_Unknown);
            EXPECT_EQ(MATW::NetworkDetectorTestAccess::LegacySubscriptionCount(detector), 3u);
        }
        EXPECT_EQ(detector.GetCurrentNetworkCost(), detector.GetNetworkCost());
        detector.Stop();
        EXPECT_EQ(MATW::NetworkDetectorTestAccess::LegacySubscriptionCount(detector), 0u);
        EXPECT_FALSE(detector.isUp());
        EXPECT_FALSE(detector.QueueNetworkCostRefresh());
    }
}

TEST_P(NetworkDetectorBackendTests, RestartDrainsPreviousCallback)
{
    ILogConfiguration configuration;
    configuration[CFG_BOOL_ENABLE_NET_DETECT] = false;
    ILogManager* manager = LogManagerFactory::Create(configuration);
    ASSERT_NE(manager, nullptr);
    MATW::NetworkDetector detector;
    SelectBackend(detector);
    BlockingStopDetectorOnNetworkChange listener(detector, true);
    auto entered = listener.GetEnteredFuture();
    manager->AddEventListener(EVT_NET_CHANGED, listener);
    EXPECT_TRUE(detector.Start());
    EXPECT_EQ(entered.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    auto restarted = std::async(std::launch::async, [&] { return detector.Start(); });
    EXPECT_EQ(restarted.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);
    listener.Release();
    EXPECT_EQ(restarted.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    EXPECT_TRUE(restarted.get());
    EXPECT_TRUE(detector.isUp());
    detector.Stop();
    manager->RemoveEventListener(EVT_NET_CHANGED, listener);
    EXPECT_EQ(LogManagerFactory::Destroy(manager), STATUS_SUCCESS);
}

TEST_P(NetworkDetectorBackendTests, QueuedRefreshRaceDoesNotOutliveStop)
{
    MATW::NetworkDetector detector;
    SelectBackend(detector);
    ASSERT_TRUE(detector.Start());
    std::atomic<bool> keepQueuing{true};
    std::thread callbacks([&] {
        while (keepQueuing.load(std::memory_order_acquire))
        {
            detector.QueueNetworkCostRefresh();
        }
    });
    detector.Stop();
    keepQueuing.store(false, std::memory_order_release);
    callbacks.join();
    EXPECT_FALSE(detector.QueueNetworkCostRefresh());
}

TEST_P(NetworkDetectorBackendTests, CostReadsDoNotWaitOnAnEarlierListenerAfterRestart)
{
    MATW::NetworkDetector detector;
    SelectBackend(detector);
    ASSERT_TRUE(detector.Start());
    std::atomic<bool> keepReading{true};
    std::promise<void> firstRead;
    auto readCompleted = firstRead.get_future();
    std::thread reader([&] {
        detector.GetCurrentNetworkCost();
        firstRead.set_value();
        while (keepReading.load(std::memory_order_acquire))
        {
            detector.GetCurrentNetworkCost();
        }
    });
    EXPECT_EQ(readCompleted.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    for (unsigned iteration = 0; iteration < 20; ++iteration)
    {
        detector.Stop();
        EXPECT_TRUE(detector.Start());
    }
    keepReading.store(false, std::memory_order_release);
    detector.Stop();
    reader.join();
    EXPECT_FALSE(detector.isUp());
}

TEST_P(NetworkDetectorBackendTests, ListenerCanStopDetector)
{
    ILogConfiguration configuration;
    configuration[CFG_BOOL_ENABLE_NET_DETECT] = false;
    ILogManager* manager = LogManagerFactory::Create(configuration);
    ASSERT_NE(manager, nullptr);
    MATW::NetworkDetector detector;
    SelectBackend(detector);
    StopDetectorOnNetworkChange listener(detector);
    auto stopped = listener.GetStoppedFuture();
    manager->AddEventListener(EVT_NET_CHANGED, listener);
    EXPECT_TRUE(detector.Start());
    EXPECT_EQ(stopped.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    detector.Stop();
    EXPECT_FALSE(detector.isUp());
    manager->RemoveEventListener(EVT_NET_CHANGED, listener);
    EXPECT_EQ(LogManagerFactory::Destroy(manager), STATUS_SUCCESS);
}

TEST(NetworkDetectorTests, LegacyCostQueryFailurePreservesConnectivity)
{
    MATW::NetworkDetector detector;
    MATW::NetworkDetectorTestAccess::FailLegacyCostQuery(detector);
    for (unsigned iteration = 0; iteration < 3; ++iteration)
    {
        ASSERT_TRUE(detector.Start());
        EXPECT_TRUE(detector.isUp());
        EXPECT_FALSE(MATW::NetworkDetectorTestAccess::HasLegacyCost(detector));
        EXPECT_EQ(detector.GetCurrentNetworkCost(), NetworkCost_Unknown);
        EXPECT_EQ(MATW::NetworkDetectorTestAccess::LegacySubscriptionCount(detector), 3u);
        detector.Stop();
        EXPECT_FALSE(MATW::NetworkDetectorTestAccess::HasLegacyManager(detector));
    }
    MATW::NetworkDetectorTestAccess::UseLegacyBackendWithoutCost(detector);
    EXPECT_TRUE(detector.Start());
    EXPECT_EQ(detector.GetCurrentNetworkCost(), NetworkCost_Unknown);
    detector.Stop();
}

TEST(NetworkDetectorTests, PartialLegacySubscriptionFailureCleansUpAndCanRetry)
{
    MATW::NetworkDetector detector;
    MATW::NetworkDetectorTestAccess::FailLegacySubscription(detector);
    for (unsigned iteration = 0; iteration < 3; ++iteration)
    {
        EXPECT_FALSE(detector.Start());
        EXPECT_FALSE(detector.isUp());
        EXPECT_FALSE(detector.QueueNetworkCostRefresh());
        EXPECT_EQ(MATW::NetworkDetectorTestAccess::LegacySubscriptionCount(detector), 0u);
        detector.Stop();
    }
    MATW::NetworkDetectorTestAccess::UseLegacyBackend(detector);
    EXPECT_TRUE(detector.Start());
    detector.Stop();
}

TEST(NetworkDetectorTests, DisconnectFailureStillCompletesApartmentShutdown)
{
    MATW::NetworkDetector detector;
    MATW::NetworkDetectorTestAccess::FailLegacyDisconnect(detector);
    for (unsigned iteration = 0; iteration < 3; ++iteration)
    {
        ASSERT_TRUE(detector.Start());
        detector.Stop();
        EXPECT_FALSE(detector.isUp());
        EXPECT_FALSE(detector.QueueNetworkCostRefresh());
        EXPECT_FALSE(MATW::NetworkDetectorTestAccess::HasLegacyCost(detector));
        EXPECT_EQ(MATW::NetworkDetectorTestAccess::LegacySubscriptionCount(detector), 0u);
    }
}

INSTANTIATE_TEST_SUITE_P(NativeAndLegacy, NetworkDetectorBackendTests,
                        Values(NetworkDetectorBackend::Native, NetworkDetectorBackend::Legacy,
                               NetworkDetectorBackend::LegacyWithoutCost));
#endif
