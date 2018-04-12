// Copyright (c) Microsoft. All rights reserved.
#ifndef PAL_CPP11_HPP
#define PAL_CPP11_HPP

#include "SystemInformationImpl.hpp"
#include "NetworkInformationImpl.hpp"
#include "DeviceInformationImpl.hpp"

#include <ISemanticContext.hpp>
#include <api/ContextFieldsProvider.hpp>

#if defined(_WIN32) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <Rpc.h>
#endif

#undef max
#undef min

#include <assert.h>
#include <stdint.h>

#include <functional>
#include <list>
#include <map>
#include <random>
#include <string>
#include <type_traits>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <climits>
#include <chrono>
#include <thread>

#include "typename.hpp"

namespace ARIASDK_NS_BEGIN {
    extern void print_backtrace();
}ARIASDK_NS_END

#ifdef _WIN32
#define PATH_SEPARATOR_CHAR '\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#endif

namespace PAL_NS_BEGIN {

    extern const char * getAriaSdkLogComponent();

    class INetworkInformation;
    class IDeviceInformation;
    class ISystemInformation;

    //
    // Startup/shutdown
    //

    void initialize();
    void shutdown();
    INetworkInformation* GetNetworkInformation();
    IDeviceInformation* GetDeviceInformation();

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
                m_condition.wait(lock, [&]()->bool {return m_bFlag;});
                return true;
            }

            auto crRelTime = std::chrono::milliseconds(millis);
            std::unique_lock<std::mutex> ulock(m_mutex);
            if (!m_condition.wait_for(ulock, crRelTime, [&]()->bool {return m_bFlag;}))
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

        inline bool IsSet() const {return m_bFlag;}

    };

    // Worker thread

    class DeferredCallbackHandle;

    namespace detail {

        class WorkerThreadItem
        {
        public:
            volatile enum {Shutdown, Call, TimedCall, Done}type;
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
        extern void cancelWorkerThreadItem(detail::WorkerThreadItemPtr item);
    } // namespace detail

    class DeferredCallbackHandle
    {
    public:
        detail::WorkerThreadItemPtr m_item;

        DeferredCallbackHandle(detail::WorkerThreadItemPtr item) : m_item(item) {};
        DeferredCallbackHandle() : m_item(nullptr) {};
        DeferredCallbackHandle(const DeferredCallbackHandle& h) : m_item(h.m_item) {};
        void cancel()
        {
            if (m_item)
            {
                detail::cancelWorkerThreadItem(m_item);
                m_item = nullptr;
            }
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

    // *INDENT-ON*

    //
    // Miscellaneous
    //

    // Return a new random UUID in a lowercase hexadecimal format with dashes and
    // without curly braces (xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx).
    extern std::string generateUuidString();

    // Pseudo-random number generator (not for cryptographic usage).
    // The instances are not thread-safe, serialize access externally if needed.
    class PseudoRandomGenerator {
#ifdef _WIN32
    public:
        double getRandomDouble()
        {
            return m_distribution(m_engine);
        }

    protected:
        std::default_random_engine m_engine {std::random_device()()};
        std::uniform_real_distribution<double> m_distribution {0.0, 1.0};
#else   /* Unfortunately the functionality above fails memory checker on Linux with gcc-5 */
    public:
        double getRandomDouble()
        {
            return ((double)rand() / RAND_MAX);
        }
#endif
    };

    // Return the current system time in milliseconds (since the UNIX epoch - Jan 1, 1970).
    extern int64_t getUtcSystemTimeMs();

    extern int64_t getUtcSystemTimeinTicks();

    extern int64_t getUtcSystemTime();

    // Convert given system timestamp in milliseconds to a string in ISO 8601 format
    std::string formatUtcTimestampMsAsISO8601(int64_t timestampMs);

    // Populate per-platform fields in ISemanticContext and keep them updated during runtime.
    void registerSemanticContext(ISemanticContext * context);
    void unregisterSemanticContext(ISemanticContext * context);

    // Convert various numeric types and bool to string in an uniform manner.
    template<typename T>// , typename Check = std::enable_if<std::is_arithmetic<T>::value || std::is_same<T, bool>::value, void>::type>
    std::string to_string(char const* format, T value)
    {
        // Max value for unsigned long is 18446744073709551615 (20 characters).
        // This function is private so it will only be called for numeric types.
        static const int buf_size = 33;
        char buf[buf_size] = {0};
        int rc = snprintf(buf, buf_size, format, value);
        (rc);
        return std::string(buf);
    }

    // Return SDK version in Aria schema "<Prefix>-<Platform>-<SKU>-<Projection>-<BuildVersion>".
    std::string getSdkVersion();

} PAL_NS_END

#endif
