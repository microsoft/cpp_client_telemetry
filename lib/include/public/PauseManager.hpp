//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef PAUSEMANAGER_HPP
#define PAUSEMANAGER_HPP

#include <chrono>
#include <condition_variable>
#include <mutex>

#include "ctmacros.hpp"

namespace MAT_NS_BEGIN
{
    class PauseManager {
        public:

        static void PauseActivity() noexcept;
        static void ResumeActivity() noexcept;
        static void QuiesceWait() noexcept;
        static void QuiesceWaitFor(std::chrono::milliseconds timeout) noexcept;
        static bool TryLock() noexcept;
        static void ReleaseLock() noexcept;

        class Lock {
        public:

            Lock();
            ~Lock();

            bool isPaused() const noexcept;

        private:
            bool acquired_lock;
        };

        private:

        static std::mutex s_pause_manager_mutex;
        static std::atomic<uint64_t> s_pause_manager_state;
        static std::condition_variable s_pause_manager_cv;
    };
}
MAT_NS_END

#endif
