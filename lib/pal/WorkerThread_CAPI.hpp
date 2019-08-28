#ifndef WORKER_THREAD_CAPI_HPP
#define WORKER_THREAD_CAPI_HPP

#include "IWorkerThread.hpp"
#include "mat.h"
#include "Version.hpp"

namespace PAL_NS_BEGIN {
    class WorkerThread_CAPI : public MAT::IWorkerThread
    {
    public:
        WorkerThread_CAPI(worker_thread_queue_fn_t queueFn, worker_thread_cancel_fn_t cancelFn, worker_thread_join_fn_t joinFn);
        void join() override;
        void queue(MAT::WorkerThreadItemPtr item) override;
        bool cancel(MAT::WorkerThreadItemPtr item) override;

    private:
        worker_thread_queue_fn_t m_queueFn;
        worker_thread_cancel_fn_t m_cancelFn;
        worker_thread_join_fn_t m_joinFn;
    };
} PAL_NS_END

#endif