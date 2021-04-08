//
// Copyright (c) 2015-2021 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "PauseManager.hpp"

#include <chrono>
#include <future>
#include <thread>

using namespace testing;
using namespace MAT;

class PauseManagerTests : public StrictMock<Test> {
  public:

};

TEST_F(PauseManagerTests, InitializedAsRunning)
{
    PauseManager::Lock lock;
    EXPECT_THAT(lock.isPaused(), false);
}

TEST_F(PauseManagerTests, CanPause)
{
    PauseManager::PauseActivity();
    PauseManager::Lock lock;
    EXPECT_THAT(lock.isPaused(), true);
    PauseManager::ResumeActivity();
}

TEST_F(PauseManagerTests, WaitReturnsIfRunning)
{
    PauseManager::Lock lock;
    EXPECT_THAT(lock.isPaused(), false);
    PauseManager::QuiesceWait();
}

TEST_F(PauseManagerTests, WaitReturnsIfPaused)
{
    PauseManager::PauseActivity();
    PauseManager::Lock lock;
    EXPECT_THAT(lock.isPaused(), true);
    PauseManager::QuiesceWait();
    PauseManager::ResumeActivity();
}

TEST_F(PauseManagerTests, WaitWaitsIfPausing)
{
    std::unique_ptr<PauseManager::Lock> lock = std::make_unique<PauseManager::Lock>();
    EXPECT_THAT(lock->isPaused(), false);
    PauseManager::PauseActivity();
    auto hasPaused = std::async(std::launch::async,
        [] () -> void {
            PauseManager::QuiesceWait();
        }
    );
    auto status = hasPaused.wait_for(std::chrono::milliseconds(0));
    EXPECT_THAT(status, std::future_status::timeout);
    lock.reset();
    auto pausedStatus = hasPaused.wait_for(std::chrono::seconds(1));
    EXPECT_THAT(pausedStatus, std::future_status::ready);
    hasPaused.get();
    PauseManager::ResumeActivity();
}

TEST_F(PauseManagerTests, WaitWaitsUntilResume)
{
    PauseManager::Lock lock;
    EXPECT_THAT(lock.isPaused(), false);
    PauseManager::PauseActivity();
    auto hasPaused = std::async(std::launch::async,
        [] () -> void {
            PauseManager::QuiesceWait();
        }
    );
    auto status = hasPaused.wait_for(std::chrono::milliseconds(0));
    EXPECT_THAT(status, std::future_status::timeout);
    PauseManager::ResumeActivity();
    auto resumedStatus = hasPaused.wait_for(std::chrono::seconds(1));
    EXPECT_THAT(resumedStatus, std::future_status::ready);
    hasPaused.get();
    PauseManager::Lock anotherLock;
    EXPECT_THAT(anotherLock.isPaused(), false);
}

TEST_F(PauseManagerTests, WaitForCompletes)
{
    std::unique_ptr<PauseManager::Lock> lock = std::make_unique<PauseManager::Lock>();
    EXPECT_THAT(lock->isPaused(), false);
    PauseManager::PauseActivity();
    auto hasPaused = std::async(std::launch::async,
        [] () -> void {
            PauseManager::QuiesceWaitFor(std::chrono::seconds(1));
        }
    );
    lock.reset();
    auto status = hasPaused.wait_for(std::chrono::milliseconds(75));
    EXPECT_THAT(status, std::future_status::ready);
    hasPaused.get();
    PauseManager::ResumeActivity();
}

TEST_F(PauseManagerTests, WaitForTimesOut)
{
    std::unique_ptr<PauseManager::Lock> lock = std::make_unique<PauseManager::Lock>();
    EXPECT_THAT(lock->isPaused(), false);
    PauseManager::PauseActivity();
    auto hasPaused = std::async(std::launch::async,
        [] () -> void {
            PauseManager::QuiesceWaitFor(std::chrono::milliseconds(50));
        }
    );
    auto status = hasPaused.wait_for(std::chrono::milliseconds(0));
    EXPECT_THAT(status, std::future_status::timeout);
    auto timedOut = hasPaused.wait_for(std::chrono::milliseconds(100));
    EXPECT_THAT(timedOut, std::future_status::ready);
    hasPaused.get();
    PauseManager::ResumeActivity();
}
