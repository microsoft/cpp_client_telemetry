#ifndef WORKER_THREAD_HPP
#define WORKER_THREAD_HPP

#include <functional>
#include <list>
#include <mutex>
#include <stdint.h>
#include <string>
#include <thread>
#include <condition_variable>
#include <climits>
#include <algorithm>

#include "IWorkerThread.hpp"
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

        // TODO: [MG] - allow lambdas, std::function, functors, etc.
        template<typename TCall>
        class WorkerThreadCall : public WorkerThreadItem
        {
        public:

            WorkerThreadCall(TCall& call) :
                WorkerThreadItem(),
                m_call(call)
            {
                this->TypeName = __typename(call);
                this->Type = WorkerThreadItem::Call;
                this->TargetTime = -1;
            }

            WorkerThreadCall(TCall& call, int64_t targetTime) :
                WorkerThreadItem(),
                m_call(call)
            {
                this->TypeName = __typename(call);
                this->Type = WorkerThreadItem::TimedCall;
                this->TargetTime = targetTime;
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

    class DeferredCallbackHandle
    {
    public:
        MAT::WorkerThreadItem* m_item;
        MAT::IWorkerThread* m_workerThread;

        DeferredCallbackHandle(MAT::WorkerThreadItem* item, MAT::IWorkerThread* workerThread) :
            m_item(item),
            m_workerThread(workerThread) { };
        DeferredCallbackHandle() : m_item(nullptr), m_workerThread(nullptr) {};
        DeferredCallbackHandle(const DeferredCallbackHandle& h) :
            m_item(h.m_item),
            m_workerThread(h.m_workerThread) { };

        bool Cancel()
        {
            if (m_item)
            {
                bool result = (m_workerThread != nullptr) && (m_workerThread->Cancel(m_item));
                m_item = nullptr;
                m_workerThread = nullptr;
                return result;
            }
            return false;
        }
    };

    // *INDENT-OFF* parameter pack expansions etc.

    template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
    void executeOnWorkerThread(MAT::IWorkerThread* workerThread, TObject* obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
    {
        assert(obj != nullptr);
        auto bound = std::bind(std::mem_fn(func), obj, std::forward<TPassedArgs>(args)...);
        MAT::WorkerThreadItem* item = new detail::WorkerThreadCall<decltype(bound)>(bound);
        workerThread->Queue(item);
    }

    template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
    void executeOnWorkerThread(MAT::IWorkerThread* workerThread, const TObject& obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
    {
        executeOnWorkerThread(workerThread, (TObject*)(&obj), func, std::forward<TPassedArgs>(args)...);
    }

    // Return the monotonic system clock time in milliseconds (since unspecified point).
    extern int64_t getMonotonicTimeMs();

    template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
    DeferredCallbackHandle scheduleOnWorkerThread(MAT::IWorkerThread* workerThread, unsigned delayMs, TObject* obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
    {
        auto bound = std::bind(std::mem_fn(func), obj, std::forward<TPassedArgs>(args)...);
        auto item = new detail::WorkerThreadCall<decltype(bound)>(bound, getMonotonicTimeMs() + (int64_t)delayMs);
        workerThread->Queue(item);
        return DeferredCallbackHandle(item, workerThread);
    }

    template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
    DeferredCallbackHandle scheduleOnWorkerThread(MAT::IWorkerThread* workerThread, unsigned delayMs, const TObject& obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
    {
        return scheduleOnWorkerThread(workerThread, delayMs, (TObject*)(&obj), func, std::forward<TPassedArgs>(args)...);
    }

    namespace WorkerThreadFactory {
        MAT::IWorkerThread* Create();
    }

} PAL_NS_END

#endif