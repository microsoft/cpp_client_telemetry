#include "pal/WorkerThread_CAPI.hpp"

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

    class WorkerThread_Task
    {
    public:
        WorkerThread_Task(std::unique_ptr<WorkerThreadItem> item)
          : m_item(std::move(item)) {}

        ~WorkerThread_Task()
        {
            ReleaseItem();
        }

        WorkerThreadItemPtr GetItem()
        {
            return m_item.get();
        }

        void OnCallback()
        {
            if (m_item) {
                (*m_item)();
            }
            ReleaseItem();
        }

    private:
        void ReleaseItem()
        {
            if (m_item) {
                m_item->type = WorkerThreadItem::Done;
                m_item = nullptr;
            }
        }

        std::unique_ptr<WorkerThreadItem> m_item;
    };


    static std::mutex s_tasksLock;

    std::map<std::string, std::shared_ptr<WorkerThread_Task>>& GetPendingTasks() {
      static std::map<std::string, std::shared_ptr<WorkerThread_Task>> s_tasks;
      return s_tasks;
    }

    std::string GetNextTaskId() {
        static std::atomic<int32_t> s_nextTaskId(0);
        std::ostringstream idStream;
        idStream << "MAT_Task-" << s_nextTaskId++;
        return idStream.str();
    }

    void EVTSDK_LIBABI_CDECL OnAsyncTaskCallback(const char* taskId)
    {
        std::shared_ptr<WorkerThread_Task> task;

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

    WorkerThread_CAPI::WorkerThread_CAPI(task_queue_fn_t queueFn, task_cancel_fn_t cancelFn, task_shutdown_fn_t shutdownFn)
      : m_queueFn(queueFn),
        m_cancelFn(cancelFn),
        m_shutdownFn(shutdownFn)
    {
        if ((queueFn == nullptr) || (cancelFn == nullptr) || (shutdownFn == nullptr))
        {
            throw std::invalid_argument("Created WorkerThread_CAPI with invalid parameters");
        }
    }

    void WorkerThread_CAPI::join()
    {
        m_shutdownFn();
    }

    void WorkerThread_CAPI::queue(WorkerThreadItemPtr item)
    {
        if (item->type != WorkerThreadItem::Call && item->type != WorkerThreadItem::TimedCall)
            return;

        auto ownedItem = std::unique_ptr<WorkerThreadItem>(item);

        // Create task
        async_task_t task;
        task.id = GetNextTaskId().c_str();
        task.typeName = ownedItem->typeName.c_str();
        task.delayMs = 0;
        if (ownedItem->type == WorkerThreadItem::TimedCall) {
            task.delayMs = ownedItem->targetTime - getMonotonicTimeMs();
        }

        // Add pending task
        {
            LOCKGUARD(s_tasksLock);
            GetPendingTasks()[task.id] = std::make_shared<WorkerThread_Task>(std::move(ownedItem));
        }

        m_queueFn(&task, &OnAsyncTaskCallback);
    }

    bool WorkerThread_CAPI::cancel(WorkerThreadItemPtr item)
    {
        std::string taskId;

        // Find and erase pending task
        {
            LOCKGUARD(s_tasksLock);
            auto itTask = std::find_if(GetPendingTasks().begin(), GetPendingTasks().end(),
                    [item](const std::pair<std::string, std::shared_ptr<WorkerThread_Task>>& task) {
                return task.second->GetItem() == item;
            });

            if (itTask != GetPendingTasks().end()) {
                taskId = itTask->first;
                GetPendingTasks().erase(itTask);
            }
        }

        return (!taskId.empty()) ? m_cancelFn(taskId.c_str()) : false;
    }

} PAL_NS_END
