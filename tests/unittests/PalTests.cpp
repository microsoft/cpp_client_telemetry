//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "config/RuntimeConfig_Default.hpp"
#include "pal/PAL.hpp"
#include "pal/PseudoRandomGenerator.hpp"
#include "pal/TaskDispatcher.hpp"
#include "pal/WorkerThread.hpp"
#include "NullObjects.hpp"
#include "Version.hpp"

#include <atomic>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <set>
#include <thread>
#include <utility>
#include <vector>

#ifdef HAVE_MAT_LOGGING
#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#endif
using namespace PAL::detail;
#endif

using namespace testing;

namespace PAL_NS_BEGIN {
#ifdef HAVE_MAT_LOGGING
    namespace detail
    {
        extern std::recursive_mutex debugLogMutex;
        extern std::string debugLogPath;
        extern std::unique_ptr<std::fstream> debugLogStream;
        extern bool isLoggingInited;
        extern uint64_t debugLogEpoch;
    }
#endif

    class PALTest
    {
    public:
        static bool HasDispatcher(const PlatformAbstractionLayer& pal)
        {
            std::lock_guard<std::mutex> lock(pal.m_stateLock);
            return static_cast<bool>(pal.m_taskDispatcher);
        }

        static bool IsInitializing(const PlatformAbstractionLayer& pal)
        {
            std::lock_guard<std::mutex> lock(pal.m_stateLock);
            return pal.m_initializing;
        }
    };
} PAL_NS_END

#if defined(_WIN32) || defined(_WIN64)
namespace PAL_NS_BEGIN {
    std::string formatWindowsOsFullVersion(
        unsigned long majorVersion,
        unsigned long minorVersion,
        unsigned long buildNumber,
        uint32_t updateBuildRevision,
        bool hasUpdateBuildRevision);
} PAL_NS_END
#endif

namespace
{
#ifdef HAVE_MAT_LOGGING
    class ScopedLogState
    {
    public:
        ScopedLogState()
        {
            std::lock_guard<std::recursive_mutex> lock(PAL::detail::debugLogMutex);
            m_path = std::move(PAL::detail::debugLogPath);
            m_stream = std::move(PAL::detail::debugLogStream);
            m_enabled = PAL::detail::isLoggingInited;
            m_epoch = PAL::detail::debugLogEpoch;
            PAL::detail::isLoggingInited = false;
        }

        ~ScopedLogState()
        {
            std::lock_guard<std::recursive_mutex> lock(PAL::detail::debugLogMutex);
            PAL::detail::debugLogStream = std::move(m_stream);
            PAL::detail::debugLogPath = std::move(m_path);
            PAL::detail::debugLogEpoch = m_epoch;
            PAL::detail::isLoggingInited = m_enabled;
        }

    private:
        std::string m_path;
        std::unique_ptr<std::fstream> m_stream;
        uint64_t m_epoch = 0;
        bool m_enabled = false;
    };
#endif

    class BlockingRuntimeConfig : public MAT::RuntimeConfig_Default
    {
    public:
        BlockingRuntimeConfig(
            MAT::ILogConfiguration& configuration,
            std::promise<void>& entered,
            std::shared_future<void> release)
            : RuntimeConfig_Default(configuration), m_entered(entered), m_release(std::move(release))
        {
        }

        bool HasConfig(const char* key) override
        {
            if (!m_blocked.exchange(true))
            {
                m_entered.set_value();
                m_release.wait();
            }
            return RuntimeConfig_Default::HasConfig(key);
        }

    private:
        std::promise<void>& m_entered;
        std::shared_future<void> m_release;
        std::atomic<bool> m_blocked { false };
    };

#if HAVE_EXCEPTIONS
    class ThrowOnceRuntimeConfig : public MAT::RuntimeConfig_Default
    {
    public:
        explicit ThrowOnceRuntimeConfig(MAT::ILogConfiguration& configuration)
            : RuntimeConfig_Default(configuration)
        {
        }

        MAT::Variant& operator[](const char* key) override
        {
            if (m_throw && std::string(key) == CFG_BOOL_ENABLE_NET_DETECT)
            {
                m_throw = false;
                throw std::runtime_error("PAL initialization failed");
            }
            return RuntimeConfig_Default::operator[](key);
        }

    private:
        bool m_throw = true;
    };
#endif

    class ReinitializePalTask
    {
    public:
        ReinitializePalTask(
            PAL::PlatformAbstractionLayer& pal,
            MAT::IRuntimeConfig& configuration,
            std::shared_future<void> release,
            std::atomic<bool>& shouldInitialize,
            std::promise<void>& started,
            std::promise<void>& finished,
            std::shared_ptr<MAT::ITaskDispatcher>& replacement)
            : m_pal(pal),
              m_configuration(configuration),
              m_release(std::move(release)),
              m_shouldInitialize(shouldInitialize),
              m_started(started),
              m_finished(finished),
              m_replacement(replacement)
        {
        }

        void Run()
        {
            m_started.set_value();
            m_release.wait();
            if (m_shouldInitialize.load())
            {
                m_pal.initialize(m_configuration);
                m_replacement = m_pal.getDefaultTaskDispatcher();
                m_pal.shutdown();
            }
            m_finished.set_value();
        }

    private:
        PAL::PlatformAbstractionLayer& m_pal;
        MAT::IRuntimeConfig& m_configuration;
        std::shared_future<void> m_release;
        std::atomic<bool>& m_shouldInitialize;
        std::promise<void>& m_started;
        std::promise<void>& m_finished;
        std::shared_ptr<MAT::ITaskDispatcher>& m_replacement;
    };

    class SignalTask
    {
    public:
        explicit SignalTask(std::promise<void>& signal) : m_signal(signal) {}
        void Run() { m_signal.set_value(); }

    private:
        std::promise<void>& m_signal;
    };

    class BlockingTask
    {
    public:
        BlockingTask(std::promise<void>& started, std::shared_future<void> release)
            : m_started(started), m_release(std::move(release))
        {
        }

        void Run()
        {
            m_started.set_value();
            m_release.wait();
        }

    private:
        std::promise<void>& m_started;
        std::shared_future<void> m_release;
    };

    class BlockingSemanticContext : public MAT::NullContext
    {
    public:
        BlockingSemanticContext(std::promise<void>& entered, std::shared_future<void> release)
            : m_entered(entered), m_release(std::move(release))
        {
        }

        void SetDeviceId(const std::string& value) override
        {
            deviceId = value;
            sawDeviceId = true;
        }

        void SetDeviceModel(const std::string& value) override
        {
            m_entered.set_value();
            m_release.wait();
            deviceModel = value;
            sawDeviceModel = true;
        }

        void SetOsName(const std::string& value) override
        {
            osName = value;
            sawOsName = true;
        }

        void SetNetworkProvider(const std::string&) override
        {
            sawNetworkProvider = true;
        }

        std::string deviceId;
        std::string deviceModel;
        std::string osName;
        bool sawDeviceId = false;
        bool sawDeviceModel = false;
        bool sawOsName = false;
        bool sawNetworkProvider = false;

    private:
        std::promise<void>& m_entered;
        std::shared_future<void> m_release;
    };
}

class PalTests : public Test
{
#ifdef HAVE_MAT_LOGGING
    ScopedLogState m_logState;
#endif
};

TEST_F(PalTests, ConcurrentInitializationPublishesCompleteState)
{
    MAT::ILogConfiguration logConfiguration;
    logConfiguration[CFG_BOOL_ENABLE_TRACE] = false;
    std::promise<void> firstEntered;
    std::promise<void> releaseFirst;
    auto firstEnteredFuture = firstEntered.get_future();
    BlockingRuntimeConfig configuration(logConfiguration, firstEntered, releaseFirst.get_future().share());
    PAL::PlatformAbstractionLayer pal;

    struct Observation
    {
        std::shared_ptr<PAL::ISystemInformation> system;
        std::shared_ptr<PAL::IDeviceInformation> device;
        std::shared_ptr<PAL::INetworkInformation> network;
        std::shared_ptr<MAT::ITaskDispatcher> dispatcher;
    };
    std::array<Observation, 2> observations;
    auto initialize = [&](size_t index) {
        pal.initialize(configuration);
        observations[index].system = pal.GetSystemInformation();
        observations[index].device = pal.GetDeviceInformation();
        observations[index].network = pal.GetNetworkInformation();
        observations[index].dispatcher = pal.getDefaultTaskDispatcher();
    };

    std::thread first([&] { initialize(0); });
    if (firstEnteredFuture.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    {
        releaseFirst.set_value();
        first.join();
        pal.shutdown();
        FAIL() << "The first initializer did not enter the configuration";
    }

    std::promise<void> secondStarted;
    std::promise<void> secondFinished;
    auto secondStartedFuture = secondStarted.get_future();
    auto secondFinishedFuture = secondFinished.get_future();
    std::thread second([&] {
        secondStarted.set_value();
        initialize(1);
        secondFinished.set_value();
    });
    secondStartedFuture.wait();

    auto getter = std::async(std::launch::async, [&] { return pal.GetSystemInformation(); });
    const auto getterStatus = getter.wait_for(std::chrono::seconds(2));
    const auto secondStatus = secondFinishedFuture.wait_for(std::chrono::milliseconds(250));
    releaseFirst.set_value();
    first.join();
    second.join();

    EXPECT_EQ(getterStatus, std::future_status::ready);
    EXPECT_EQ(getter.get(), nullptr);
    EXPECT_EQ(secondStatus, std::future_status::timeout);
    EXPECT_THAT(observations[0].system, NotNull());
    EXPECT_THAT(observations[0].device, NotNull());
    EXPECT_THAT(observations[0].network, NotNull());
    EXPECT_THAT(observations[0].dispatcher, NotNull());
    EXPECT_EQ(observations[0].system, observations[1].system);
    EXPECT_EQ(observations[0].device, observations[1].device);
    EXPECT_EQ(observations[0].network, observations[1].network);
    EXPECT_EQ(observations[0].dispatcher, observations[1].dispatcher);

    std::promise<void> releaseShutdown;
    auto shutdownGate = releaseShutdown.get_future().share();
    std::thread shutdown1([&] { shutdownGate.wait(); pal.shutdown(); });
    std::thread shutdown2([&] { shutdownGate.wait(); pal.shutdown(); });
    releaseShutdown.set_value();
    shutdown1.join();
    shutdown2.join();
    EXPECT_EQ(pal.GetSystemInformation(), nullptr);
    EXPECT_EQ(pal.GetDeviceInformation(), nullptr);
    EXPECT_EQ(pal.GetNetworkInformation(), nullptr);
    EXPECT_FALSE(PAL::PALTest::HasDispatcher(pal));
}

#if HAVE_EXCEPTIONS
TEST_F(PalTests, FailedInitializationCanBeRetried)
{
    MAT::ILogConfiguration logConfiguration;
    logConfiguration[CFG_BOOL_ENABLE_TRACE] = false;
    ThrowOnceRuntimeConfig configuration(logConfiguration);
    PAL::PlatformAbstractionLayer pal;

    EXPECT_THROW(pal.initialize(configuration), std::runtime_error);
    ASSERT_FALSE(PAL::PALTest::IsInitializing(pal));
    EXPECT_FALSE(pal.GetSystemInformation());
    EXPECT_FALSE(pal.GetDeviceInformation());
    EXPECT_FALSE(pal.GetNetworkInformation());
    pal.initialize(configuration);
    EXPECT_THAT(pal.GetSystemInformation(), NotNull());
    pal.shutdown();
}
#endif

TEST_F(PalTests, RetainedDispatcherSurvivesFinalShutdown)
{
    MAT::ILogConfiguration logConfiguration;
    logConfiguration[CFG_BOOL_ENABLE_TRACE] = false;
    MAT::RuntimeConfig_Default configuration(logConfiguration);
    PAL::PlatformAbstractionLayer pal;
    pal.initialize(configuration);
    auto dispatcher = pal.getDefaultTaskDispatcher();
    pal.shutdown();

    std::promise<void> ran;
    auto ranFuture = ran.get_future();
    SignalTask task(ran);
    PAL::dispatchTask(dispatcher.get(), &task, &SignalTask::Run);
    EXPECT_EQ(ranFuture.wait_for(std::chrono::seconds(5)), std::future_status::ready);
    dispatcher.reset();
}

TEST_F(PalTests, WorkerCanReinitializeWhileFinalShutdownReleasesDispatcher)
{
    MAT::ILogConfiguration logConfiguration;
    logConfiguration[CFG_BOOL_ENABLE_TRACE] = false;
    MAT::RuntimeConfig_Default configuration(logConfiguration);
    PAL::PlatformAbstractionLayer pal;
    pal.initialize(configuration);
    auto dispatcher = pal.getDefaultTaskDispatcher();

    std::promise<void> workerStarted;
    std::promise<void> releaseWorker;
    std::promise<void> workerFinished;
    auto startedFuture = workerStarted.get_future();
    auto finishedFuture = workerFinished.get_future();
    std::atomic<bool> shouldInitialize { false };
    std::shared_ptr<MAT::ITaskDispatcher> replacement;
    ReinitializePalTask task(
        pal, configuration, releaseWorker.get_future().share(), shouldInitialize,
        workerStarted, workerFinished, replacement);
    PAL::dispatchTask(dispatcher.get(), &task, &ReinitializePalTask::Run);
    if (startedFuture.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    {
        releaseWorker.set_value();
        dispatcher.reset();
        pal.shutdown();
        FAIL() << "The worker did not start";
    }
    dispatcher.reset();

    std::thread shutdown([&] { pal.shutdown(); });
    std::promise<bool> dispatcherRetired;
    auto retiredFuture = dispatcherRetired.get_future();
    std::thread observer([&] {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (PAL::PALTest::HasDispatcher(pal) && std::chrono::steady_clock::now() < deadline)
        {
            std::this_thread::yield();
        }
        dispatcherRetired.set_value(!PAL::PALTest::HasDispatcher(pal));
    });
    const bool retired =
        retiredFuture.wait_for(std::chrono::seconds(5)) == std::future_status::ready && retiredFuture.get();
    shouldInitialize.store(retired);
    releaseWorker.set_value();
    observer.join();
    const auto finished = finishedFuture.wait_for(std::chrono::seconds(5));
    shutdown.join();

    EXPECT_TRUE(retired);
    EXPECT_EQ(finished, std::future_status::ready);
    EXPECT_THAT(replacement, NotNull());
}

TEST_F(PalTests, UuidGeneration)
{
    // Canonical UUID string length ("8-4-4-4-12") and the number of UUIDs
    // generated for the uniqueness check below.
    constexpr size_t UuidStringLength = 36;
    constexpr size_t UuidBatchSize = 1000;

    std::string uuid0 = PAL::generateUuidString();

    EXPECT_THAT(uuid0.length(), UuidStringLength);

    std::string mask = uuid0;
    for (char& ch : mask) {
#ifdef _WIN32
        if (::isdigit(ch) || (::isupper(ch) && ::isxdigit(ch)))
#else
        if (::isdigit(ch) || (::islower(ch) && ::isxdigit(ch)))
#endif
	{
            ch = 'x';
        }
    }
    EXPECT_THAT(mask, Eq("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"));

    std::string uuid1 = PAL::generateUuidString();

    EXPECT_THAT(uuid1.length(), UuidStringLength);

    size_t diff = 0;
    for (size_t i = 0; i < UuidStringLength; i++) {
        diff += (uuid0[i] != uuid1[i]);
    }
    EXPECT_THAT(diff, Gt(20u));

    // A batch of generated UUIDs must all be distinct (guards against a stuck
    // or low-entropy generator).
    std::set<std::string> uuids;
    for (size_t i = 0; i < UuidBatchSize; i++) {
        std::string u = PAL::generateUuidString();
        EXPECT_THAT(u.length(), UuidStringLength);
        uuids.insert(u);
    }
    EXPECT_THAT(uuids.size(), UuidBatchSize);
}

TEST_F(PalTests, PseudoRandomGenerator)
{
    PAL::PseudoRandomGenerator prg;

    constexpr size_t NumQueries = 1000;
    constexpr size_t NumBuckets = 11;
    size_t buckets[NumBuckets] = {};

    for (size_t i = 0; i < NumQueries; i++) {
        double x = prg.getRandomDouble();
        buckets[static_cast<int>(x * NumBuckets)]++;

        // Check range [0,1)
        EXPECT_THAT(x, AllOf(Ge(0.0), Lt(1.0)));
    }

    // Check somewhat uniform distribution
    EXPECT_THAT(buckets, Not(Contains(Lt(NumQueries / NumBuckets / 2))));
    EXPECT_THAT(buckets, Not(Contains(Gt(2 * NumQueries / NumBuckets))));
}

TEST_F(PalTests, SystemTime)
{
    int64_t t0 = PAL::getUtcSystemTimeMs();

    PAL::sleep(369);

    int64_t t1 = PAL::getUtcSystemTimeMs();
    EXPECT_THAT(t1, Gt(t0 + 360));
    EXPECT_THAT(t1, Lt(t0 + 1000));
}

#if !defined(_WIN32) && !defined(_WIN64)
TEST_F(PalTests, SystemTimeInTicksPreservesSubMillisecondPrecision)
{
    constexpr int64_t TicksPerMillisecond = 10000;
    bool observedSubMillisecondTick = false;

    for (int i = 0; i < 1000 && !observedSubMillisecondTick; ++i)
    {
        observedSubMillisecondTick =
            PAL::getUtcSystemTimeinTicks() % TicksPerMillisecond != 0;
    }

    EXPECT_TRUE(observedSubMillisecondTick);
}
#endif

TEST_F(PalTests, FormatUtcTimestampMsAsISO8601)
{
    EXPECT_THAT(PAL::formatUtcTimestampMsAsISO8601(0ll),             Eq("1970-01-01T00:00:00.000Z"));
    EXPECT_THAT(PAL::formatUtcTimestampMsAsISO8601(1234567890123ll), Eq("2009-02-13T23:31:30.123Z"));
    EXPECT_THAT(PAL::formatUtcTimestampMsAsISO8601(2147483647999ll), Eq("2038-01-19T03:14:07.999Z"));
}

#if defined(_WIN32) || defined(_WIN64)
TEST_F(PalTests, WindowsOsFullVersionIncludesUbrWhenPresent)
{
    EXPECT_THAT(
        PAL::formatWindowsOsFullVersion(10, 0, 26200, 1234, true),
        Eq("10.0.26200.1234"));
}

TEST_F(PalTests, WindowsOsFullVersionOmitsUbrWhenMissing)
{
    EXPECT_THAT(
        PAL::formatWindowsOsFullVersion(10, 0, 26200, 0, false),
        Eq("10.0.26200"));
}

TEST_F(PalTests, WindowsOsFullVersionIncludesZeroUbrWhenPresent)
{
    EXPECT_THAT(
        PAL::formatWindowsOsFullVersion(10, 0, 26200, 0, true),
        Eq("10.0.26200.0"));
}
#endif

TEST_F(PalTests, MonotonicTime)
{
    int64_t t0 = PAL::getMonotonicTimeMs();

    PAL::sleep(789);

    int64_t t1 = PAL::getMonotonicTimeMs();
    EXPECT_THAT(t1 - t0, Gt(780));
    EXPECT_THAT(t1 - t0, Lt(1500));
}

TEST_F(PalTests, SemanticContextPopulation)
{
    MAT::ILogConfiguration logConfiguration;
    logConfiguration[CFG_BOOL_ENABLE_TRACE] = false;
    MAT::RuntimeConfig_Default configuration(logConfiguration);
    PAL::PlatformAbstractionLayer pal;
    pal.initialize(configuration);

    ASSERT_TRUE(pal.GetDeviceInformation());
    ASSERT_TRUE(pal.GetSystemInformation());
    ASSERT_TRUE(pal.GetNetworkInformation());
    const auto expectedDeviceId = pal.GetDeviceInformation()->GetDeviceId();
    const auto expectedDeviceModel = pal.GetDeviceInformation()->GetModel();
    const auto expectedOsName = pal.GetSystemInformation()->GetOsName();

    std::promise<void> entered;
    std::promise<void> release;
    auto enteredFuture = entered.get_future();
    BlockingSemanticContext context(entered, release.get_future().share());
    std::thread populate([&] { pal.registerSemanticContext(&context); });
    if (enteredFuture.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    {
        release.set_value();
        populate.join();
        pal.shutdown();
        FAIL() << "Semantic-context population did not reach the device callback";
    }

    auto shutdown = std::async(std::launch::async, [&] { pal.shutdown(); });
    const auto shutdownStatus = shutdown.wait_for(std::chrono::seconds(2));
    release.set_value();
    populate.join();
    shutdown.get();

    EXPECT_EQ(shutdownStatus, std::future_status::ready);
#ifdef MATSDK_DISABLE_DEVICE_ID
    EXPECT_EQ(context.sawDeviceId, !expectedDeviceId.empty());
#else
    EXPECT_TRUE(context.sawDeviceId);
#endif
    EXPECT_EQ(context.deviceId, expectedDeviceId);
    EXPECT_TRUE(context.sawDeviceModel);
    EXPECT_EQ(context.deviceModel, expectedDeviceModel);
    EXPECT_TRUE(context.sawOsName);
    EXPECT_EQ(context.osName, expectedOsName);
    EXPECT_TRUE(context.sawNetworkProvider);
}

TEST_F(PalTests, SdkVersion)
{
    std::string v = PAL::getSdkVersion();

    // <Prefix>-<Platform>-<SKU>-<Projection>-<BuildVersion>

    EXPECT_THAT(std::count(v.cbegin(), v.cend(), '-'), Eq(4));
    EXPECT_THAT(v, StartsWith(EVTSDK_VERSION_PREFIX "-"));
    EXPECT_THAT(v.at(v.find('-', 0) + 1), Ne('-'));
    EXPECT_THAT(v, HasSubstr(std::string("-C++-" ECS_SUPP "-")));
    EXPECT_THAT(v, EndsWith(BUILD_VERSION_STR));

    EXPECT_THAT(PAL::getSdkVersion(), Eq(v));
}

namespace
{
    class ThrowingTaskHelper
    {
    public:
        void ThrowStdException() { throw std::runtime_error("worker task boom"); }
        void ThrowNonStdException() { throw 123; }
        void Signal(std::atomic<bool>* ran) { ran->store(true); }
    };

    class DroppingTaskDispatcher final : public ITaskDispatcher
    {
    public:
        void Join() override {}
        void Queue(Task* task) override { delete task; }

        bool Cancel(Task*, uint64_t = 0) override
        {
            cancelCalled = true;
            return false;
        }

        bool cancelCalled = false;
    };

    class ScheduledTaskTarget
    {
    public:
        explicit ScheduledTaskTarget(std::atomic<bool>& callbackRan) :
            m_callbackRan(callbackRan)
        {
        }

        void Callback()
        {
            m_callbackRan.store(true);
        }

    private:
        std::atomic<bool>& m_callbackRan;
    };

    class BlockingScheduledTaskTarget
    {
    public:
        void Callback()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_entered = true;
            m_condition.notify_all();
            m_condition.wait(lock, [this]() { return m_released; });
        }

        bool WaitUntilEntered()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_condition.wait_for(
                lock, std::chrono::seconds(2), [this]() { return m_entered; });
        }

        void Release()
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_released = true;
            }
            m_condition.notify_all();
        }

    private:
        std::mutex m_mutex;
        std::condition_variable m_condition;
        bool m_entered {false};
        bool m_released {false};
    };

    class ReentrantQueueScheduledTaskTarget
    {
    public:
        explicit ReentrantQueueScheduledTaskTarget(ITaskDispatcher* dispatcher) :
            m_dispatcher(dispatcher)
        {
        }

        void Callback()
        {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_entered = true;
                m_condition.notify_all();
                m_condition.wait(lock, [this]() { return m_queueAllowed; });
            }

            PAL::dispatchTask(
                m_dispatcher, this, &ReentrantQueueScheduledTaskTarget::FollowUp);

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_queueReturned = true;
            }
            m_condition.notify_all();
        }

        void FollowUp()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_followUpRan = true;
            m_condition.notify_all();
        }

        bool WaitUntilEntered()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_condition.wait_for(
                lock, std::chrono::seconds(2), [this]() { return m_entered; });
        }

        void AllowQueue()
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_queueAllowed = true;
            }
            m_condition.notify_all();
        }

        bool WaitUntilQueueReturned()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_condition.wait_for(
                lock, std::chrono::seconds(1), [this]() { return m_queueReturned; });
        }

        bool WaitUntilFollowUpRan()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_condition.wait_for(
                lock, std::chrono::seconds(2), [this]() { return m_followUpRan; });
        }

    private:
        ITaskDispatcher* m_dispatcher;
        std::mutex m_mutex;
        std::condition_variable m_condition;
        bool m_entered {false};
        bool m_queueAllowed {false};
        bool m_queueReturned {false};
        bool m_followUpRan {false};
    };
}

// A task throwing an exception must be contained by the worker thread loop;
// otherwise the exception unwinds out of the thread entry function and calls
// std::terminate, killing the host process.
TEST_F(PalTests, WorkerThreadContainsThrowingTask)
{
    auto dispatcher = PAL::WorkerThreadFactory::Create();
    ThrowingTaskHelper helper;
    std::atomic<bool> ranAfterStdThrow(false);
    std::atomic<bool> ranAfterNonStdThrow(false);

    PAL::dispatchTask(dispatcher.get(), &helper, &ThrowingTaskHelper::ThrowStdException);
    PAL::dispatchTask(dispatcher.get(), &helper, &ThrowingTaskHelper::Signal, &ranAfterStdThrow);

    PAL::dispatchTask(dispatcher.get(), &helper, &ThrowingTaskHelper::ThrowNonStdException);
    PAL::dispatchTask(dispatcher.get(), &helper, &ThrowingTaskHelper::Signal, &ranAfterNonStdThrow);

    // Wait for the follow-up tasks to run, proving the thread survived each throw.
    for (int i = 0; i < 500 && !(ranAfterStdThrow.load() && ranAfterNonStdThrow.load()); ++i)
        PAL::sleep(10);

    EXPECT_TRUE(ranAfterStdThrow.load());
    EXPECT_TRUE(ranAfterNonStdThrow.load());

    dispatcher->Join();
}

TEST_F(PalTests, ScheduleTaskReturnsNoOpHandleWhenDispatcherDropsTask)
{
    DroppingTaskDispatcher dispatcher;
    std::atomic<bool> callbackRan(false);
    ScheduledTaskTarget target(callbackRan);

    auto handle = PAL::scheduleTask(&dispatcher, 0, &target, &ScheduledTaskTarget::Callback);

    EXPECT_EQ(handle.GetTask(), nullptr);
    EXPECT_TRUE(handle.Cancel());
    EXPECT_FALSE(dispatcher.cancelCalled);
    EXPECT_FALSE(callbackRan.load());
}

TEST_F(PalTests, ScheduleTaskHandleClearsAfterCallbackCompletes)
{
    auto dispatcher = PAL::WorkerThreadFactory::Create();
    std::atomic<bool> callbackRan(false);
    ScheduledTaskTarget target(callbackRan);
    auto handle = PAL::scheduleTask(dispatcher.get(), 0, &target, &ScheduledTaskTarget::Callback);

    for (int i = 0; i < 500 && (!callbackRan.load() || handle.GetTask() != nullptr); ++i)
    {
        PAL::sleep(10);
    }

    EXPECT_TRUE(callbackRan.load());
    EXPECT_EQ(handle.GetTask(), nullptr);
    EXPECT_TRUE(handle.Cancel());

    dispatcher->Join();
}

TEST_F(PalTests, ScheduleTaskCancelSerializesTaskDestruction)
{
    auto dispatcher = PAL::WorkerThreadFactory::Create();
    std::atomic<bool> callbackRan(false);
    ScheduledTaskTarget target(callbackRan);
    auto handle = PAL::scheduleTask(
        dispatcher.get(), 60000, &target, &ScheduledTaskTarget::Callback);

    ASSERT_NE(handle.GetTask(), nullptr);
    EXPECT_TRUE(handle.Cancel());
    EXPECT_EQ(handle.GetTask(), nullptr);
    EXPECT_FALSE(callbackRan.load());

    dispatcher->Join();
}

TEST_F(PalTests, ScheduleTaskCancelWaitDoesNotDeadlockTaskDestruction)
{
    auto dispatcher = PAL::WorkerThreadFactory::Create();
    BlockingScheduledTaskTarget target;
    auto handle = PAL::scheduleTask(
        dispatcher.get(), 0, &target, &BlockingScheduledTaskTarget::Callback);

    if (!target.WaitUntilEntered())
    {
        target.Release();
        handle.Cancel(2000);
        dispatcher->Join();
        FAIL() << "scheduled task did not start";
    }

    std::atomic<bool> cancelReturned(false);
    bool cancelResult = false;
    std::thread canceller([&]() {
        cancelResult = handle.Cancel(2000);
        cancelReturned.store(true);
    });

    PAL::sleep(50);
    target.Release();
    for (int i = 0; i < 50 && !cancelReturned.load(); ++i)
    {
        PAL::sleep(10);
    }

    EXPECT_TRUE(cancelReturned.load());
    canceller.join();
    EXPECT_TRUE(cancelResult);
    for (int i = 0; i < 50 && handle.GetTask() != nullptr; ++i)
    {
        PAL::sleep(10);
    }
    EXPECT_EQ(handle.GetTask(), nullptr);

    dispatcher->Join();
}

TEST_F(PalTests, ScheduleTaskCancelWaitAllowsRunningTaskToQueue)
{
    constexpr uint64_t CancelWaitMs = 3000;
    auto dispatcher = PAL::WorkerThreadFactory::Create();
    ReentrantQueueScheduledTaskTarget target(dispatcher.get());
    auto handle = PAL::scheduleTask(
        dispatcher.get(), 0, &target, &ReentrantQueueScheduledTaskTarget::Callback);

    if (!target.WaitUntilEntered())
    {
        target.AllowQueue();
        handle.Cancel(CancelWaitMs);
        dispatcher->Join();
        FAIL() << "scheduled task did not start";
    }

    std::promise<void> cancelStarted;
    std::future<void> cancelStartedFuture = cancelStarted.get_future();
    std::promise<void> cancelFinished;
    std::future<void> cancelFinishedFuture = cancelFinished.get_future();
    bool cancelResult = false;
    std::thread canceller([&]() {
        cancelStarted.set_value();
        cancelResult = handle.Cancel(CancelWaitMs);
        cancelFinished.set_value();
    });

    EXPECT_EQ(cancelStartedFuture.wait_for(std::chrono::seconds(2)), std::future_status::ready);
    EXPECT_EQ(
        cancelFinishedFuture.wait_for(std::chrono::milliseconds(100)),
        std::future_status::timeout);

    target.AllowQueue();

    EXPECT_TRUE(target.WaitUntilQueueReturned());
    EXPECT_EQ(
        cancelFinishedFuture.wait_for(std::chrono::seconds(1)),
        std::future_status::ready);

    canceller.join();
    EXPECT_TRUE(cancelResult);
    EXPECT_TRUE(target.WaitUntilFollowUpRan());

    dispatcher->Join();
}

namespace
{
    class WorkerThreadScheduleTarget
    {
    public:
        void Callback() {}
    };

    class BlockingCancellationTarget
    {
    public:
        void Block()
        {
            std::unique_lock<std::mutex> lock(m_lock);
            m_entered = true;
            m_stateChanged.notify_all();
            m_stateChanged.wait(lock, [this]() { return m_release; });
        }

        void Signal()
        {
            std::lock_guard<std::mutex> lock(m_lock);
            m_successorRan = true;
            m_stateChanged.notify_all();
        }

        bool WaitUntilEntered()
        {
            std::unique_lock<std::mutex> lock(m_lock);
            return m_stateChanged.wait_for(
                lock, std::chrono::seconds{5}, [this]() { return m_entered; });
        }

        bool WaitUntilSuccessorRan()
        {
            std::unique_lock<std::mutex> lock(m_lock);
            return m_stateChanged.wait_for(
                lock, std::chrono::seconds{5}, [this]() { return m_successorRan; });
        }

        void Release()
        {
            std::lock_guard<std::mutex> lock(m_lock);
            m_release = true;
            m_stateChanged.notify_all();
        }

    private:
        std::mutex m_lock;
        std::condition_variable m_stateChanged;
        bool m_entered = false;
        bool m_release = false;
        bool m_successorRan = false;
    };

    class SelfDisposeHelper
    {
    public:
        std::function<void()> releaseLastRef;
        std::atomic<bool>* done = nullptr;

        void Run()
        {
            releaseLastRef();
            done->store(true);
        }
    };

    class SelfJoinTarget
    {
    public:
        explicit SelfJoinTarget(ITaskDispatcher* dispatcher)
            : m_dispatcher(dispatcher)
        {
        }

        void Run()
        {
            {
                std::unique_lock<std::mutex> lock(m_lock);
                m_entered = true;
                m_changed.notify_all();
                m_changed.wait(lock, [this]() { return m_allowJoin; });
            }

            m_dispatcher->Join();

            std::unique_lock<std::mutex> lock(m_lock);
            m_joinReturned = true;
            m_changed.notify_all();
            m_changed.wait(lock, [this]() { return m_allowReturn; });
        }

        bool WaitUntilEntered()
        {
            std::unique_lock<std::mutex> lock(m_lock);
            return m_changed.wait_for(
                lock, std::chrono::seconds(5), [this]() { return m_entered; });
        }

        bool WaitUntilJoinReturns()
        {
            std::unique_lock<std::mutex> lock(m_lock);
            return m_changed.wait_for(
                lock, std::chrono::seconds(5), [this]() { return m_joinReturned; });
        }

        void AllowJoin()
        {
            std::lock_guard<std::mutex> lock(m_lock);
            m_allowJoin = true;
            m_changed.notify_all();
        }

        void AllowReturn()
        {
            std::lock_guard<std::mutex> lock(m_lock);
            m_allowReturn = true;
            m_changed.notify_all();
        }

    private:
        ITaskDispatcher* m_dispatcher;
        std::mutex m_lock;
        std::condition_variable m_changed;
        bool m_entered = false;
        bool m_allowJoin = false;
        bool m_joinReturned = false;
        bool m_allowReturn = false;
    };
}

TEST_F(PalTests, ScheduleTaskAfterWorkerThreadJoinReturnsNoOpHandle)
{
    auto dispatcher = PAL::WorkerThreadFactory::Create();
    dispatcher->Join();
    WorkerThreadScheduleTarget target;

    auto handle = PAL::scheduleTask(
        dispatcher.get(), 100, &target, &WorkerThreadScheduleTarget::Callback);

    EXPECT_EQ(handle.GetTask(), nullptr);
    EXPECT_TRUE(handle.Cancel());
}

TEST_F(PalTests, ScheduleTaskHandleClearsAfterWorkerThreadCallbackCompletes)
{
    auto dispatcher = PAL::WorkerThreadFactory::Create();
    std::atomic<bool> callbackRan(false);

    class WorkerThreadCompletionTarget
    {
    public:
        explicit WorkerThreadCompletionTarget(std::atomic<bool>& callbackRan) :
            m_callbackRan(callbackRan)
        {
        }

        void Callback() { m_callbackRan.store(true); }

    private:
        std::atomic<bool>& m_callbackRan;
    } target(callbackRan);

    auto handle = PAL::scheduleTask(
        dispatcher.get(), 0, &target, &WorkerThreadCompletionTarget::Callback);

    for (int i = 0; i < 500 && !callbackRan.load(); ++i)
    {
        PAL::sleep(10);
    }

    ASSERT_TRUE(callbackRan.load());
    EXPECT_EQ(handle.GetTask(), nullptr);
    EXPECT_TRUE(handle.Cancel());

    dispatcher->Join();
}

TEST_F(PalTests, CancellingRunningTaskDoesNotDropSuccessor)
{
    auto dispatcher = PAL::WorkerThreadFactory::Create();
    constexpr int Iterations = 400;

    for (int iteration = 0; iteration < Iterations; ++iteration)
    {
        BlockingCancellationTarget target;
        auto running = PAL::scheduleTask(
            dispatcher.get(), 0, &target, &BlockingCancellationTarget::Block);

        if (!target.WaitUntilEntered())
        {
            target.Release();
            dispatcher->Join();
            FAIL() << "Worker did not start the blocking task";
            return;
        }

        auto successor = PAL::scheduleTask(
            dispatcher.get(), 0, &target, &BlockingCancellationTarget::Signal);
        std::promise<void> cancelStarted;
        auto cancelStartedFuture = cancelStarted.get_future();
        bool cancelResult = false;
        std::thread cancelThread([&]() {
            cancelStarted.set_value();
            cancelResult = running.Cancel(std::numeric_limits<uint64_t>::max());
        });

        cancelStartedFuture.wait();
        for (int i = 0; i < 100; ++i)
        {
            std::this_thread::yield();
        }
        target.Release();
        cancelThread.join();

        EXPECT_TRUE(cancelResult);
        if (!target.WaitUntilSuccessorRan())
        {
            dispatcher->Join();
            FAIL() << "Cancellation dropped the successor task at iteration " << iteration;
            return;
        }
        (void)successor;
    }

    dispatcher->Join();
}

TEST_F(PalTests, WorkerThreadSelfDisposeOnOwnThreadIsSafe)
{
    auto dispatcher = PAL::WorkerThreadFactory::Create();
    auto* raw = dispatcher.get();
    auto box = std::make_shared<decltype(dispatcher)>(std::move(dispatcher));

    std::atomic<bool> done(false);
    SelfDisposeHelper helper;
    helper.releaseLastRef = [box]() { box->reset(); };
    helper.done = &done;

    PAL::dispatchTask(raw, &helper, &SelfDisposeHelper::Run);

    for (int i = 0; i < 500 && !done.load(); ++i)
    {
        PAL::sleep(10);
    }
    ASSERT_TRUE(done.load());

    PAL::sleep(200);
}

TEST_F(PalTests, WorkerThreadSelfJoinDoesNotDeadlockExternalJoin)
{
    auto dispatcher = PAL::WorkerThreadFactory::Create();
    SelfJoinTarget target(dispatcher.get());
    PAL::dispatchTask(dispatcher.get(), &target, &SelfJoinTarget::Run);
    ASSERT_TRUE(target.WaitUntilEntered());

    std::promise<void> externalJoinStarted;
    auto externalJoinStartedFuture = externalJoinStarted.get_future();
    std::thread externalJoiner([&]() {
        externalJoinStarted.set_value();
        dispatcher->Join();
    });
    ASSERT_EQ(
        externalJoinStartedFuture.wait_for(std::chrono::seconds(2)),
        std::future_status::ready);

    PAL::sleep(50);
    target.AllowJoin();
    ASSERT_TRUE(target.WaitUntilJoinReturns());
    target.AllowReturn();
    externalJoiner.join();
}

TEST_F(PalTests, WorkerThreadSelfJoinSurvivesConcurrentFinalRelease)
{
    auto dispatcher = PAL::WorkerThreadFactory::Create();
    SelfJoinTarget target(dispatcher.get());
    PAL::dispatchTask(dispatcher.get(), &target, &SelfJoinTarget::Run);
    ASSERT_TRUE(target.WaitUntilEntered());

    target.AllowJoin();
    ASSERT_TRUE(target.WaitUntilJoinReturns());

    std::atomic<bool> releaseReturned(false);
    std::thread releaser(
        [owner = std::move(dispatcher), &releaseReturned]() mutable {
            owner.reset();
            releaseReturned.store(true, std::memory_order_release);
        });

    PAL::sleep(100);
    EXPECT_FALSE(releaseReturned.load(std::memory_order_acquire));
    target.AllowReturn();
    releaser.join();
    EXPECT_TRUE(releaseReturned.load(std::memory_order_acquire));
}

#ifdef HAVE_MAT_LOGGING
class LogInitTest : public Test
{
    protected:
        ScopedLogState m_logState;
        std::string validPath = "valid/path/";

        void SetUp() override
        {
            // Create the valid path directory and any intermediate directories
    #if defined(_WIN32) || defined(_WIN64)
            CreateDirectoryA("valid", NULL);
            CreateDirectoryA(validPath.c_str(), NULL);
    #elif defined(ANDROID)
            std::string temp = MAT::GetTempDirectory();
            std::string parent = temp + PATH_SEPARATOR_CHAR + "valid";
            validPath = parent + PATH_SEPARATOR_CHAR + "path" + PATH_SEPARATOR_CHAR;
            mkdir(parent.c_str(), 0777);
            mkdir(validPath.c_str(), 0777);
    #else
            mkdir("valid", 0777);
            mkdir(validPath.c_str(), 0777);
    #endif
        }

        void TearDown() override
        {
            PAL::detail::log_done();
            if (!PAL::detail::getDebugLogPath().empty())
            {
                std::remove(PAL::detail::getDebugLogPath().c_str());
            }

            // Remove the valid path directory
    #if defined(_WIN32) || defined(_WIN64)
            RemoveDirectoryA(validPath.c_str());
            RemoveDirectoryA("valid");
    #elif defined(ANDROID)
            rmdir(validPath.c_str());
            std::string temp = MAT::GetTempDirectory();
            std::string parent = temp + PATH_SEPARATOR_CHAR + "valid";
            rmdir(parent.c_str());
    #else
            rmdir(validPath.c_str());
            rmdir("valid");
    #endif
        }
};

TEST_F(LogInitTest, LogInitDisabled)
{
    EXPECT_FALSE(log_init(false, validPath));
}

TEST_F(LogInitTest, LogInitValidPath)
{
    EXPECT_TRUE(PAL::detail::log_init(true, validPath));
    EXPECT_TRUE(PAL::detail::getDebugLogStream()->is_open());
}

TEST_F(LogInitTest, LogInitParentDirectoryInvalidPath)
{
    EXPECT_FALSE(PAL::detail::log_init(true, "invalid/../path/"));
}

TEST_F(LogInitTest, LogInitPathDoesNotExist)
{
    EXPECT_FALSE(PAL::detail::log_init(true, "nonexistent/path/"));
}

TEST_F(LogInitTest, LogInitAlreadyInitialized)
{
    EXPECT_TRUE(PAL::detail::log_init(true, validPath));
    EXPECT_TRUE(PAL::detail::getDebugLogStream()->is_open());
    EXPECT_TRUE(PAL::detail::log_init(true, validPath)); // Should return true as it's already initialized
}

TEST_F(LogInitTest, LogInitPathWithoutTrailingSlash)
{
#if defined(ANDROID)
    std::string pathWithoutSlash = validPath;
    pathWithoutSlash.pop_back();
#else
    std::string pathWithoutSlash = "valid/path";
#endif
    EXPECT_TRUE(PAL::detail::log_init(true, pathWithoutSlash));
    EXPECT_TRUE(PAL::detail::getDebugLogStream()->is_open());
}

TEST_F(LogInitTest, LogInitPathWithoutTrailingBackslash)
{
#if defined(_WIN32) || defined(_WIN64)
    std::string pathWithoutBackslash = "valid\\path";
#elif defined(ANDROID)
    std::string pathWithoutBackslash = MAT::GetTempDirectory() + PATH_SEPARATOR_CHAR + "valid//path";
#else
    std::string pathWithoutBackslash = "valid//path";
#endif
    EXPECT_TRUE(PAL::detail::log_init(true, pathWithoutBackslash));
    EXPECT_TRUE(PAL::detail::getDebugLogStream()->is_open());
}

TEST_F(LogInitTest, OlderEpochCannotCloseCurrentLog)
{
    uint64_t oldEpoch = 0;
    uint64_t currentEpoch = 0;
    ASSERT_TRUE(log_init(true, validPath, oldEpoch));
    ASSERT_TRUE(log_init(true, validPath, currentEpoch));
    EXPECT_NE(oldEpoch, currentEpoch);

    log_done(oldEpoch);
    ASSERT_TRUE(getDebugLogStream());
    EXPECT_TRUE(getDebugLogStream()->is_open());
    log_done(currentEpoch);
    EXPECT_FALSE(getDebugLogStream());
}

TEST_F(LogInitTest, PalGenerationSurvivesOlderShutdown)
{
    MAT::ILogConfiguration firstLogConfiguration;
    firstLogConfiguration[CFG_BOOL_ENABLE_TRACE] = true;
    firstLogConfiguration[CFG_STR_TRACE_FOLDER_PATH] = validPath;
    MAT::RuntimeConfig_Default firstConfiguration(firstLogConfiguration);
    PAL::PlatformAbstractionLayer pal;
    pal.initialize(firstConfiguration);
    ASSERT_TRUE(getDebugLogStream());
    const std::string oldPath = getDebugLogPath();

    auto dispatcher = pal.getDefaultTaskDispatcher();
    std::promise<void> started;
    std::promise<void> release;
    auto startedFuture = started.get_future();
    BlockingTask task(started, release.get_future().share());
    PAL::dispatchTask(dispatcher.get(), &task, &BlockingTask::Run);
    if (startedFuture.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    {
        release.set_value();
        dispatcher.reset();
        pal.shutdown();
        FAIL() << "The worker did not start";
    }
    dispatcher.reset();

    std::thread shutdown([&] { pal.shutdown(); });
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (PAL::PALTest::HasDispatcher(pal) && std::chrono::steady_clock::now() < deadline)
    {
        std::this_thread::yield();
    }
    if (PAL::PALTest::HasDispatcher(pal))
    {
        release.set_value();
        shutdown.join();
        FAIL() << "The old dispatcher was not retired";
    }

    const auto separator = validPath.find_last_of("/\\", validPath.size() - 2);
    const std::string otherDirectory = validPath.substr(0, separator + 1);
    MAT::ILogConfiguration secondLogConfiguration;
    secondLogConfiguration[CFG_BOOL_ENABLE_TRACE] = true;
    secondLogConfiguration[CFG_STR_TRACE_FOLDER_PATH] = otherDirectory;
    MAT::RuntimeConfig_Default secondConfiguration(secondLogConfiguration);
    pal.initialize(secondConfiguration);
    const std::string newPath = getDebugLogPath();
    release.set_value();
    shutdown.join();

    EXPECT_NE(newPath, oldPath);
    ASSERT_TRUE(getDebugLogStream());
    EXPECT_TRUE(getDebugLogStream()->is_open());
    EXPECT_EQ(getDebugLogPath(), newPath);
    pal.shutdown();
    std::remove(oldPath.c_str());
}

TEST_F(LogInitTest, NewEpochUsesItsConfiguredPath)
{
    uint64_t oldEpoch = 0;
    uint64_t currentEpoch = 0;
    ASSERT_TRUE(log_init(true, validPath, oldEpoch));
    const std::string oldPath = getDebugLogPath();
    const auto separator = validPath.find_last_of("/\\", validPath.size() - 2);
    ASSERT_NE(separator, std::string::npos);
    const std::string otherDirectory = validPath.substr(0, separator + 1);

    const bool opened = log_init(true, otherDirectory, currentEpoch);
    EXPECT_TRUE(opened);
    if (opened)
    {
        EXPECT_NE(getDebugLogPath(), oldPath);
        log_done(oldEpoch);
        EXPECT_TRUE(getDebugLogStream());
    }
    std::remove(oldPath.c_str());
}

TEST_F(LogInitTest, FailedOpenIsNotReusedAsSuccessfulLogging)
{
    uint64_t epoch = 0;
    ASSERT_TRUE(log_init(true, validPath, epoch));
    const std::string blockedPath = getDebugLogPath();
    log_done(epoch);
    ASSERT_EQ(std::remove(blockedPath.c_str()), 0);
#if defined(_WIN32) || defined(_WIN64)
    ASSERT_TRUE(CreateDirectoryA(blockedPath.c_str(), nullptr));
#else
    ASSERT_EQ(mkdir(blockedPath.c_str(), 0777), 0);
#endif

    uint64_t firstEpoch = 0;
    uint64_t secondEpoch = 0;
    EXPECT_FALSE(log_init(true, validPath, firstEpoch));
    EXPECT_FALSE(log_init(true, validPath, secondEpoch));
    EXPECT_EQ(firstEpoch, 0u);
    EXPECT_EQ(secondEpoch, 0u);
    log_done();
#if defined(_WIN32) || defined(_WIN64)
    EXPECT_TRUE(RemoveDirectoryA(blockedPath.c_str()));
#else
    EXPECT_EQ(rmdir(blockedPath.c_str()), 0);
#endif
}

#endif
