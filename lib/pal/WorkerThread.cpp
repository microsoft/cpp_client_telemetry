//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
// clang-format off
#include "pal/WorkerThread.hpp"
#include "pal/PAL.hpp"

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

        // Cancel a task or wait for task completion for up to waitTime ms:
        //
        // - acquire the m_lock to prevent a new task from getting scheduled.
        //   This may block the scheduling of a new task in queue for up to
        //   waitTime in case if the task being canceled
        //   is the one being executed right now.
        //
        // - if currently executing task is the one we are trying to cancel,
        //   then verify for recursion: if the current thread is the same
        //   we're waiting on, prevent the recursion (we can't cancel our own
        //   thread task). If it's different thread, then idle-poll-wait for
        //   task completion for up to waitTime ms. m_itemInProgress is nullptr
        //   once the item is done executing. Method may fail and return if
        //   waitTime given was insufficient to wait for completion.
        //
        // - if task being cancelled is not executing yet, then erase it from
        //   timer queue without any wait.
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
            LOCKGUARD(m_lock);
            if (item == nullptr)
            {
                return false;
            }

            if (m_itemInProgress == item)
            {
                /* Can't recursively wait on completion of our own thread */
                if (m_hThread.get_id() != std::this_thread::get_id())
                {
                    if (waitTime > 0 && m_execution_mutex.try_lock_for(std::chrono::milliseconds(waitTime)))
                    {
                        m_itemInProgress = nullptr;
                        m_execution_mutex.unlock();
                    }
                }
                else
                {
                    // The SDK may attempt to cancel itself from within its own task.
                    // Return true and assume that the current task will finish, and therefore be cancelled.
                    return true;
                }

                /* Either waited long enough or the task is still executing. Return:
                 *  true    - if item in progress is different than item (other task)
                 *  false   - if item in progress is still the same (didn't wait long enough)
                 */
                return (m_itemInProgress != item);
            }

            {
                auto it = std::find(m_timerQueue.begin(), m_timerQueue.end(), item);
                if (it != m_timerQueue.end()) {
                    // Still in the queue
                    m_timerQueue.erase(it);
                    delete item;
                }
            }
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
                    }
                }

                if (!item) {
                    if (!self->m_event.Reset())
                        self->m_event.wait(nextTimerInMs);
                    continue;
                }

                if (item->Type == MAT::Task::Shutdown) {
                    item.reset();
                    self->m_itemInProgress = nullptr;
                    break;
                }

                {
                    std::lock_guard<std::timed_mutex> lock(self->m_execution_mutex);

                    // Item wasn't cancelled before it could be executed
                    if (self->m_itemInProgress != nullptr) {
                        LOG_TRACE("%10llu Execute item=%p type=%s\n", wakeupCount, item.get(), item.get()->TypeName.c_str() );
                        (*item)();
                        self->m_itemInProgress = nullptr;
                    }

                    if (item) {
                        item->Type = MAT::Task::Done;
                        item = nullptr;
                    }
                }
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

