#ifndef TASK_DISPATCHER_CAPI_HPP
#define TASK_DISPATCHER_CAPI_HPP

#include "ITaskDispatcher.hpp"
#include "mat.h"
#include "Version.hpp"

namespace PAL_NS_BEGIN {
    class TaskDispatcher_CAPI : public MAT::ITaskDispatcher
    {
    public:
        TaskDispatcher_CAPI(evt_task_dispatcher_queue_fn queueFn, evt_task_dispatcher_cancel_fn cancelFn, evt_task_dispatcher_join_fn joinFn);
        void Join() override;
        void Queue(MAT::Task* task) override;
        bool Cancel(MAT::Task* task) override;

    private:
        evt_task_dispatcher_queue_fn m_queueFn;
        evt_task_dispatcher_cancel_fn m_cancelFn;
        evt_task_dispatcher_join_fn m_joinFn;
    };
} PAL_NS_END

#endif
