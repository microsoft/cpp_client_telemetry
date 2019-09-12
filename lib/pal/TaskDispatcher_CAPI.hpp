#ifndef TASK_DISPATCHER_CAPI_HPP
#define TASK_DISPATCHER_CAPI_HPP

#include "ITaskDispatcher.hpp"
#include "mat.h"
#include "Version.hpp"

namespace PAL_NS_BEGIN {
    class TaskDispatcher_CAPI : public MAT::ITaskDispatcher
    {
    public:
        TaskDispatcher_CAPI(task_dispatcher_queue_fn_t queueFn, task_dispatcher_cancel_fn_t cancelFn, task_dispatcher_join_fn_t joinFn);
        void Join() override;
        void Queue(MAT::Task* task) override;
        bool Cancel(MAT::Task* task) override;

    private:
        task_dispatcher_queue_fn_t m_queueFn;
        task_dispatcher_cancel_fn_t m_cancelFn;
        task_dispatcher_join_fn_t m_joinFn;
    };
} PAL_NS_END

#endif
