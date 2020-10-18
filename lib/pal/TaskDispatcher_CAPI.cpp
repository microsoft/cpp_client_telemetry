//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "pal/TaskDispatcher_CAPI.hpp"

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <map>
#include <sstream>

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
            return m_task.get();
        }

        void OnCallback()
        {
            if (m_task) {
                (*m_task)();
            }
            ReleaseItem();
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
            task->OnCallback();
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

    // TODO: currently shutdown wait on task cancellation is not implemented for C API Task Dispatcher
    bool TaskDispatcher_CAPI::Cancel(Task* task, uint64_t)
    {
        std::string taskId;

        // Find and erase pending task
        {
            LOCKGUARD(s_tasksLock);
            auto itTask = std::find_if(GetPendingTasks().begin(), GetPendingTasks().end(),
                    [task](const std::pair<std::string, std::shared_ptr<Task_CAPI>>& capiTask) {
                return capiTask.second->GetTask() == task;
            });

            if (itTask != GetPendingTasks().end()) {
                taskId = itTask->first;
                GetPendingTasks().erase(itTask);
            }
        }

        return (!taskId.empty()) ? m_cancelFn(taskId.c_str()) : false;
    }

} PAL_NS_END

