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
            std::recursive_mutex mutex;
            MAT::Task* task {nullptr};
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
                std::lock_guard<std::recursive_mutex> lock(m_lifetimeState->mutex);
                m_lifetimeState->task = this;
            }

            virtual void operator()() override
            {
                m_call();
            }

            virtual ~TaskCall() noexcept
            {
                if (m_lifetimeState)
                {
                    std::lock_guard<std::recursive_mutex> lock(m_lifetimeState->mutex);
                    m_lifetimeState->task = nullptr;
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
            if (this == &other)
            {
                return *this;
            }

            std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);
            std::unique_lock<std::mutex> otherLock(other.m_mutex, std::defer_lock);
            std::lock(lock, otherLock);
            m_taskLifetimeState = std::move(other.m_taskLifetimeState);
            m_taskDispatcher = other.m_taskDispatcher;
            other.m_taskDispatcher = nullptr;

            return *this;
        }

        MAT::Task* GetTask() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_taskLifetimeState == nullptr)
            {
                return nullptr;
            }
            std::lock_guard<std::recursive_mutex> lifetimeLock(m_taskLifetimeState->mutex);
            return m_taskLifetimeState->task;
        }

        bool Cancel(uint64_t waitTime = 0)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_taskLifetimeState == nullptr)
            {
                return true;
            }

            // Keep task destruction serialized with the dispatcher's pointer
            // lookup so this address cannot be freed and reused for a different
            // task between the lookup here and Cancel(). A recursive mutex is
            // required because dispatchers may delete queued tasks synchronously
            // from Cancel(), re-entering TaskCall's destructor on this thread.
            std::lock_guard<std::recursive_mutex> lifetimeLock(m_taskLifetimeState->mutex);
            MAT::Task* task = m_taskLifetimeState->task;
            if (task)
            {
                bool result = (m_taskDispatcher != nullptr) && (m_taskDispatcher->Cancel(task, waitTime));
                return result || (m_taskLifetimeState->task == nullptr);
            }
            return true;
        }

    private:
        mutable std::mutex m_mutex;
        std::shared_ptr<detail::TaskLifetimeState> m_taskLifetimeState;
        MAT::ITaskDispatcher* m_taskDispatcher = nullptr;
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
        auto taskLifetimeState = std::make_shared<detail::TaskLifetimeState>();
        auto task = new detail::TaskCall<decltype(bound)>(
            bound,
            getMonotonicTimeMs() + (int64_t)delayMs,
            taskLifetimeState);
        taskDispatcher->Queue(task);
        {
            std::lock_guard<std::recursive_mutex> lock(taskLifetimeState->mutex);
            if (taskLifetimeState->task == nullptr)
            {
                return DeferredCallbackHandle();
            }
        }
        return DeferredCallbackHandle(taskLifetimeState, taskDispatcher);
    }

    template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
    DeferredCallbackHandle scheduleTask(MAT::ITaskDispatcher* taskDispatcher, unsigned delayMs, const TObject& obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
    {
        return scheduleTask(taskDispatcher, delayMs, (TObject*)(&obj), func, std::forward<TPassedArgs>(args)...);
    }

} PAL_NS_END

#endif
