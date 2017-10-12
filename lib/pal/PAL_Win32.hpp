// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "SystemInformationImpl.hpp"
#include "NetworkInformationImpl.hpp"
#include "DeviceInformationImpl.hpp"
#include <ISemanticContext.hpp>
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#undef max
#undef min
#include <Rpc.h>
#include <assert.h>
#include <stdint.h>
#include <functional>
#include <list>
#include <map>
#include <random>
#include <string>
#include <type_traits>
#include <mutex>
//#include <winsqlite/winsqlite3.h>




namespace ARIASDK_NS_BEGIN {
namespace PAL {

#define PATH_SEPARATOR_CHAR '\\'


//
// Startup/shutdown
//

void initialize();
void shutdown();
INetworkInformation* GetNetworkInformation();
IDeviceInformation* GetDeviceInformation();


//
// Debug logging
//

// *INDENT-OFF*

// Declare/define log component for a namespace
#define ARIASDK_LOG_DECL_COMPONENT_NS()                        extern const char* getAriaSdkLogComponent()
#define ARIASDK_LOG_INST_COMPONENT_NS(_name, _desc)            char const* getAriaSdkLogComponent() { return _name; }

// Declare/define log component for a class
#define ARIASDK_LOG_DECL_COMPONENT_CLASS()                     static char const* getAriaSdkLogComponent()
#define ARIASDK_LOG_INST_COMPONENT_CLASS(_class, _name, _desc) char const* _class::getAriaSdkLogComponent() { return _name; }

// *INDENT-ON*

// Declare log component for the PAL namespace
ARIASDK_LOG_DECL_COMPONENT_NS();

enum LogLevel {
    Error    = 1,
    Warning  = 2,
    Info     = 3,
    Detail   = 4
};

namespace detail {

extern LogLevel g_logLevel;
extern void log(LogLevel level, char const* component, char const* fmt, ...);

} // namespace detail

#define ARIASDK_SET_LOG_LEVEL_(level_) \
    (::ARIASDK_NS::PAL::detail::g_logLevel = (level_))
// Check if logging is enabled on a specific level
#define ARIASDK_LOG_ENABLED_(level_) \
    (::ARIASDK_NS::PAL::detail::g_logLevel >= (level_))
#define ARIASDK_LOG_ENABLED_DETAIL()   ARIASDK_LOG_ENABLED_(::ARIASDK_NS::PAL::Detail)
#define ARIASDK_LOG_ENABLED_INFO()     ARIASDK_LOG_ENABLED_(::ARIASDK_NS::PAL::Info)
#define ARIASDK_LOG_ENABLED_WARNING()  ARIASDK_LOG_ENABLED_(::ARIASDK_NS::PAL::Warning)
#define ARIASDK_LOG_ENABLED_ERROR()    ARIASDK_LOG_ENABLED_(::ARIASDK_NS::PAL::Error)

// Log a message on a specific level, which is checked efficiently before evaluating arguments
#define ARIASDK_LOG_(level_, comp_, fmt_, ...)                                    \
    if (ARIASDK_LOG_ENABLED_(level_)) {                                           \
        ::ARIASDK_NS::PAL::detail::log((level_), (comp_), (fmt_), ##__VA_ARGS__); \
    } else static_cast<void>(0)
#define ARIASDK_LOG_DETAIL(fmt_, ...)  ARIASDK_LOG_(::ARIASDK_NS::PAL::Detail,  getAriaSdkLogComponent(), fmt_, ##__VA_ARGS__)
#define ARIASDK_LOG_INFO(fmt_, ...)    ARIASDK_LOG_(::ARIASDK_NS::PAL::Info,    getAriaSdkLogComponent(), fmt_, ##__VA_ARGS__)
#define ARIASDK_LOG_WARNING(fmt_, ...) ARIASDK_LOG_(::ARIASDK_NS::PAL::Warning, getAriaSdkLogComponent(), fmt_, ##__VA_ARGS__)
#define ARIASDK_LOG_ERROR(fmt_, ...)   ARIASDK_LOG_(::ARIASDK_NS::PAL::Error,   getAriaSdkLogComponent(), fmt_, ##__VA_ARGS__)


//
// Reference-counted objects
//

// Smart pointer for reference-counted objects
template<typename T>
class RefCountedPtr
{
  private:
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

    ~RefCountedPtr()
    {
        if (m_ptr) {
            m_ptr->release();
        }
    }

    void reset()
    {
        if (m_ptr) {
            m_ptr->release();
            m_ptr = nullptr;
        }
    }

    void reset(T* ptr, bool addRef)
    {
        assert(ptr == nullptr || ptr != m_ptr);
        if (m_ptr) {
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

  private:
    virtual void addRef() = 0;
    virtual void release() = 0;
    template<typename TAny>
    friend class RefCountedPtr;
};
} // namespace detail

// Interface for reference-counted objects
class IRefCounted : public virtual detail::IRefCountedBase {};

// Base class implementing a reference-counted object
template<typename T, typename TInterface1 = IRefCounted, typename... TOtherInterfaces>
class RefCountedImpl : public TInterface1,
                       public TOtherInterfaces...
{
  private:
    volatile LONG m_refCount;

  public:
    // *INDENT-OFF* parameter pack expansions etc.
    template<typename... TArgs> static RefCountedPtr<T> create(TArgs&&... args)
    {
        return RefCountedPtr<T>(new T(std::forward<TArgs>(args)...), false);
    }
    // *INDENT-ON*

  protected:
    RefCountedImpl()
      : m_refCount(1)
    {
    }

    RefCountedPtr<T> self()
    {
        return RefCountedPtr<T>(static_cast<T*>(this), true);
    }

  private:
    virtual void addRef() override
    {
        InterlockedIncrement(&m_refCount);
    }

    virtual void release() override
    {
        long now = InterlockedDecrement(&m_refCount);
        if (now == 0) {
            delete this;
        }
    }

    template<typename TAny>
    friend class RefCountedPtr;
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
    HANDLE m_hEvent;

  public:
    Event()
      : m_hEvent(::CreateEvent(NULL, FALSE, FALSE, NULL))
    {
    }

    ~Event()
    {
        ::CloseHandle(m_hEvent);
    }

    void post()
    {
        ::SetEvent(m_hEvent);
    }

    void wait(unsigned msec = INFINITE)
    {
        ::WaitForSingleObject(m_hEvent, msec);
    }
};

// Worker thread

namespace detail {

class WorkerThreadItem : public IRefCounted
{
  public:
    volatile enum { Shutdown, Call, TimedCall, Done } type;
    int64_t targetTime;
    virtual ~WorkerThreadItem() {}
    virtual void operator()() {}
};
using WorkerThreadItemPtr = RefCountedPtr<WorkerThreadItem>;

template<typename TCall>
class WorkerThreadCall : public RefCountedImpl<WorkerThreadCall<TCall>, WorkerThreadItem>
{
  public:
    WorkerThreadCall(IRefCountedBase* obj, TCall call)
      : m_holder(obj, true),
        m_call(std::forward<TCall>(call))
    {
        type = Call;
    }

    WorkerThreadCall(IRefCountedBase* obj, int64_t targetTime, TCall call)
      : m_holder(obj, true),
        m_call(std::forward<TCall>(call))
    {
        type = TimedCall;
        this->targetTime = targetTime;
    }

    virtual void operator()() override
    {
        m_call();
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
    static_assert(std::is_convertible<TObject*, detail::IRefCountedBase*>::value,
        "Callback object must inherit from PAL::IRefCounted or PAL::RefCountedImpl");
    auto bound = std::bind(std::mem_fn(func), obj, std::forward<TPassedArgs>(args)...);
    auto item = detail::WorkerThreadCall<decltype(bound)>::create(obj, bound);
    detail::queueWorkerThreadItem(item);
}

template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
void executeOnWorkerThread(PAL::RefCountedPtr<TObject> const& obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
{
    executeOnWorkerThread(obj.get(), func, std::forward<TPassedArgs>(args)...);
}

template<typename TObject, typename... TFuncArgs, typename... TPassedArgs>
DeferredCallbackHandle scheduleOnWorkerThread(unsigned delayMs, TObject* obj, void (TObject::*func)(TFuncArgs...), TPassedArgs&&... args)
{
    static_assert(std::is_convertible<TObject*, detail::IRefCountedBase*>::value,
        "Callback object must inherit from PAL::IRefCounted or PAL::RefCountedImpl");
    auto bound = std::bind(std::mem_fn(func), obj, std::forward<TPassedArgs>(args)...);
    auto item = detail::WorkerThreadCall<decltype(bound)>::create(obj, getMonotonicTimeMs() + delayMs, bound);
    detail::queueWorkerThreadItem(item);
    return item;
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
  public:
    double getRandomDouble()
    {
        return m_distribution(m_engine);
    }

  protected:
    std::default_random_engine             m_engine{std::random_device()()};
    std::uniform_real_distribution<double> m_distribution{0.0, 1.0};
};

// Return the current system time in milliseconds (since the UNIX epoch - Jan 1, 1970).
extern int64_t getUtcSystemTimeinTicks();
extern int64_t getUtcSystemTimeMs();
extern int64_t getUtcSystemTime();

// Convert given system timestamp in milliseconds to a string in ISO 8601 format
std::string formatUtcTimestampMsAsISO8601(int64_t timestampMs);

// Return the monotonic system clock time in milliseconds (since unspecified point).
extern int64_t getMonotonicTimeMs();

// Delay execution for specified number of milliseconds. Generally for testing code only.
inline void sleep(unsigned delayMs)
{
    ::Sleep(delayMs);
}

// Populate per-platform fields in ISemanticContext and keep them updated during runtime.
void registerSemanticContext(ISemanticContext * context);
void unregisterSemanticContext(ISemanticContext * context);

// Convert various numeric types and bool to string in an uniform manner.
template<typename T, typename Check = std::enable_if<std::is_arithmetic<T>::value || std::is_same<T, bool>::value, void>::type>
std::string numericToString(char const* format, T value)
{
    char buf[40];
    ::sprintf_s(buf, format, value);
    return buf;
}

// Return SDK version in Aria schema "<Prefix>-<Platform>-<SKU>-<Projection>-<BuildVersion>".
std::string getSdkVersion();


} // namespace PAL
} ARIASDK_NS_END
