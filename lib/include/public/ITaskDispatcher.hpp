//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef ITASKDISPATCHER_HPP
#define ITASKDISPATCHER_HPP

#include "IModule.hpp"
#include "ctmacros.hpp"

#include <atomic>
#include <string>
#include <vector>

///@cond INTERNAL_DOCS
namespace MAT_NS_BEGIN
{
    /// <summary>
    /// The Task class represents a single executable task that can be dispatched to an asynchronous worker
    /// thread.
    /// Individual Task implementations override operator() to execute
    /// </summary>
    class Task
    {
        /// <summary>
        /// Atomic counter that returns sequentially incrementing unique Task ID
        /// </summary>
        static uint64_t GetNewTid()
        {
            static std::atomic<uint64_t> lastTid;
            return lastTid.fetch_add(1);
        }

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
            Done,
            /// <summary>
            /// A Cancelled item is an item that has been marked by the worker thread as Cancelled.
            /// </summary>
            Cancelled
        } Type;

        Task() :
            tid(GetNewTid())
        {};

        /// <summary>
        /// The time (in milliseconds since epoch) when this work item should be executed
        /// </summary>
        uint64_t TargetTime;

        /// <summary>
        /// Unique Task Id.
        /// TODO: [maxgolov] - use this Task Id for task cancellation instead of raw ptr
        /// </summary>
        uint64_t tid;

        /// <summary>
        /// The Task class destructor.
        /// </summary>
        virtual ~Task() noexcept = default;

        /// <summary>
        /// The functor implementation that executes task logic. This method is overridden by Task
        /// implementations.
        /// </summary>
        virtual void operator()() {}

        /// <summary>
        /// The typename of the underlying functor executed by this work item
        /// </summary>
        std::string TypeName;
    };

    /// <summary>
    /// The ITaskDispatcher class manages dispatching of asynchronous tasks to background worker thread(s).
    /// Individual TaskDispatcher implementations can manage creation/destruction of thread resources.
    /// </summary>
    class ITaskDispatcher : public IModule
    {
    public:
        /// <summary>
        /// The ITaskDispatcher class destructor.
        /// </summary>
        virtual ~ITaskDispatcher() noexcept = default;

        /// <summary>
        /// Terminate worker thread(s) and clean up thread resources
        /// </summary>
        virtual void Join() = 0;

        /// <summary>
        /// Queue an asynchronous task for dispatching to a worker thread
        /// </summary>
        /// <param name="task">Task to be executed on a worker thread</param>
        virtual void Queue(Task* task) = 0;

        /// <summary>
        /// Cancel a previously queued tasks
        /// </summary>
        /// <param name="task">Task to be cancelled</param>
        /// <param name="waitTime">Amount of time to wait for if the task is currently executing</param>
        /// <returns>True if successfully cancelled, else false</returns>
        virtual bool Cancel(Task* task, uint64_t waitTime = 0) = 0;
    };

    /// @endcond

} MAT_NS_END

#endif // ITASKDISPATCHER_HPP

