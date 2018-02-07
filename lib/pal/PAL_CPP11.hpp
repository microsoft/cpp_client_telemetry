// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "SystemInformationImpl.hpp"
#include "NetworkInformationImpl.hpp"
#include "DeviceInformationImpl.hpp"
#include <ISemanticContext.hpp>

#include <atomic>
#include <condition_variable>
#include <climits>
#include <chrono>
#include <thread>

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
#include "typename.hpp"


namespace ARIASDK_NS_BEGIN {

    extern void print_backtrace();

    namespace
        PAL {

#ifdef _WIN32
#define PATH_SEPARATOR_CHAR '\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#endif

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
// Reference-counted objects
//

// Smart pointer for reference-counted objects
        template<typename T>
        class RefCountedPtr
        {
        protected:
            T* m_ptr;

        public:
            RefCountedPtr()
                : m_ptr(nullptr)
            {
            }

            explicit RefCountedPtr(T* ptr, bool addRef)
                : m_ptr(ptr)
            {
                if (m_ptr && addRef) {
                    m_ptr->addRef();
                }
            }

            RefCountedPtr(RefCountedPtr<T> const& other)
                : m_ptr(other.get())
            {
                if (m_ptr) {
                    m_ptr->addRef();
                }
            }

            template<typename TOther>
            RefCountedPtr(RefCountedPtr<TOther> const& other)
                : m_ptr(other.get())
            {
                if (m_ptr) {
                    m_ptr->addRef();
                }
            }

            RefCountedPtr<T>& operator=(RefCountedPtr<T> const& other)
            {
                if (m_ptr != other.get()) {
                    reset(other.get(), true);
                }
                return *this;
            }

            template<typename TOther>
            RefCountedPtr<T>& operator=(RefCountedPtr<TOther> const& other)
            {
                if (m_ptr != other.get()) {
                    reset(other.get(), true);
                }
                return *this;
            }

            void delete_check()
            {
#ifdef USE_REFCOUNTER
                auto &trk = refCountedTracker[__typename(m_ptr)];
                auto &delCount = std::get<3>(trk);
                if (m_ptr->use_count() == 1)
                    delCount++;
#endif
            }

            ~RefCountedPtr()
            {
                if (m_ptr) {
                    delete_check();
                    m_ptr->release();
                }
            }

            void reset()
            {
                if (m_ptr) {
#if 0
                    delete_check();
                    if (m_ptr->use_count() != 1) {
                        Microsoft::Applications::Telemetry::print_backtrace();
                    }
#endif
                    m_ptr->release();
                    m_ptr = nullptr;
                }
            }

            void reset(T* ptr, bool addRef)
            {
                assert(ptr == nullptr || ptr != m_ptr);
                if (m_ptr) {
#if 0
                    delete_check();
                    if (m_ptr->use_count() != 1) {
                        Microsoft::Applications::Telemetry::print_backtrace();
                    }
#endif
                    m_ptr->release();
                }
                m_ptr = ptr;
                if (m_ptr && addRef) {
                    m_ptr->addRef();
                }
            }

            T* get() const
            {
                return m_ptr;
            }

            T* operator->() const
            {
                return m_ptr;
            }

            T& operator*() const
            {
                return *m_ptr;
            }

            explicit operator bool() const
            {
                return (m_ptr != nullptr);
            }

            template<typename TOther>
            bool operator==(RefCountedPtr<TOther> const& other) const
            {
                return (m_ptr == other.get());
            }

            template<typename TOther>
            bool operator!=(RefCountedPtr<TOther> const& other) const
            {
                return (m_ptr != other.get());
            }

            template<typename TOther>
            bool operator<(RefCountedPtr<TOther> const& other) const
            {
                return (m_ptr < other.get());
            }
        };

        // Internal base interface for reference-counted objects
        // Separated from IRefCounted so that the virtual inheritance can be hidden inside PAL.
        namespace detail {
            class IRefCountedBase
            {
            public:
                IRefCountedBase() {}
                virtual ~IRefCountedBase() {}
                IRefCountedBase(IRefCountedBase const&) = delete;
                IRefCountedBase& operator=(IRefCountedBase const&) = delete;

                virtual void addRef() = 0;
                virtual void release() = 0;
                virtual long use_count() const noexcept = 0;

                template<typename TAny>
                friend class RefCountedPtr;
            };
        } // namespace detail

        // Interface for reference-counted objects
        class IRefCounted : public virtual detail::IRefCountedBase {};

        // Base class implementing a reference-counted object
        template<typename T, typename TInterface1 = IRefCounted , typename... TOtherInterfaces>
        class RefCountedImpl : public TInterface1, public TOtherInterfaces...
        {

        protected:
        
            mutable volatile std::atomic<long> m_refCount;

        public:

            long use_count() const noexcept override
            {
                return m_refCount;
            }

            // *INDENT-OFF* parameter pack expansions etc.
            template<typename... TArgs> static RefCountedPtr<T> create(TArgs&&... args)
            {
                auto ptr = new T(std::forward<TArgs>(args)...);
#ifdef USE_REFCOUNTER
                auto &trk = refCountedTracker[__typename(ptr)];
                auto &newCount = std::get<0>(trk);
                newCount++;
#endif
                return RefCountedPtr<T>(ptr, false);
            }
            // *INDENT-ON*

            virtual ~RefCountedImpl()
            {
#ifdef USE_REFCOUNTER
                auto &trk = refCountedTracker[__typename(this)];
                auto &delCount = std::get<3>(trk);
                delCount++;
#endif
            }

        protected:

            RefCountedImpl()
                : m_refCount(1)
            {
#ifdef USE_REFCOUNTER
                auto &trk = refCountedTracker[__typename(this)];
                auto &newCount = std::get<0>(trk);
                newCount++;

                //
                auto &incCount = std::get<1>(trk);
                incCount++;
#endif
            }

            RefCountedPtr<T> self()
            {
                return RefCountedPtr<T>(static_cast<T*>(this), true);
            }

            virtual void addRef() override
            {
                m_refCount++;
#ifdef USE_REFCOUNTER
                auto &trk = refCountedTracker[__typename(this)];
                auto &incCount = std::get<1>(trk);
                incCount++;
#endif
            }

            virtual void release() override
            {
                long count = (--m_refCount);
                assert(count >= 0);

#ifdef USE_REFCOUNTER
                auto &trk = refCountedTracker[__typename(this)];
                auto &decCount = std::get<2>(trk);
                decCount++;
#endif
				if (count == 0) {
					delete this;
				}
            }

            friend class RefCountedPtr<T>;
        };

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

            bool wait(unsigned msec = UINT_MAX) const
            {
                if (msec == UINT_MAX)
                {
                    std::unique_lock< std::mutex > lock(m_mutex);
                    m_condition.wait(lock, [&]()->bool {return m_bFlag; });
                    return true;
                }

                auto crRelTime = std::chrono::microseconds(msec);
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

            class WorkerThreadItem: public RefCountedImpl<WorkerThreadItem>
            {
            public:
                volatile enum { Shutdown, Call, TimedCall, Done }type;
                int64_t targetTime;
                virtual ~WorkerThreadItem() {}
                virtual void operator()() {}
            };

            using WorkerThreadItemPtr = RefCountedPtr<WorkerThreadItem>;

            template<typename TCall>
			class WorkerThreadCall : public RefCountedImpl< WorkerThreadCall<TCall> , WorkerThreadItem > /* , public WorkerThreadItem */
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

            extern void queueWorkerThreadItem(detail::WorkerThreadItemPtr const& item);
            extern void cancelWorkerThreadItem(detail::WorkerThreadItemPtr const& item);

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
        	// ARIASDK_LOG_INFO(">>>>>>>>>>>>>>>>>>>>> executeOnWorkerThread: %s (%p)", __typename(obj), obj);
            static_assert(std::is_convertible<TObject*, detail::IRefCountedBase*>::value, "Callback object must inherit from PAL::IRefCounted or PAL::RefCountedImpl");

            auto bound = std::bind(std::mem_fn(func), obj, std::forward<TPassedArgs>(args)...);
            auto item = detail::WorkerThreadCall<decltype(bound)>::create(obj, bound);
            detail::queueWorkerThreadItem(item);
        }

        template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
        void executeOnWorkerThread(PAL::RefCountedPtr<TObject> const& obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
        {
            executeOnWorkerThread(obj.get(), func, std::forward<TPassedArgs>(args)...);
        }

        // Return the monotonic system clock time in milliseconds (since unspecified point).
        extern int64_t getMonotonicTimeMs();

        template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
        DeferredCallbackHandle scheduleOnWorkerThread(unsigned delayMs, TObject* obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
        {
        	// ARIASDK_LOG_INFO(">>>>>>>>>>>>>>>>>>>>> scheduleOnWorkerThread: %s (%p)", __typename(obj), obj);

            static_assert(std::is_convertible<TObject*, detail::IRefCountedBase*>::value, "Callback object must inherit from PAL::IRefCounted or PAL::RefCountedImpl");
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
#if 0
        public:
            double getRandomDouble()
            {
                return m_distribution(m_engine);
            }

        protected:
            std::default_random_engine m_engine{ std::random_device()() };
            std::uniform_real_distribution<double> m_distribution{ 0.0, 1.0 };
#else
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

    } // namespace PAL
}ARIASDK_NS_END
