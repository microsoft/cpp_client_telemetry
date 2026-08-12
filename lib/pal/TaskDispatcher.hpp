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
        DeferredCallbackHandle(std::shared_ptr<detail::TaskLifetimeState> taskLifetimeState, MAT::ITaskDispatcher* taskDispatcher) :
            m_taskLifetimeState(std::move(taskLifetimeState)),
            m_taskDispatcher(taskDispatcher) { }

        DeferredCallbackHandle() = default;
        DeferredCallbackHandle(DeferredCallbackHandle&& h)
        {
            *this = std::move(h);
        }

        DeferredCallbackHandle& operator=(DeferredCallbackHandle&& other)
        {
            if (this == &other) {
                return *this;
            }

            std::lock_guard<std::mutex> lock(m_mutex);
            std::lock_guard<std::mutex> otherLock(other.m_mutex);
            m_taskLifetimeState = std::move(other.m_taskLifetimeState);
            m_taskDispatcher = other.m_taskDispatcher;
            other.m_taskDispatcher = nullptr;

            return *this;
        }

        MAT::Task* GetTask() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return (m_taskLifetimeState != nullptr) ? m_taskLifetimeState->task.load(std::memory_order_acquire) : nullptr;
        }

        bool Cancel(uint64_t waitTime = 0)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            MAT::Task* task = (m_taskLifetimeState != nullptr) ? m_taskLifetimeState->task.load(std::memory_order_acquire) : nullptr;
            if (task)
            {
                bool result = (m_taskDispatcher != nullptr) && (m_taskDispatcher->Cancel(task, waitTime));
                return result || ((m_taskLifetimeState != nullptr) && (m_taskLifetimeState->task.load(std::memory_order_acquire) == nullptr));
            }
            else {
                // Canceled nothing successfully
                return true;
            }
        }

    private:
        mutable std::mutex m_mutex;
        std::shared_ptr<detail::TaskLifetimeState> m_taskLifetimeState;
        MAT::ITaskDispatcher* m_taskDispatcher = nullptr;
    };

    inline DeferredCallbackHandle scheduleTask(
        MAT::ITaskDispatcher* taskDispatcher,
        unsigned delayMs,
        std::function<void()> call)
    {
        auto taskLifetime = std::make_shared<detail::TaskLifetimeState>();
        auto task = new detail::TaskCall<std::function<void()>>(
            call,
            getMonotonicTimeMs() + static_cast<int64_t>(delayMs),
            taskLifetime);
        taskDispatcher->Queue(task);
        if (taskLifetime->task.load(std::memory_order_acquire) == nullptr)
        {
            return DeferredCallbackHandle();
        }
        return DeferredCallbackHandle(taskLifetime, taskDispatcher);
    }

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
        // synchronously clears this state before Queue() returns, and the task
        // destructor publishes completion after normal asynchronous execution.
        // Cancel() treats the pointer only as an opaque dispatcher identity
        // because completion may race the handle's atomic load.
        if (taskLifetime->task.load(std::memory_order_acquire) == nullptr)
        {
            return DeferredCallbackHandle();
        }
        return DeferredCallbackHandle(taskLifetime, taskDispatcher);
    }

    template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
    DeferredCallbackHandle scheduleTask(MAT::ITaskDispatcher* taskDispatcher, unsigned delayMs, const TObject& obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
    {
        return scheduleTask(taskDispatcher, delayMs, (TObject*)(&obj), func, std::forward<TPassedArgs>(args)...);
    }

} PAL_NS_END

#endif
