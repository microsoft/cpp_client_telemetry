#ifndef WORKER_THREAD_CAPI_HPP
#define WORKER_THREAD_CAPI_HPP

#include "IWorkerThread.hpp"
#include "mat.h"
#include "Version.hpp"

namespace PAL_NS_BEGIN {
    class WorkerThread_CAPI : public MAT::IWorkerThread
    {
    public:
        WorkerThread_CAPI(task_queue_fn_t queueFn, task_cancel_fn_t cancelFn, task_shutdown_fn_t shutdownFn);
        void join() override;
        void queue(MAT::WorkerThreadItemPtr item) override;
        bool cancel(MAT::WorkerThreadItemPtr item) override;

    private:
        task_queue_fn_t m_queueFn;
        task_cancel_fn_t m_cancelFn;
        task_shutdown_fn_t m_shutdownFn;
    };
} PAL_NS_END

#endif