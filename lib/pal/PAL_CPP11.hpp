// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "SystemInformationImpl.hpp"
#include "NetworkInformationImpl.hpp"
#include "DeviceInformationImpl.hpp"

#include <ISemanticContext.hpp>

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
}

#ifdef _WIN32
#define PATH_SEPARATOR_CHAR '\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#endif

namespace PAL_NS_BEGIN {

    class INetworkInformation;
    class IDeviceInformation;
    
#ifdef USE_REFCOUNTER
        extern std::map<std::string, std::tuple<
            //size_t, size_t, size_t, size_t
            std::atomic<size_t>,
            std::atomic<size_t>,
            std::atomic<size_t>,
            std::atomic<size_t>
        > > refCountedTracker;
#endif

        //
        // Startup/shutdown
        //

        void initialize();
        void shutdown();
        INetworkInformation* GetNetworkInformation();
        IDeviceInformation* GetDeviceInformation();

        //
        // Threading
        //

        // Basic non-recursive inter-process mutex
        // For use through ScopedMutexLock only.
        class Mutex
        {
        protected:
            std::mutex m_cs;

        public:
            Mutex()
            {
            }

            ~Mutex()
            {
            }

        protected:
            void lock()
            {
                m_cs.lock();
            }

            void unlock()
            {
                m_cs.unlock();
            }

            friend class ScopedMutexLock;
        };

        // RAII object for guarding a block of code
        class ScopedMutexLock
        {
        protected:
            Mutex& m_mutex;

        public:
            ScopedMutexLock(Mutex& mutex)
                : m_mutex(mutex)
            {
                m_mutex.lock();
            }

            ~ScopedMutexLock()
            {
                m_mutex.unlock();
            }
        };

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

        // Worker thread

        namespace detail {

            class WorkerThreadItem
            {
            public:
                volatile enum { Shutdown, Call, TimedCall, Done }type;
                int64_t targetTime;
                virtual ~WorkerThreadItem() {}
                virtual void operator()() {}
            };

            typedef WorkerThreadItemPtr WorkerThreadItem*;

            template<typename TCall>
	    class WorkerThreadCall
            {
            public:

                WorkerThreadCall(IRefCountedBase* obj, TCall call)
                    : m_holder(obj, true),
                    m_call(std::forward<TCall>(call))
                {
                    this->type = WorkerThreadItem::Call;
                }

                WorkerThreadCall(IRefCountedBase* obj, int64_t targetTime, TCall call)
                    : m_holder(obj, true),
                    m_call(std::forward<TCall>(call))
                {
                    this->type = WorkerThreadItem::TimedCall;
                    this->targetTime = targetTime;
                }

                virtual void operator()() override
                {
                    m_call();
                }

                virtual ~WorkerThreadCall()
                {
                    m_holder.reset();
                }

            protected:
                RefCountedPtr<IRefCountedBase> m_holder;
                TCall m_call;
            };

        } // namespace detail


        class DeferredCallbackHandle;

        namespace detail {

            extern void queueWorkerThreadItem(detail::WorkerThreadItemPtr item);
            extern void cancelWorkerThreadItem(detail::WorkerThreadItemPtr item);

        } // namespace detail

        class DeferredCallbackHandle
        {
        protected:
            detail::WorkerThreadItemPtr m_item;

        public:
            DeferredCallbackHandle()
            {
            }

            DeferredCallbackHandle(detail::WorkerThreadItemPtr const& item)
                : m_item(item)
            {
            }

            DeferredCallbackHandle(detail::WorkerThreadItemPtr & item)
                : m_item(item)
            {
            }

            explicit operator bool() const
            {
                return m_item && (m_item->type != detail::WorkerThreadItem::Done);
            }

            void cancel()
            {
                if (m_item) {
                    detail::cancelWorkerThreadItem(m_item);
                    m_item.reset();
                }
            }

            void reset()
            {
                m_item.reset();
            }
        };


        // *INDENT-OFF* parameter pack expansions etc.

        template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
        void executeOnWorkerThread(TObject* obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
        {
            auto bound = std::bind(std::mem_fn(func), obj, std::forward<TPassedArgs>(args)...);
            auto item = detail::WorkerThreadCall<decltype(bound)>::create(obj, bound);
            detail::queueWorkerThreadItem(item);
        }

        // Return the monotonic system clock time in milliseconds (since unspecified point).
        extern int64_t getMonotonicTimeMs();

        template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
        DeferredCallbackHandle scheduleOnWorkerThread(unsigned delayMs, TObject* obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
        {
            auto bound = std::bind(std::mem_fn(func), obj, std::forward<TPassedArgs>(args)...);
            auto item = detail::WorkerThreadCall<decltype(bound)>::create(obj, getMonotonicTimeMs() + delayMs, bound);
            detail::queueWorkerThreadItem(item);
            // TODO: check for leaks
            return DeferredCallbackHandle(item);
        }

        template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
        DeferredCallbackHandle scheduleOnWorkerThread(unsigned delayMs, PAL::RefCountedPtr<TObject> const& obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
        {
            return scheduleOnWorkerThread(delayMs, obj.get(), func, std::forward<TPassedArgs>(args)...);
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
            std::default_random_engine m_engine{ std::random_device()() };
            std::uniform_real_distribution<double> m_distribution{ 0.0, 1.0 };
#else   /* Unfortunately the functionality above fails memory checker on Linux with gcc-5 */
        public:
            double getRandomDouble()
            {
                return ((double)rand()/RAND_MAX);
            }
#endif
        };

        // Return the current system time in milliseconds (since the UNIX epoch - Jan 1, 1970).
        extern int64_t getUtcSystemTimeMs();

        extern int64_t getUtcSystemTime();

        // Convert given system timestamp in milliseconds to a string in ISO 8601 format
        std::string formatUtcTimestampMsAsISO8601(int64_t timestampMs);

        // Populate per-platform fields in ISemanticContext and keep them updated during runtime.
        void registerSemanticContext(ISemanticContext * context);
        void unregisterSemanticContext(ISemanticContext * context);

        template <typename T>
        std::string to_string(const char* pFormatter, const T& value)
        {
            // Max value for unsigned long is 18446744073709551615 (20 characters).
            // This function is private so it will only be called for numeric types.
            static const int buf_size = 33;
            char buf[buf_size] = { 0 };
            int rc = snprintf(buf, buf_size, pFormatter, value);
            (rc);
            return std::string(buf);
        }

        // Return SDK version in Aria schema "<Prefix>-<Platform>-<SKU>-<Projection>-<BuildVersion>".
        std::string getSdkVersion();

} PAL_NS_END

