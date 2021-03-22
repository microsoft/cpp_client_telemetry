//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef WORKER_THREAD_HPP
#define WORKER_THREAD_HPP

#include <functional>
#include <list>
#include <mutex>
#include <stdint.h>
#include <string>
#include <thread>
#include <condition_variable>
#include <climits>
#include <algorithm>
#include <memory>

#include "ITaskDispatcher.hpp"
#include "ctmacros.hpp"

namespace PAL_NS_BEGIN {

    // Event (binary semaphore)
    class Event
    {
    protected:
        bool m_bFlag;
        mutable std::mutex m_mutex;
        mutable std::condition_variable m_condition;

    public:

        inline Event() : m_bFlag(false) {}

        ~Event()
        {
            Reset();
        }

        bool wait(unsigned millis = UINT_MAX) const
        {
            if (millis == UINT_MAX)
            {
                std::unique_lock< std::mutex > lock(m_mutex);
                m_condition.wait(lock, [&]()->bool {return m_bFlag; });
                return true;
            }

            auto crRelTime = std::chrono::milliseconds(millis);
            std::unique_lock<std::mutex> ulock(m_mutex);
            if (!m_condition.wait_for(ulock, crRelTime, [&]()->bool {return m_bFlag; }))
                return false;
            return true;
        }

        inline bool post()
        {
            bool bWasSignalled;
            m_mutex.lock();
            bWasSignalled = m_bFlag;
            m_bFlag = true;
            m_mutex.unlock();
            m_condition.notify_all();
            return bWasSignalled == false;
        }

        inline bool Reset()
        {
            bool bWasSignalled;
            m_mutex.lock();
            bWasSignalled = m_bFlag;
            m_bFlag = false;
            m_mutex.unlock();
            return bWasSignalled;
        }

        inline bool IsSet() const { return m_bFlag; }
    };

    namespace WorkerThreadFactory {
        std::shared_ptr<MAT::ITaskDispatcher> Create();
    }

} PAL_NS_END

#endif

