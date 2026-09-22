//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
// clang-format off
#include "pal/WorkerThread.hpp"
#include "pal/PAL.hpp"
#include "ctmacros.hpp"

#include <exception>
#include <system_error>
#include <atomic>
#include <limits>

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
        // The worker thread's own id, captured under m_lock once threadFunc starts.
        // onLastReferenceReleased() reads it (under m_lock) rather than m_hThread.get_id()
        // to detect "am I running on my own worker thread?", because m_hThread.get_id()
        // returns the default not-a-thread id after a detach() -- so this keeps
        // self-dispose detection correct even if the thread was detached first. A plain
        // std::thread::id guarded by m_lock is used rather than std::atomic<std::thread::id>,
        // which is not portable (std::thread::id is not guaranteed trivially copyable).
        std::thread::id       m_workerId;

        std::recursive_mutex  m_lock;
        std::timed_mutex      m_execution_mutex;

        std::list<MAT::Task*> m_queue;
        std::list<MAT::Task*> m_timerQueue;
        Event                 m_event;
        MAT::Task*            m_itemInProgress = nullptr;
        uint64_t              m_itemInProgressGeneration = 0;
        bool                  m_itemCancellationRequested = false;
        int count = 0;
        bool                  m_shuttingDown = false;
        std::mutex            m_joinLock;
        // Set when the last reference is released by a task running on this worker
        // thread, so threadFunc performs the final delete after its loop breaks
        // (see onLastReferenceReleased() and WorkerThreadFactory::Create()).
        std::atomic<bool>     m_disposeFromThread { false };

    public:

        WorkerThread()
        {
            m_hThread = std::thread(WorkerThread::threadFunc, static_cast<void*>(this));
            LOG_INFO("Started new thread %zu", std::hash<std::thread::id>{}(m_hThread.get_id()));
        }

        ~WorkerThread()
        {
            Join();
        }

    private:
        void enqueueShutdownItemLocked()
        {
            if (!m_shuttingDown) {
                m_shuttingDown = true;
                m_queue.push_back(new WorkerThreadShutdownItem());
                m_event.post();
            }
        }

        void drainPendingTasks()
        {
            std::list<MAT::Task*> queue;
            std::list<MAT::Task*> timerQueue;
            {
                LOCKGUARD(m_lock);
                queue.splice(queue.end(), m_queue);
                timerQueue.splice(timerQueue.end(), m_timerQueue);
            }
            if (!queue.empty()) {
                LOG_WARN("Shutdown with %zu queued task(s) pending", queue.size());
            }
            if (!timerQueue.empty()) {
                LOG_WARN("Shutdown with %zu timer(s) pending", timerQueue.size());
            }
            for (auto task : queue) { delete task; }
            for (auto task : timerQueue) { delete task; }
        }

    public:
        void Join() final
        {
            std::thread::id this_id = std::this_thread::get_id();
            {
                LOCKGUARD(m_lock);
                if (m_workerId == this_id)
                {
                    enqueueShutdownItemLocked();
                    return;
                }
            }

            LOCKGUARD(m_joinLock);
            std::thread threadToJoin;
            bool joined = false;
            {
                LOCKGUARD(m_lock);
                enqueueShutdownItemLocked();
                if (!m_hThread.joinable()) {
                    return;
                }
                threadToJoin = std::move(m_hThread);
            }
#if HAVE_EXCEPTIONS
            try {
                if (threadToJoin.joinable()) {
                    threadToJoin.join();
                    joined = true;
                }
            }
            catch (const std::system_error& e) {
                (void)e;
                LOG_ERROR("Thread join/detach failed: [%d] %s", e.code().value(), e.what());
                std::terminate();
            }
            catch (const std::exception& e) {
                (void)e;
                LOG_ERROR("Thread join/detach failed: %s", e.what());
                std::terminate();
            }
#else
            if (threadToJoin.joinable()) {
                threadToJoin.join();
                joined = true;
            }
#endif

            // Clean up any tasks remaining in the queues after shutdown.
            // Only safe after join() — the thread has fully exited.
            if (joined) {
                drainPendingTasks();
            }
        }

        // Invoked by the shared_ptr deleter when the last reference is released.
        // Returns true if the caller should delete the object, false if deletion was
        // deferred to the worker thread. The worker is shared process-wide, so the
        // last reference can be dropped by a task running on the worker thread itself
        // (e.g. a task that tears down its LogManager/PAL). In that case threadFunc is
        // still on the stack below the task and keeps touching members after the task
        // returns, so freeing the object here would be a use-after-free: instead
        // detach, signal shutdown, mark the thread to delete itself once its loop
        // breaks, and leave the object alive. On any other thread it is safe to delete
        // immediately (~WorkerThread joins the worker first).
        bool onLastReferenceReleased()
        {
            LOCKGUARD(m_lock);
            if (m_workerId == std::this_thread::get_id())
            {
                enqueueShutdownItemLocked();
                m_disposeFromThread.store(true, std::memory_order_release);
#if HAVE_EXCEPTIONS
                try {
                    if (m_hThread.joinable()) {
                        m_hThread.detach();
                    }
                }
                catch (const std::exception& e) {
                    (void)e;
                    LOG_ERROR("Worker self-detach failed: %s", e.what());
                }
#else
                if (m_hThread.joinable()) {
                    m_hThread.detach();
                }
#endif
                return false;
            }
            return true;
        }

        void Queue(MAT::Task* item) final
        {
            LOG_INFO("queue item=%p", static_cast<void*>(item));
            bool rejected = false;
            {
                LOCKGUARD(m_lock);
                if (m_shuttingDown) {
                    rejected = true;
                }
                else if (item->Type == MAT::Task::TimedCall) {
                    auto it = m_timerQueue.begin();
                    while (it != m_timerQueue.end() && (*it)->TargetTime < item->TargetTime) {
                        ++it;
                    }
                    m_timerQueue.insert(it, item);
                }
                else {
                    m_queue.push_back(item);
                }
            }
            if (rejected) {
                LOG_WARN("Dropping queued task %p during shutdown", static_cast<void*>(item));
                delete item;
                return;
            }
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
                if (m_workerId == std::this_thread::get_id())
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

                bool completed = false;
                if (waitTime == std::numeric_limits<uint64_t>::max())
                {
                    m_execution_mutex.lock();
                    completed = true;
                }
                else
                {
                    completed =
                        m_execution_mutex.try_lock_for(std::chrono::milliseconds(waitTime));
                }
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
            {
                LOCKGUARD(self->m_lock);
                self->m_workerId = std::this_thread::get_id();
            }
            LOG_INFO("Running thread %zu", std::hash<std::thread::id>{}(std::this_thread::get_id()));

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
                    // Drop any tasks still queued behind the shutdown sentinel
                    // (e.g. future-dated timers) before exiting. The owning thread
                    // deletes these in Join() only after a successful join(); on the
                    // self-Join path it detaches and skips that cleanup, so draining
                    // here prevents leaking those tasks. This matches the join()-path
                    // behavior of dropping un-run work at shutdown.
                    self->drainPendingTasks();
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
                        MATSDK_TRY {
                            (*item)();
                        }
#if HAVE_EXCEPTIONS
                        MATSDK_CATCH(const std::exception& ex) {
                            (void)ex;
                            LOG_ERROR("Unhandled exception in worker task: %s", ex.what());
                        }
                        MATSDK_CATCH(...) {
                            LOG_ERROR("Unhandled non-standard exception in worker task");
                        }
#endif
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

            // The loop has broken on a Shutdown item. If the last reference was
            // released by a task on this worker thread, onLastReferenceReleased()
            // detached and deferred deletion to us; perform it now, after all member
            // access is done, so the object outlives threadFunc rather than being
            // freed underneath it.
            if (self->m_disposeFromThread.load(std::memory_order_acquire)) {
                delete self;
            }
        }
    };

    namespace WorkerThreadFactory {
        std::shared_ptr<ITaskDispatcher> Create()
        {
            // Custom deleter so that a last-reference release happening on the worker
            // thread itself defers destruction to the thread (see
            // onLastReferenceReleased) instead of freeing the object underneath a
            // still-running threadFunc.
            return std::shared_ptr<WorkerThread>(
                new WorkerThread(),
                [](WorkerThread* self) { if (self->onLastReferenceReleased()) delete self; });
        }
    }

} PAL_NS_END

#endif
