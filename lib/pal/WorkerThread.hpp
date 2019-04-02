#ifndef WORKER_THREAD_HPP
#define WORKER_THREAD_HPP

#include <list>
#include <mutex>
#include <stdint.h>
#include <string>
#include <thread>
#include <condition_variable>
#include <climits>
#include <algorithm>

#include "Version.hpp"

namespace PAL_NS_BEGIN {

    // Event (binary semaphore)
    class Event
    {
    protected:
        bool m_bFlag;
        mutable std::mutex m_mutex;
        mutable std::condition_variable m_condition;

    public:

        inline Event() : m_bFlag(false) {}

        ~Event()
        {
            Reset();
        }

        bool wait(unsigned millis = UINT_MAX) const
        {
            if (millis == UINT_MAX)
            {
                std::unique_lock< std::mutex > lock(m_mutex);
                m_condition.wait(lock, [&]()->bool {return m_bFlag; });
                return true;
            }

            auto crRelTime = std::chrono::milliseconds(millis);
            std::unique_lock<std::mutex> ulock(m_mutex);
            if (!m_condition.wait_for(ulock, crRelTime, [&]()->bool {return m_bFlag; }))
                return false;
            return true;
        }

        inline bool post()
        {
            bool bWasSignalled;
            m_mutex.lock();
            bWasSignalled = m_bFlag;
            m_bFlag = true;
            m_mutex.unlock();
            m_condition.notify_all();
            return bWasSignalled == false;
        }

        inline bool Reset()
        {
            bool bWasSignalled;
            m_mutex.lock();
            bWasSignalled = m_bFlag;
            m_bFlag = false;
            m_mutex.unlock();
            return bWasSignalled;
        }

        inline bool IsSet() const { return m_bFlag; }
    };

    class DeferredCallbackHandle;

    namespace detail {

        class WorkerThreadItem
        {
        public:
            volatile enum { Shutdown, Call, TimedCall, Done } type;
            int64_t targetTime;
            virtual ~WorkerThreadItem() {}
            virtual void operator()() {}
            std::string typeName;
        };

        typedef WorkerThreadItem* WorkerThreadItemPtr;

        // TODO: [MG] - allow lambdas, std::function, functors, etc.
        template<typename TCall>
        class WorkerThreadCall : public WorkerThreadItem
        {
        public:

            WorkerThreadCall(TCall& call) :
                WorkerThreadItem(),
                m_call(call)
            {
                this->typeName = __typename(call);
                this->type = WorkerThreadItem::Call;
                this->targetTime = -1;
            }

            WorkerThreadCall(TCall& call, int64_t targetTime) :
                WorkerThreadItem(),
                m_call(call)
            {
                this->typeName = __typename(call);
                this->type = WorkerThreadItem::TimedCall;
                this->targetTime = targetTime;
            }

            virtual void operator()() override
            {
                m_call();
            }

            virtual ~WorkerThreadCall()
            {
            }

            const TCall m_call;
        };

    } // namespace detail

    namespace detail {
        extern void queueWorkerThreadItem(detail::WorkerThreadItemPtr item);
        extern bool cancelWorkerThreadItem(detail::WorkerThreadItemPtr item);
    } // namespace detail

    class DeferredCallbackHandle
    {
    public:
        detail::WorkerThreadItemPtr m_item;

        DeferredCallbackHandle(detail::WorkerThreadItemPtr item) : m_item(item) { };
        DeferredCallbackHandle() : m_item(nullptr) {};
        DeferredCallbackHandle(const DeferredCallbackHandle& h) : m_item(h.m_item) { };
        bool cancel()
        {
            if (m_item)
            {
                bool result = detail::cancelWorkerThreadItem(m_item);
                m_item = nullptr;
                return result;
            }
            return false;
        }
    };

    // *INDENT-OFF* parameter pack expansions etc.

    template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
    void executeOnWorkerThread(TObject* obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
    {
        assert(obj != nullptr);
        auto bound = std::bind(std::mem_fn(func), obj, std::forward<TPassedArgs>(args)...);
        detail::WorkerThreadItemPtr item = new detail::WorkerThreadCall<decltype(bound)>(bound);
        detail::queueWorkerThreadItem(item);
    }

    template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
    void executeOnWorkerThread(const TObject& obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
    {
        executeOnWorkerThread((TObject*)(&obj), func, std::forward<TPassedArgs>(args)...);
    }

    // Return the monotonic system clock time in milliseconds (since unspecified point).
    extern int64_t getMonotonicTimeMs();

    template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
    DeferredCallbackHandle scheduleOnWorkerThread(unsigned delayMs, TObject* obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
    {
        auto bound = std::bind(std::mem_fn(func), obj, std::forward<TPassedArgs>(args)...);
        auto item = new detail::WorkerThreadCall<decltype(bound)>(bound, getMonotonicTimeMs() + (int64_t)delayMs);
        detail::queueWorkerThreadItem(item);
        return DeferredCallbackHandle(item);
    }

    template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
    DeferredCallbackHandle scheduleOnWorkerThread(unsigned delayMs, const TObject& obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
    {
        return scheduleOnWorkerThread(delayMs, (TObject*)(&obj), func, std::forward<TPassedArgs>(args)...);
    }

    class IWorkerThread
    {
    public:
        virtual ~IWorkerThread() {}

        virtual void join() = 0;
        virtual void queue(detail::WorkerThreadItemPtr item) = 0;
        virtual bool cancel(detail::WorkerThreadItemPtr item) = 0;
    };

    namespace WorkerThreadFactory {
        IWorkerThread* Create();
    }

} PAL_NS_END

#endif