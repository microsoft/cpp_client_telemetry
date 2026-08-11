//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "pal/TaskDispatcher_CAPI.hpp"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <exception>
#include <limits>
#include <memory>
#include <mutex>
#include <map>
#include <sstream>
#include <thread>

#include "ctmacros.hpp"
#include "pal/PAL.hpp"

using namespace MAT;

namespace PAL_NS_BEGIN {

    class Task_CAPI
    {
    public:
        Task_CAPI(std::unique_ptr<Task> task)
          : m_task(std::move(task)) {}

        ~Task_CAPI() noexcept
        {
            ReleaseItem();
        }

        Task* GetTask()
        {
            std::lock_guard<std::mutex> lock(m_stateLock);
            return m_task.get();
        }

        bool BeginCallback()
        {
            std::lock_guard<std::mutex> lock(m_stateLock);
            if (m_done || m_cancelled)
            {
                return false;
            }
            m_running = true;
            m_callbackThread = std::this_thread::get_id();
            return true;
        }

        void OnCallback()
        {
            if (!BeginCallback())
            {
                return;
            }
            if (m_task) {
                // The task is host/user code running on the external dispatcher's
                // thread; an exception escaping here would terminate the process.
                // Log it (mirroring WorkerThread) instead of swallowing silently.
                try {
                    (*m_task)();
                }
                catch (const std::exception& ex) {
                    (void)ex;
                    LOG_ERROR("Unhandled exception in CAPI task: %s", ex.what());
                }
                catch (...) {
                    LOG_ERROR("Unhandled non-standard exception in CAPI task");
                }
            }
            {
                std::lock_guard<std::mutex> lock(m_stateLock);
                ReleaseItem();
                m_running = false;
                m_done = true;
            }
            m_doneCv.notify_all();
        }

        bool RequestCancel()
        {
            std::lock_guard<std::mutex> lock(m_stateLock);
            if (m_done)
            {
                return false;
            }
            m_cancelled = true;
            if (!m_running)
            {
                m_done = true;
                m_doneCv.notify_all();
            }
            return m_running;
        }

        bool WaitForCompletion(uint64_t waitTime)
        {
            std::unique_lock<std::mutex> lock(m_stateLock);
            if (m_done || m_callbackThread == std::this_thread::get_id())
            {
                return true;
            }
            if (waitTime == std::numeric_limits<uint64_t>::max())
            {
                m_doneCv.wait(lock, [this] { return m_done; });
            }
            else if (waitTime > 0)
            {
                m_doneCv.wait_for(lock, std::chrono::milliseconds(waitTime), [this] { return m_done; });
            }
            return m_done;
        }

    private:
        void ReleaseItem()
        {
            if (m_task) {
                m_task->Type = Task::Done;
                m_task = nullptr;
            }
        }

        std::unique_ptr<Task> m_task;
        std::mutex m_stateLock;
        std::condition_variable m_doneCv;
        std::thread::id m_callbackThread;
        bool m_running = false;
        bool m_done = false;
        bool m_cancelled = false;
    };


    static std::mutex s_tasksLock;

    std::map<std::string, std::shared_ptr<Task_CAPI>>& GetPendingTasks() {
      static std::map<std::string, std::shared_ptr<Task_CAPI>> s_tasks;
      return s_tasks;
    }

    std::string GetNextTaskId() {
        static std::atomic<int32_t> s_nextTaskId(0);
        std::ostringstream idStream;
        idStream << "OneDS_Task-" << s_nextTaskId++;
        return idStream.str();
    }

    void EVTSDK_LIBABI_CDECL OnAsyncTaskCallback(const char* taskId)
    {
        std::shared_ptr<Task_CAPI> task;

        // Find and remove pending task
        {
            LOCKGUARD(s_tasksLock);
            auto itTask = GetPendingTasks().find(taskId);
            if (itTask != GetPendingTasks().end()) {
                task = itTask->second;
                GetPendingTasks().erase(itTask);
            }
        }

        if (task)
        {
            task->OnCallback();
            LOCKGUARD(s_tasksLock);
            auto itTask = GetPendingTasks().find(taskId);
            if (itTask != GetPendingTasks().end() && itTask->second == task)
            {
                GetPendingTasks().erase(itTask);
            }
        }
    }

    TaskDispatcher_CAPI::TaskDispatcher_CAPI(task_dispatcher_queue_fn_t queueFn, task_dispatcher_cancel_fn_t cancelFn, task_dispatcher_join_fn_t joinFn)
      : m_queueFn(queueFn),
        m_cancelFn(cancelFn),
        m_joinFn(joinFn)
    {
        if ((queueFn == nullptr) || (cancelFn == nullptr) || (joinFn == nullptr))
        {
            MATSDK_THROW(std::invalid_argument("Created TaskDispatcher_CAPI with invalid parameters"));
        }
    }

    void TaskDispatcher_CAPI::Join()
    {
        m_joinFn();
    }

    void TaskDispatcher_CAPI::Queue(Task* task)
    {
        if (task->Type != Task::Call && task->Type != Task::TimedCall)
            return;

        auto ownedItem = std::unique_ptr<Task>(task);

        // Create task
        evt_task_t capiTask;
        std::string taskId = GetNextTaskId();
        capiTask.id = taskId.c_str();
        capiTask.typeName = ownedItem->TypeName.c_str();
        capiTask.delayMs = 0;
        if (ownedItem->Type == Task::TimedCall) {
            capiTask.delayMs = ownedItem->TargetTime - getMonotonicTimeMs();
        }

        // Add pending task
        {
            LOCKGUARD(s_tasksLock);
            GetPendingTasks()[capiTask.id] = std::make_shared<Task_CAPI>(std::move(ownedItem));
        }

        m_queueFn(&capiTask, &OnAsyncTaskCallback);
    }

    bool TaskDispatcher_CAPI::Cancel(Task* task, uint64_t waitTime)
    {
        std::string taskId;
        std::shared_ptr<Task_CAPI> capiTask;

        // Find and erase pending task
        {
            LOCKGUARD(s_tasksLock);
            auto itTask = std::find_if(GetPendingTasks().begin(), GetPendingTasks().end(),
                    [task](const std::pair<std::string, std::shared_ptr<Task_CAPI>>& capiTask) {
                return capiTask.second->GetTask() == task;
            });

            if (itTask != GetPendingTasks().end()) {
                taskId = itTask->first;
                capiTask = itTask->second;
            }
        }

        if (taskId.empty())
        {
            return false;
        }

        const bool wasRunning = capiTask->RequestCancel();
        m_cancelFn(taskId.c_str());
        if (!wasRunning)
        {
            LOCKGUARD(s_tasksLock);
            GetPendingTasks().erase(taskId);
            return true;
        }

        if (capiTask->WaitForCompletion(waitTime))
        {
            LOCKGUARD(s_tasksLock);
            GetPendingTasks().erase(taskId);
            return true;
        }

        return false;
    }

} PAL_NS_END
