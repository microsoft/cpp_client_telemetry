// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <aria/ISemanticContext.hpp>
#include <aria/Utils.hpp>
#include <auf/auf.hpp>
#include <auf/auf_random_xorshift.hpp>
#include <auf/auf_uuid.hpp>
#include <spl/spl_macros.hpp>
#include <spl/spl_sysinfo.hpp>
#include <stdint.h>
#include <sstream>
#include <type_traits>
#include <sqlite/sqlite3.h>


namespace PAL_NS_BEGIN {

//
// Startup/shutdown
//

void initialize();
void shutdown();


//
// Debug logging
//

// Declare/define log component for a namespace
#define ARIASDK_LOG_DECL_COMPONENT_NS()                        AUF_LOG_DECL_COMPONENT_NS()
#define ARIASDK_LOG_INST_COMPONENT_NS(_name, _desc)            \
    AUF_LOG_DECL_COMPONENT_DESCRIPTION(_name, _desc);          \
    AUF_LOG_DECL_COMPONENT_SAFE(_name, true);                  \
    AUF_LOG_INST_COMPONENT_NS(_name)

// Declare/define log component for a class
#define ARIASDK_LOG_DECL_COMPONENT_CLASS()                     AUF_LOG_DECL_COMPONENT_CLASS()
#define ARIASDK_LOG_INST_COMPONENT_CLASS(_class, _name, _desc) \
    AUF_LOG_DECL_COMPONENT_DESCRIPTION(_name, _desc);          \
    AUF_LOG_DECL_COMPONENT_SAFE(_name, true);                  \
    AUF_LOG_INST_COMPONENT_CLASS(_class, _name)

// Declare log component for the PAL namespace
ARIASDK_LOG_DECL_COMPONENT_NS();

// Check if logging is enabled on a specific level
#define ARIASDK_LOG_ENABLED_DETAIL()   AUF_LOG_ENABLED_DEBUG4()
#define ARIASDK_LOG_ENABLED_INFO()     AUF_LOG_ENABLED_DEBUG3()
#define ARIASDK_LOG_ENABLED_WARNING()  AUF_LOG_ENABLED_DEBUG2()
#define ARIASDK_LOG_ENABLED_ERROR()    AUF_LOG_ENABLED_WARN()

// Log a message on a specific level, which is checked efficiently before evaluating arguments
#define LOG_TRACE(fmt_, ...)  AUF_LOG_DEBUG4(fmt_, ##__VA_ARGS__)
#define LOG_INFO(fmt_, ...)    AUF_LOG_DEBUG3(fmt_, ##__VA_ARGS__)
#define LOG_WARN(fmt_, ...) AUF_LOG_DEBUG2(fmt_, ##__VA_ARGS__)
#define LOG_ERROR(fmt_, ...)   AUF_LOG_WARN(fmt_, ##__VA_ARGS__)


//
// Reference-counted objects
//

// Smart pointer for reference-counted objects
template<typename T>
class RefCountedPtr : public auf::IntrusivePtr<T>
{
  public:
    RefCountedPtr()
    {
    }

    explicit RefCountedPtr(T* ptr, bool addRef)
      : auf::IntrusivePtr<T>(ptr, addRef)
    {
    }

    template<typename TOther>
    RefCountedPtr(RefCountedPtr<TOther> const& other)
      : auf::IntrusivePtr<T>(other)
    {
    }

    template<typename TOther>
    RefCountedPtr& operator=(RefCountedPtr<TOther> const& other)
    {
        auf::IntrusivePtr<T>::operator=(other);
        return *this;
    }

    void reset()
    {
        auf::IntrusivePtr<T>::reset();
    }

    void reset(T* ptr, bool addRef)
    {
        auf::IntrusivePtr<T>::reset(ptr, addRef);
    }

    explicit operator bool() const
    {
        return (this->get() != nullptr);
    }

  private:
    using auf::IntrusivePtr<T>::useCount;
    using auf::IntrusivePtr<T>::unique;
};

// Interface for reference-counted objects
class IRefCounted : public virtual auf::IReferenceCountable
{
  private:
    template<typename TAny>
    friend class auf::IntrusivePtr;

    using auf::IReferenceCountable::retainRef;
    using auf::IReferenceCountable::releaseRef;
    using auf::IReferenceCountable::refCount;
    using auf::IReferenceCountable::createWeakRef;
    using auf::IReferenceCountable::releaseWeakRef;
    using auf::IReferenceCountable::conditionalRetain;
    using auf::IReferenceCountable::onFinalRelease;
};

// Base class implementing a reference-counted object
template<typename T, typename TInterface1 = IRefCounted, typename... TOtherInterfaces>
class RefCountedImpl : public virtual auf::Object,
                       public TInterface1,
                       public TOtherInterfaces...
{
  public:
    // *INDENT-OFF* parameter pack expansions etc.
    template<typename... TArgs> static RefCountedPtr<T> create(TArgs&&... args)
    {
        return RefCountedPtr<T>(new T(std::forward<TArgs>(args)...), false);
    }
    // *INDENT-ON*

  protected:
    RefCountedPtr<T> self()
    {
        return RefCountedPtr<T>(static_cast<T*>(this), true);
    }

  private:
    using auf::IReferenceCountable::retainRef;
    using auf::IReferenceCountable::releaseRef;
    using auf::IReferenceCountable::refCount;
    using auf::IReferenceCountable::createWeakRef;
    using auf::IReferenceCountable::releaseWeakRef;
    using auf::IReferenceCountable::conditionalRetain;
    using auf::IReferenceCountable::onFinalRelease;

    using auf::CounterBase::retain;
    using auf::CounterBase::count;
    using auf::CounterBase::unique;

    using auf::WeaklyReferencableCounterBase::release;

    using auf::Object::setLoggingPrefix;
    using auf::Object::loggingPrefix;
    using auf::Object::logv;
};


//
// Threading
//

// Basic non-recursive inter-process mutex. For use through ScopedMutexLock only.
class Mutex : protected auf::Mutex
{
  public:
    Mutex(char const* name)
      : auf::Mutex(name, false)
    {
    }

    friend class ScopedMutexLock;
};

// RAII object for guarding a block of code.
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
class Event : protected auf::Event
{
  public:
    Event()
    {
    }

    void post()
    {
        auf::Event::post();
    }

    void wait()
    {
        auf::Event::wait(SPL_USECS_MAX);
    }
};


// Worker thread

class DeferredCallbackHandle
{
  protected:
    auf::TimerPtr m_timer;

  public:
    DeferredCallbackHandle()
    {
    }

    DeferredCallbackHandle(auf::TimerPtr const& timer)
      : m_timer(timer)
    {
    }

    DeferredCallbackHandle(DeferredCallbackHandle const& other) = default;

    DeferredCallbackHandle& operator=(DeferredCallbackHandle const& other) = default;

    explicit operator bool() const
    {
        return m_timer && !m_timer->expired();
    }

    void cancel()
    {
        if (m_timer) {
            m_timer->cancelSync();
            m_timer.reset();
        }
    }

    void reset()
    {
        m_timer.reset();
    }
};

extern auf::ThreadPoolTransportPtr g_strand;

// *INDENT-OFF* parameter pack expansions etc.

template<typename TObject, typename... TArgs, typename... TValues>
inline void executeOnWorkerThread(TObject* obj, void (TObject::*callback)(TArgs...), TValues&&... values)
{
    static_assert(std::is_convertible<TObject*, auf::IReferenceCountable*>::value,
        "Callback object must inherit from PAL::IRefCounted or PAL::RefCountedImpl");
    auf::callAsync(*g_strand, *obj, callback, std::forward<TValues>(values)...);
}

template<typename TObject, typename... TArgs, typename... TValues>
inline void executeOnWorkerThread(PAL::RefCountedPtr<TObject> const& obj, void (TObject::*callback)(TArgs...), TValues&&... values)
{
    executeOnWorkerThread(obj.get(), callback, std::forward<TValues>(values)...);
}

template<typename TObject, typename... TArgs, typename... TValues>
inline DeferredCallbackHandle scheduleOnWorkerThread(unsigned delayMs, TObject* obj, void (TObject::*callback)(TArgs...), TValues&&... values)
{
    static_assert(std::is_convertible<TObject*, auf::IReferenceCountable*>::value,
        "Callback object must inherit from PAL::IRefCounted or PAL::RefCountedImpl");
    auf::LockfreePacker packer;
    auf::Call* call = auf::createCall(packer, callback, *obj, std::forward<TValues>(values)...);
    return auf::createTimerWithTransport(g_strand, delayMs * 1000, 0, call);
}

template<typename TObject, typename... TArgs, typename... TValues>
inline DeferredCallbackHandle scheduleOnWorkerThread(unsigned delayMs, PAL::RefCountedPtr<TObject> const& obj, void (TObject::*callback)(TArgs...), TValues&&... values)
{
    return scheduleOnWorkerThread(delayMs, obj.get(), callback, std::forward<TValues>(values)...);
}

// *INDENT-ON*


//
// Miscellaneous
//

// Return a new random UUID in a lowercase hexadecimal format with dashes and
// without curly braces (xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx).
inline std::string generateUuidString()
{
    auf::UUID::StringType buf;
    auf::UUID::createWithRNG().toString(buf);
    return buf;
}

// Pseudo-random number generator (not for cryptographic usage).
// The instances are not thread-safe, serialize access externally if needed.
class PseudoRandomGenerator {
  public:
    double getRandomDouble()
    {
        return m_generator.randomDouble();
    }

  protected:
    auf::XorshiftRNG m_generator;
};

// Return the current system time in milliseconds (since the UNIX epoch - Jan 1, 1970).
inline int64_t getUtcSystemTimeMs()
{
    return spl::utcHpTimestamp() / 10000;
}

// Convert given system timestamp in milliseconds to a string in ISO 8601 format
std::string formatUtcTimestampMsAsISO8601(int64_t timestampMs);

// Return the monotonic system clock time in milliseconds (since unspecified point).
inline int64_t getMonotonicTimeMs()
{
    return spl::msFromHp(spl::highPrecisionTimestamp());
}

// Delay execution for specified number of milliseconds. Generally for testing code only.
inline void sleep(unsigned delayMs)
{
    spl::sleep(delayMs * 1000);
}

// Populate per-platform fields in ISemanticContext and keep them updated during runtime.
void registerSemanticContext(ISemanticContext* context);
void unregisterSemanticContext(ISemanticContext* context);

// Convert various numeric types and bool to string in an uniform manner.
template<typename T, typename Check = typename std::enable_if<std::is_arithmetic<T>::value || std::is_same<T, bool>::value, void>::type>
std::string to_string(char const* format, T value)
{
    char buf[40];
    spl::snprintf_s(buf, sizeof(buf), format, value);
    return buf;
}

// Return SDK version in Aria schema "<Prefix>-<Platform>-<SKU>-<Projection>-<BuildVersion>".
std::string getSdkVersion();


} PAL_NS_END
