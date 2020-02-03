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

#include "ITaskDispatcher.hpp"
#include "Version.hpp"

namespace PAL_NS_BEGIN {

    namespace detail {

        // TODO: [MG] - allow lambdas, std::function, functors, etc.
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

            virtual void operator()() override
            {
                m_call();
            }

            virtual ~TaskCall() noexcept = default;

            const TCall m_call;
        };

    } // namespace detail

    class DeferredCallbackHandle
    {
    public:
        MAT::Task* m_task;
        MAT::ITaskDispatcher* m_taskDispatcher;

        DeferredCallbackHandle(MAT::Task* task, MAT::ITaskDispatcher* taskDispatcher) :
            m_task(task),
            m_taskDispatcher(taskDispatcher) { };
        DeferredCallbackHandle() : m_task(nullptr), m_taskDispatcher(nullptr) {};
        DeferredCallbackHandle(const DeferredCallbackHandle& h) :
            m_task(h.m_task),
            m_taskDispatcher(h.m_taskDispatcher) { };

        bool Cancel(uint64_t waitTime = 0)
        {
            if (m_task)
            {
                bool result = (m_taskDispatcher != nullptr) && (m_taskDispatcher->Cancel(m_task, waitTime));
                m_task = nullptr;
                m_taskDispatcher = nullptr;
                return result;
            }
            return false;
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
        auto task = new detail::TaskCall<decltype(bound)>(bound, getMonotonicTimeMs() + (int64_t)delayMs);
        taskDispatcher->Queue(task);
        return DeferredCallbackHandle(task, taskDispatcher);
    }

    template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
    DeferredCallbackHandle scheduleTask(MAT::ITaskDispatcher* taskDispatcher, unsigned delayMs, const TObject& obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
    {
        return scheduleTask(taskDispatcher, delayMs, (TObject*)(&obj), func, std::forward<TPassedArgs>(args)...);
    }

} PAL_NS_END

#endif
