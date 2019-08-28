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

    class WorkItem_CAPI
    {
    public:
        WorkItem_CAPI(std::unique_ptr<WorkerThreadItem> item)
          : m_item(std::move(item)) {}

        ~WorkItem_CAPI() noexcept
        {
            ReleaseItem();
        }

        WorkerThreadItem* GetItem()
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
                m_item->Type = WorkerThreadItem::Done;
                m_item = nullptr;
            }
        }

        std::unique_ptr<WorkerThreadItem> m_item;
    };


    static std::mutex s_workItemsLock;

    std::map<std::string, std::shared_ptr<WorkItem_CAPI>>& GetPendingWorkItems() {
      static std::map<std::string, std::shared_ptr<WorkItem_CAPI>> s_workItems;
      return s_workItems;
    }

    std::string GetNextWorkItemId() {
        static std::atomic<int32_t> s_nextWorkItemId(0);
        std::ostringstream idStream;
        idStream << "MAT_WorkItem-" << s_nextWorkItemId++;
        return idStream.str();
    }

    void EVTSDK_LIBABI_CDECL OnAsyncWorkItemCallback(const char* workItemId)
    {
        std::shared_ptr<WorkItem_CAPI> workItem;

        // Find and remove pending work item
        {
            LOCKGUARD(s_workItemsLock);
            auto itWorkItem = GetPendingWorkItems().find(workItemId);
            if (itWorkItem != GetPendingWorkItems().end()) {
                workItem = itWorkItem->second;
                GetPendingWorkItems().erase(itWorkItem);
            }
        }

        if (workItem)
            workItem->OnCallback();
    }

    WorkerThread_CAPI::WorkerThread_CAPI(worker_thread_queue_fn_t queueFn, worker_thread_cancel_fn_t cancelFn, worker_thread_join_fn_t joinFn)
      : m_queueFn(queueFn),
        m_cancelFn(cancelFn),
        m_joinFn(joinFn)
    {
        if ((queueFn == nullptr) || (cancelFn == nullptr) || (joinFn == nullptr))
        {
            throw std::invalid_argument("Created WorkerThread_CAPI with invalid parameters");
        }
    }

    void WorkerThread_CAPI::Join()
    {
        m_joinFn();
    }

    void WorkerThread_CAPI::Queue(WorkerThreadItem* item)
    {
        if (item->Type != WorkerThreadItem::Call && item->Type != WorkerThreadItem::TimedCall)
            return;

        auto ownedItem = std::unique_ptr<WorkerThreadItem>(item);

        // Create work item
        work_item_t workItem;
        std::string workItemId = GetNextWorkItemId();
        workItem.id = workItemId.c_str();
        workItem.typeName = ownedItem->TypeName.c_str();
        workItem.delayMs = 0;
        if (ownedItem->Type == WorkerThreadItem::TimedCall) {
            workItem.delayMs = ownedItem->TargetTime - getMonotonicTimeMs();
        }

        // Add pending work item
        {
            LOCKGUARD(s_workItemsLock);
            GetPendingWorkItems()[workItem.id] = std::make_shared<WorkItem_CAPI>(std::move(ownedItem));
        }

        m_queueFn(&workItem, &OnAsyncWorkItemCallback);
    }

    bool WorkerThread_CAPI::Cancel(WorkerThreadItem* item)
    {
        std::string workItemId;

        // Find and erase pending work item
        {
            LOCKGUARD(s_workItemsLock);
            auto itWorkItem = std::find_if(GetPendingWorkItems().begin(), GetPendingWorkItems().end(),
                    [item](const std::pair<std::string, std::shared_ptr<WorkItem_CAPI>>& workItem) {
                return workItem.second->GetItem() == item;
            });

            if (itWorkItem != GetPendingWorkItems().end()) {
                workItemId = itWorkItem->first;
                GetPendingWorkItems().erase(itWorkItem);
            }
        }

        return (!workItemId.empty()) ? m_cancelFn(workItemId.c_str()) : false;
    }

} PAL_NS_END
