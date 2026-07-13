//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "pal/PseudoRandomGenerator.hpp"
#include "pal/TaskDispatcher.hpp"
#include "pal/WorkerThread.hpp"
#include "Version.hpp"

#include <atomic>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <set>
#include <functional>
#include <memory>

#ifdef HAVE_MAT_LOGGING
#include "pal/PAL.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <memory>
#include <string>
#include <cstdio>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#endif
using namespace PAL::detail;
#endif

using namespace testing;

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

class PalTests : public Test {};

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
 /*   MockISemanticContext context;


    EXPECT_CALL(context, SetAppId(Not(IsEmpty()))).WillOnce(DoDefault());
    EXPECT_CALL(context, SetAppVersion(_)).WillOnce(DoDefault());
    EXPECT_CALL(context, SetAppLanguage(_)).WillOnce(DoDefault());

    EXPECT_CALL(context, SetUserLanguage(_)).WillOnce(DoDefault());
    EXPECT_CALL(context, SetUserTimeZone(Not(IsEmpty()))).WillOnce(DoDefault());
    //EXPECT_CALL(context, SetUserAdvertisingId(_)).WillOnce(DoDefault());

   
    PAL::registerSemanticContext(&context);

    PAL::sleep(500);

    PAL::unregisterSemanticContext(&context);
    */
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

    class WorkerThreadScheduleTarget
    {
    public:
        void Callback() {}
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

TEST_F(PalTests, ScheduleTaskAfterWorkerThreadJoinReturnsNoOpHandle)
{
    auto dispatcher = PAL::WorkerThreadFactory::Create();
    dispatcher->Join();
    WorkerThreadScheduleTarget target;

    auto handle = PAL::scheduleTask(dispatcher.get(), 100, &target, &WorkerThreadScheduleTarget::Callback);

    EXPECT_EQ(handle.m_task, nullptr);
    EXPECT_TRUE(handle.Cancel());
}

namespace
{
    // Runs on the worker thread and releases the last reference to the dispatcher
    // that owns this very thread, exercising the self-dispose path.
    class SelfDisposeHelper
    {
    public:
        std::function<void()> releaseLastRef;
        std::atomic<bool>* done = nullptr;
        void Run()
        {
            releaseLastRef();      // drops the last dispatcher reference on its own thread
            done->store(true);
        }
    };
}

// The process-wide worker is shared by reference count, and a task can drop the last
// reference from within itself (e.g. by tearing down its LogManager/PAL) while running
// ON the worker thread. The worker must not be freed underneath its own still-running
// threadFunc: it detaches and defers destruction to the thread. This exercises that
// path and must not use-after-free (caught by ASAN).
TEST_F(PalTests, WorkerThreadSelfDisposeOnOwnThreadIsSafe)
{
    auto dispatcher = PAL::WorkerThreadFactory::Create();
    auto* raw = dispatcher.get();
    // 'box' holds the only remaining reference; the task releases it on the worker
    // thread. Keep it in a shared box so a copy captured by the task's callable can
    // reset it without naming the dispatcher's concrete type.
    auto box = std::make_shared<decltype(dispatcher)>(std::move(dispatcher));

    std::atomic<bool> done(false);
    SelfDisposeHelper helper;
    helper.releaseLastRef = [box]() { box->reset(); };
    helper.done = &done;

    PAL::dispatchTask(raw, &helper, &SelfDisposeHelper::Run);

    for (int i = 0; i < 500 && !done.load(); ++i)
        PAL::sleep(10);
    ASSERT_TRUE(done.load());

    // Give the worker time to break its loop and delete itself after the task
    // returns. Reaching here without a crash / ASAN report means the object was not
    // freed underneath its own threadFunc.
    PAL::sleep(200);
}

#ifdef HAVE_MAT_LOGGING
class LogInitTest : public Test
{
    protected:
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

#endif
