//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
// clang-format off
#ifndef TASK_DISPATCHER_HPP
#define TASK_DISPATCHER_HPP

#include <functional>
#include <list>
#include <mutex>
#include <stdint.h>
#include <string>
#include <thread>
#include <condition_variable>
#include <climits>
#include <algorithm>
#include <atomic>
#include <memory>
#include <utility>

#include "ITaskDispatcher.hpp"
#include "ctmacros.hpp"

namespace PAL_NS_BEGIN {

    namespace detail {

        struct TaskLifetimeState
        {
            TaskLifetimeState() :
                task(nullptr)
            {}

            std::atomic<MAT::Task*> task;
        };

        template<typename TCall>
        class TaskCall : public Task
        {
        public:

            TaskCall(TCall& call) :
                Task(),
                m_call(call)
            {
                this->TypeName = TYPENAME(call);
                this->Type = Task::Call;
                this->TargetTime = 0;
            }

            TaskCall(TCall& call, int64_t targetTime) :
                Task(),
                m_call(call)
            {
                this->TypeName = TYPENAME(call);
                this->Type = Task::TimedCall;
                this->TargetTime = targetTime;
            }

            TaskCall(TCall& call, int64_t targetTime, std::shared_ptr<TaskLifetimeState> lifetimeState) :
                Task(),
                m_call(call),
                m_lifetimeState(std::move(lifetimeState))
            {
                this->TypeName = TYPENAME(call);
                this->Type = Task::TimedCall;
                this->TargetTime = targetTime;
                if (m_lifetimeState) {
                    m_lifetimeState->task.store(this, std::memory_order_release);
                }
            }

            virtual void operator()() override
            {
                m_call();
            }

            virtual ~TaskCall() noexcept
            {
                if (m_lifetimeState) {
                    m_lifetimeState->task.store(nullptr, std::memory_order_release);
                }
            }

            const TCall m_call;

        private:
            std::shared_ptr<TaskLifetimeState> m_lifetimeState;
        };

    } // namespace detail

    class DeferredCallbackHandle
    {
    public:
        std::mutex m_mutex;
        MAT::Task* m_task = nullptr;
        MAT::ITaskDispatcher* m_taskDispatcher = nullptr;

        DeferredCallbackHandle(MAT::Task* task, MAT::ITaskDispatcher* taskDispatcher) :
            m_task(task),
            m_taskDispatcher(taskDispatcher) { }
        DeferredCallbackHandle() {}
        DeferredCallbackHandle(DeferredCallbackHandle&& h)
        {
            *this = std::move(h);
        }

        DeferredCallbackHandle& operator=(DeferredCallbackHandle&& other)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::lock_guard<std::mutex> otherLock(other.m_mutex);
            m_task = other.m_task;
            other.m_task = nullptr;
            m_taskDispatcher = other.m_taskDispatcher;

            return *this;
        }

        bool Cancel(uint64_t waitTime = 0)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_task)
            {
                bool result = (m_taskDispatcher != nullptr) && (m_taskDispatcher->Cancel(m_task, waitTime));
                return result;
            }
            else {
                // Canceled nothing successfully
                return true;
            }
        }
    };

    template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
    void dispatchTask(MAT::ITaskDispatcher* taskDispatcher, TObject* obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
    {
        assert(obj != nullptr);
        auto bound = std::bind(std::mem_fn(func), obj, std::forward<TPassedArgs>(args)...);
        MAT::Task* task = new detail::TaskCall<decltype(bound)>(bound);
        taskDispatcher->Queue(task);
    }

    template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
    void dispatchTask(MAT::ITaskDispatcher* taskDispatcher, const TObject& obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
    {
        dispatchTask(taskDispatcher, (TObject*)(&obj), func, std::forward<TPassedArgs>(args)...);
    }

    template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
    DeferredCallbackHandle scheduleTask(MAT::ITaskDispatcher* taskDispatcher, unsigned delayMs, TObject* obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
    {
        auto bound = std::bind(std::mem_fn(func), obj, std::forward<TPassedArgs>(args)...);
        auto taskLifetime = std::make_shared<detail::TaskLifetimeState>();
        auto task = new detail::TaskCall<decltype(bound)>(bound, getMonotonicTimeMs() + (int64_t)delayMs, taskLifetime);
        taskDispatcher->Queue(task);
        // Queue() is void; an SDK dispatcher that rejects by deleting the task
        // synchronously clears this state before Queue() returns.
        auto queuedTask = taskLifetime->task.load(std::memory_order_acquire);
        if (queuedTask == nullptr)
        {
            return DeferredCallbackHandle();
        }
        return DeferredCallbackHandle(queuedTask, taskDispatcher);
    }

    template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
    DeferredCallbackHandle scheduleTask(MAT::ITaskDispatcher* taskDispatcher, unsigned delayMs, const TObject& obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
    {
        return scheduleTask(taskDispatcher, delayMs, (TObject*)(&obj), func, std::forward<TPassedArgs>(args)...);
    }

} PAL_NS_END

#endif
