//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
// clang-format off
#include "pal/WorkerThread.hpp"
#include "pal/PAL.hpp"

#include <exception>

#if defined(MATSDK_PAL_CPP11) || defined(MATSDK_PAL_WIN32)

/* Maximum scheduler interval for SDK is 1 hour required for clamping in case of monotonic clock drift */
#define MAX_FUTURE_DELTA_MS (60 * 60 * 1000)

namespace PAL_NS_BEGIN {

    class WorkerThreadShutdownItem : public Task
    {
    public:
        WorkerThreadShutdownItem() :
            Task()
        {
            Type = MAT::Task::Shutdown;
        }
    };

    class WorkerThread : public ITaskDispatcher
    {
    protected:
        std::thread           m_hThread;

        std::recursive_mutex  m_lock;
        std::timed_mutex      m_execution_mutex;

        std::list<MAT::Task*> m_queue;
        std::list<MAT::Task*> m_timerQueue;
        Event                 m_event;
        MAT::Task*            m_itemInProgress;
        uint64_t              m_itemInProgressGeneration = 0;
        bool                  m_itemCancellationRequested = false;
        int count = 0;

    public:

        WorkerThread()
        {
            m_itemInProgress = nullptr;
            m_hThread = std::thread(WorkerThread::threadFunc, static_cast<void*>(this));
            LOG_INFO("Started new thread %u", m_hThread.get_id());
        }

        ~WorkerThread()
        {
            Join();
        }

        void Join() final
        {
            auto item = new WorkerThreadShutdownItem();
            Queue(item);
            std::thread::id this_id = std::this_thread::get_id();
            try {
                if (m_hThread.joinable() && (m_hThread.get_id() != this_id))
                    m_hThread.join();
                else
                    m_hThread.detach();
            }
            catch (...) {};

            // TODO: [MG] - investigate if we ever drop work items on shutdown.
            if (!m_queue.empty())
            {
                LOG_WARN("m_queue is not empty!");
            }
            if (!m_timerQueue.empty())
            {
                LOG_WARN("m_timerQueue is not empty!");
            }
        }

        void Queue(MAT::Task* item) final
        {
            LOG_INFO("queue item=%p", &item);
            LOCKGUARD(m_lock);
            if (item->Type == MAT::Task::TimedCall) {
                auto it = m_timerQueue.begin();
                while (it != m_timerQueue.end() && (*it)->TargetTime < item->TargetTime) {
                    ++it;
                }
                m_timerQueue.insert(it, item);
            }
            else {
                m_queue.push_back(item);
            }
            count++;
            m_event.post();
        }

        // Lock rule: never wait for m_execution_mutex while holding m_lock.
        // Task callbacks may call Queue(), which needs m_lock while the callback
        // owns m_execution_mutex.
        //
        // TODO: current callers of this API do not check the status code.
        // Refactor this code to return the following cancellation status:
        // - TASK_NOTFOUND  - task not found
        // - TASK_CANCELLED - task found and cancelled without execution
        // - TASK_COMPLETED - task found and ran to completion
        // - TASK_RUNNING   - task is still running (insufficient waitTime)
        //
        bool Cancel(MAT::Task* item, uint64_t waitTime) override
        {
            MAT::Task* queuedItem = nullptr;
            std::unique_lock<std::recursive_mutex> lock(m_lock);
            if (item == nullptr)
            {
                return false;
            }

            if (m_itemInProgress == item)
            {
                /* Can't recursively wait on completion of our own thread */
                if (m_hThread.get_id() == std::this_thread::get_id())
                {
                    // The SDK may attempt to cancel itself from within its own task.
                    // Return true and assume that the current task will finish, and therefore be cancelled.
                    return true;
                }

                if (waitTime == 0)
                {
                    return false;
                }

                const uint64_t generation = m_itemInProgressGeneration;
                m_itemCancellationRequested = true;
                lock.unlock();

                const bool completed =
                    m_execution_mutex.try_lock_for(std::chrono::milliseconds(waitTime));
                if (completed)
                {
                    m_execution_mutex.unlock();
                }

                lock.lock();
                const bool sameItem =
                    m_itemInProgress == item &&
                    m_itemInProgressGeneration == generation;
                if (completed && sameItem)
                {
                    m_itemInProgress = nullptr;
                    m_itemCancellationRequested = false;
                }

                return completed || !sameItem;
            }

            auto it = std::find(m_timerQueue.begin(), m_timerQueue.end(), item);
            if (it != m_timerQueue.end()) {
                // Transfer ownership under m_lock, but destroy outside all worker locks.
                queuedItem = *it;
                m_timerQueue.erase(it);
            }
            lock.unlock();
            delete queuedItem;
#if 0
            for (;;) {
                {
                    LOCKGUARD(m_lock);
                    if (item->Type == MAT::Task::Done) {
                        return;
                    }
                }
                Sleep(10);
            }
#endif
            return true;
        }

    protected:
        static void threadFunc(void* lpThreadParameter)
        {
            uint64_t wakeupCount = 0;

            WorkerThread* self = reinterpret_cast<WorkerThread*>(lpThreadParameter);
            LOG_INFO("Running thread %u", std::this_thread::get_id());

            for (;;) {
                std::unique_ptr<MAT::Task> item = nullptr;
                wakeupCount++;
                unsigned nextTimerInMs = MAX_FUTURE_DELTA_MS;
                {
                    LOCKGUARD(self->m_lock);

                    auto now = getMonotonicTimeMs();
                    if (!self->m_timerQueue.empty()) {
                        const auto currTargetTime = self->m_timerQueue.front()->TargetTime;
                        if (currTargetTime <= now) {
                            // process the item at the front immediately
                            item = std::unique_ptr<MAT::Task>(self->m_timerQueue.front());
                            self->m_timerQueue.pop_front();
                        } else {
                           // timed call in future, we need to resort the items in the queue
                           const auto delta = currTargetTime - now;
                           if (delta > MAX_FUTURE_DELTA_MS) {
                               const auto itemPtr = self->m_timerQueue.front();
                               self->m_timerQueue.pop_front();
                               itemPtr->TargetTime = now + MAX_FUTURE_DELTA_MS;
                               self->Queue(itemPtr);
                               continue;
                           }
                           // value used for sleep in case if m_queue ends up being empty
                           nextTimerInMs = static_cast<unsigned>(delta);
                        }
                    }

                    if (!self->m_queue.empty() && !item) {
                        item = std::unique_ptr<MAT::Task>(self->m_queue.front());
                        self->m_queue.pop_front();
                    }

                    if (item) {
                        self->m_itemInProgress = item.get();
                        ++self->m_itemInProgressGeneration;
                        self->m_itemCancellationRequested = false;
                    }
                }

                if (!item) {
                    if (!self->m_event.Reset())
                        self->m_event.wait(nextTimerInMs);
                    continue;
                }

                if (item->Type == MAT::Task::Shutdown) {
                    {
                        LOCKGUARD(self->m_lock);
                        if (self->m_itemInProgress == item.get()) {
                            self->m_itemInProgress = nullptr;
                            self->m_itemCancellationRequested = false;
                        }
                    }
                    item.reset();
                    break;
                }

                {
                    std::lock_guard<std::timed_mutex> lock(self->m_execution_mutex);

                    bool executeItem = false;
                    {
                        LOCKGUARD(self->m_lock);
                        executeItem =
                            self->m_itemInProgress == item.get() &&
                            !self->m_itemCancellationRequested;
                    }

                    if (executeItem) {
                        LOG_TRACE("%10llu Execute item=%p type=%s\n", wakeupCount, item.get(), item.get()->TypeName.c_str() );
                        // A task can run arbitrary work (storage I/O, HTTP encode, and
                        // user DebugEventListener callbacks). An exception escaping here
                        // would unwind out of the thread entry function and call
                        // std::terminate, killing the host process. Contain it.
                        try {
                            (*item)();
                        }
                        catch (const std::exception& ex) {
                            UNREFERENCED_PARAMETER(ex);
                            LOG_ERROR("Unhandled exception in worker task: %s", ex.what());
                        }
                        catch (...) {
                            LOG_ERROR("Unhandled non-standard exception in worker task");
                        }
                    }

                    if (item) {
                        item->Type = MAT::Task::Done;
                    }
                }
                {
                    LOCKGUARD(self->m_lock);
                    if (self->m_itemInProgress == item.get()) {
                        self->m_itemInProgress = nullptr;
                        self->m_itemCancellationRequested = false;
                    }
                }
                // Task destruction may synchronize with a cancellation caller.
                // Never run it while holding m_execution_mutex, which Cancel()
                // waits on while that caller owns the task lifetime lock.
                item = nullptr;
            }
        }
    };

    namespace WorkerThreadFactory {
        std::shared_ptr<ITaskDispatcher> Create()
        {
            return std::make_shared<WorkerThread>();
        }
    }

} PAL_NS_END

#endif
