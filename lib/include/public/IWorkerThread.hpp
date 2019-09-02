// Copyright (c) Microsoft. All rights reserved.
#ifndef IWORKERTHREAD_HPP
#define IWORKERTHREAD_HPP

#include "IModule.hpp"
#include "Version.hpp"

#include <string>
#include <vector>

///@cond INTERNAL_DOCS
namespace ARIASDK_NS_BEGIN
{
    /// <summary>
    /// The WorkerThreadItem class represents a single executable task that can be dispatched to an asynchronous worker
    /// thread.
    /// Individual WorkerThreadItem implementations override operator() to execute
    /// </summary>
    class WorkerThreadItem : public IModule
    {
    public:
        /// <summary>
        /// Type of work item
        /// </summary>
        volatile enum
        {
            /// <summary>
            /// A Shutdown item indicates that the worker thread should terminate and stop processing further items
            /// </summary>
            Shutdown,
            /// <summary>
            /// A Call item is a generic functor that should execute as soon as possible
            /// </summary>
            Call,
            /// <summary>
            /// A TimedCall item is a generic functor that should execute at the time specified by TargetTime
            /// </summary>
            TimedCall,
            /// <summary>
            /// A Done item is an item that has been marked by the worker thread as already completed.
            /// </summary>
            Done
        } Type;

        /// <summary>
        /// The time (in milliseconds since epoch) when this work item should be executed
        /// </summary>
        int64_t TargetTime;

        /// <summary>
        /// The WorkerThreadItem class destructor.
        /// </summary>
        virtual ~WorkerThreadItem() noexcept = default;

        /// <summary>
        /// The functor implementation that executes task logic. This method is overridden by WorkerThreadItem
        /// implementations.
        /// </summary>
        virtual void operator()() {}

        /// <summary>
        /// The typename of the underlying functor executed by this work item
        /// </summary>
        std::string TypeName;
    };

    /// <summary>
    /// The IWorkerThread class manages dispatching of asynchronous tasks to background worker thread(s).
    /// Individual WorkerThread implementations can manage creation/destruction of thread resources.
    /// </summary>
    class IWorkerThread
    {
    public:
        /// <summary>
        /// The IWorkerThread class destructor.
        /// </summary>
        virtual ~IWorkerThread() noexcept = default;

        /// <summary>
        /// Terminate worker thread(s) and clean up thread resources
        /// </summary>
        virtual void Join() = 0;

        /// <summary>
        /// Queue an asynchronous task for dispatching to a worker thread
        /// </summary>
        /// <param name="item">Task to be executed on a worker thread</param>
        virtual void Queue(WorkerThreadItem* item) = 0;

        /// <summary>
        /// Cancel a previously queued tasks
        /// </summary>
        /// <param name="item">Task to be cancelled</param>
        /// <returns>True if successfully cancelled, else false</returns>
        virtual bool Cancel(WorkerThreadItem* item) = 0;
    };

    /// @endcond

} ARIASDK_NS_END

#endif // IWORKERTHREAD_HPP