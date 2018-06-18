// Copyright (c) Microsoft. All rights reserved.

#include "PAL.hpp"
#ifdef ARIASDK_PAL_CPP11
#include "ILogManager.hpp"
#include <ISemanticContext.hpp>
#include "utils/Utils.hpp"

#include <algorithm>
#include <list>
#include <memory>
#include <chrono>
#include <thread>

#include <iostream>
#include <sstream>
#include <fstream>

#include <iomanip>

#include <stdarg.h>

#include "utils/Utils.hpp"
#include <sys/types.h>

#ifndef _WIN32
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>

#ifdef __linux__
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#endif

#endif

#if defined(_WIN32) || defined(_WIN64)
#include <IPHlpApi.h>
#include <Objbase.h>
#include <oacr.h>
#endif

#include <ctime>

#define Sleep	MAT::sleep

namespace PAL_NS_BEGIN {

    void sleep(unsigned delayMs)
    {
        Sleep(delayMs);
    }

    std::map<std::string, std::tuple<
        // size_t, size_t, size_t, size_t
        std::atomic<size_t>,        // newCount
        std::atomic<size_t>,        // incCount
        std::atomic<size_t>,        // decCount
        std::atomic<size_t>         // delCount
    > > refCountedTracker;

    void dumpRefCounted()
    {
#ifdef USE_DUMP_REFCOUNTER
        for (auto &kv : refCountedTracker)
        {
            std::string key = kv.first;
            ARIASDK_NS::replace(key, "Microsoft::Applications::Telemetry::", "MAT::");
            ARIASDK_NS::replace(key, "Microsoft::Applications::Telemetry::PAL::", "PAL::");
            auto newCount = std::get<0>(kv.second).load();
            auto incCount = std::get<1>(kv.second).load();
            auto decCount = std::get<2>(kv.second).load();
            auto delCount = std::get<3>(kv.second).load();
            fprintf(stderr, "%64s\t-%8u-%8u-%8u-%8u\n",
                key.c_str(),
                newCount,
                incCount,
                decCount,
                delCount
            );
        }
#endif
    }

    ARIASDK_LOG_INST_COMPONENT_NS("AriaSDK.PAL", "Aria telemetry client - platform abstraction layer");

    namespace detail {

        LogLevel g_logLevel = LogLevel::Detail;

#define DBG_BUFFER_LEN		2048

#ifndef _WIN32
#define DEBUG_LOG_NULL		"/dev/null"
#else
#define DEBUG_LOG_NULL		"NUL"
#endif

        static std::recursive_mutex	debugLogMutex;
        static std::string			debugLogPath;
        static std::ostream*		debugLogStream = NULL;

        static bool log_init()
        {
            bool result = true;
            // Allow classic win32 desktop to redirect the output to debug log file
            debugLogMutex.lock();
            if (debugLogStream != NULL)
                return result;
            debugLogPath = MAT::GetTempDirectory();
            debugLogPath += "aria-debug-";
            debugLogPath += std::to_string(MAT::GetCurrentProcessId());
            debugLogPath += ".log";

            std::fstream *debugFileStream = new std::fstream();
            debugFileStream->open(debugLogPath, std::fstream::out);
            if (!debugFileStream->is_open())
            {
                // If file cannot be created, log to /dev/null
                debugFileStream->open(DEBUG_LOG_NULL);
                result = false;
            }
            debugLogStream = debugFileStream;

#ifdef USE_TTY_LOG
            // Redirect to stdout
            debugLogStream = &std::cout;
#endif

            debugLogMutex.unlock();
            (*debugLogStream) << "Logging inited " << debugLogPath << " " << result << "\n";
            return result;
        }

        static bool isLoggingInited = log_init();

#if !defined(_WIN32) && defined(__linux__)
        static std::mutex m;
        static std::map<std::thread::id, pid_t> threads;
        static long int gettid()
        {
            std::lock_guard<std::mutex> l(m);
            long int tid = syscall(SYS_gettid);
            threads[std::this_thread::get_id()] = tid;
            return tid;
        }
#else
#define     gettid()       std::this_thread::get_id()
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#pragma warning(push)
#pragma warning(disable:4996)
        void log(LogLevel level, char const* component, char const* fmt, ...)
        {
            if (!isLoggingInited)
                return;

            static char const levels[] = "?EWID";
            char buffer[2048] = { 0 };

#ifdef _WIN32
            SYSTEMTIME st;
            ::GetSystemTime(&st);

            // *INDENT-OFF* ::ApiFuncName() misunderstood as object access continuation twice,
            //              std::min<size_t> misunderstood as comparison operators

            int len = ::sprintf_s(buffer, "%04u-%02u-%02u %02u:%02u:%02u.%03u T#%u <%c> [%s] ",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
                ::GetCurrentThreadId(), levels[level], component);

            va_list args;
            va_start(args, fmt);
            len += ::vsprintf_s(buffer + len, sizeof(buffer) - len, fmt, args);
            va_end(args);

            buffer[std::min<size_t>(len + 0, sizeof(buffer) - 2)] = '\n';
            buffer[std::min<size_t>(len + 1, sizeof(buffer) - 1)] = '\0';
            ::OutputDebugStringA(buffer);
#else
            auto now = std::chrono::system_clock::now();
            int64_t millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;

            ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X"); // warning C4996: 'localtime': This function or variable may be unsafe. Consider using localtime_s instead
            ss << "." << std::setfill('0') << std::setw(3) << (unsigned)(millis % 1000);
            ss << "|";

            ss << std::setfill('0') << std::setw(8) << gettid();
            ss << "|";
            ss << levels[level];
            ss << "|";
            ss << component;
            ss << "|";
            ss << fmt;

            va_list ap;
            va_start(ap, fmt);
            int len = vsnprintf(buffer, sizeof(buffer) - 1, ss.str().c_str(), ap);
            // Avoid stack corruption if vsnprintf failed to format the output
            // and returned -1
            if ((len > 0) && (len < DBG_BUFFER_LEN))
            {
                // Make sure all of our debug strings contain EOL
                buffer[len] = '\n';
                // Log to debug log file if enabled
                debugLogMutex.lock();
                if (debugLogStream->good())
                {
                    (*debugLogStream) << buffer;
                    // flush is not very efficient, but needed to get realtime file updates
                    debugLogStream->flush();
                }
                debugLogMutex.unlock();
            }
            va_end(ap);
#endif
        }
#pragma warning(pop)

    } // namespace detail


    //---


    class WorkerThreadShutdownItem : public detail::WorkerThreadItem
    {
    public:
        WorkerThreadShutdownItem()
        {
            type = detail::WorkerThreadItem::Shutdown;
        }
    };

    class WorkerThread
    {

    protected:
        std::thread                            m_hThread;

        // TODO: [MG] - investigate all the cases why we need recursive here
        std::recursive_mutex                   m_lock;

        std::list<detail::WorkerThreadItemPtr> m_queue;
        std::list<detail::WorkerThreadItemPtr> m_timerQueue;
        Event                                  m_event;
        detail::WorkerThreadItemPtr            m_itemInProgress;
        int count = 0;

    public:

        WorkerThread()
        {
            m_hThread = std::thread(WorkerThread::threadFunc, static_cast<void*>(this));
            LOG_INFO("Started new thread %u", m_hThread.get_id());
        }

        ~WorkerThread()
        {
            join();
        }

        void join()
        {
            auto item = new WorkerThreadShutdownItem();
            queue(item);
            std::thread::id this_id = std::this_thread::get_id();
            try {
                if (m_hThread.joinable() && (m_hThread.get_id() != this_id))
                    m_hThread.join();
                else
                    m_hThread.detach();
            }
            catch (...) {};
            assert(m_queue.empty());
            // FIXME: [MG] - investigate why sometimes we shutdown on non-empty queue?!
            assert(m_timerQueue.empty());
        }

        void queue(detail::WorkerThreadItemPtr item)
        {
            // TODO: [MG] - show item type
            LOG_INFO("queue item=%p", &item);
            LOCKGUARD(m_lock);
            if (item->type == detail::WorkerThreadItem::TimedCall) {
                auto it = m_timerQueue.begin();
                while (it != m_timerQueue.end() && (*it)->targetTime < item->targetTime) {
                    ++it;
                }
                m_timerQueue.insert(it, item);
            }
            else {
                m_queue.push_back(item);
            }
            count++;
            m_event.post();
        }

        void cancel(detail::WorkerThreadItemPtr item)
        {
            if ((m_itemInProgress == item)||(item==nullptr))
            {
                return;
            }

            {
                LOCKGUARD(m_lock);
                auto it = std::find(m_timerQueue.begin(), m_timerQueue.end(), item);
                if (it != m_timerQueue.end()) {
                    // Still in the queue
                    m_timerQueue.erase(it);
                    delete item;
                    return;
                }
            }
#if 0
            for (;;) {
                {
                    LOCKGUARD(m_lock);
                    if (item->type == detail::WorkerThreadItem::Done) {
                        return;
                    }
                }
                Sleep(10);
            }
#endif
        }

    protected:
        static void threadFunc(void* lpThreadParameter)
        {
            WorkerThread* self = reinterpret_cast<WorkerThread*>(lpThreadParameter);
            LOG_INFO("Running thread %u", std::this_thread::get_id());

            detail::WorkerThreadItemPtr item = nullptr;
            for (;;) {
                unsigned nextTimerInMs = UINT_MAX;
                {
                    LOCKGUARD(self->m_lock);

                    int64_t now = getMonotonicTimeMs();
                    if (!self->m_timerQueue.empty() && self->m_timerQueue.front()->targetTime <= now) {
                        item = self->m_timerQueue.front();
                        self->m_timerQueue.pop_front();
                    }
                    if (!self->m_timerQueue.empty()) {
                        nextTimerInMs = static_cast<unsigned>(self->m_timerQueue.front()->targetTime - now);
                    }

                    if (!self->m_queue.empty() && !item) {
                        item = self->m_queue.front();
                        self->m_queue.pop_front();
                    }
                }

                if (!item) {
                    self->m_event.wait(nextTimerInMs);
                    Sleep(100);
                    continue;
                }

                if (item->type == detail::WorkerThreadItem::Shutdown) {
                    break;
                }
                
                LOG_TRACE("Execute item=%p type=%s\n", item, item->typeName.c_str() );
                self->m_itemInProgress = item;
                (*item)();
                self->m_itemInProgress = nullptr;

                if (item) {
                    item->type = detail::WorkerThreadItem::Done;
                    delete item;
                    item = nullptr;
                }
            }
        }
    };

    static WorkerThread *g_workerThread = nullptr;

    namespace detail {

        void queueWorkerThreadItem(detail::WorkerThreadItemPtr item)
        {
            g_workerThread->queue(item);
        }

        void cancelWorkerThreadItem(detail::WorkerThreadItemPtr item)
        {
            g_workerThread->cancel(item);
        }

    } // namespace detail

#pragma warning(push)
#pragma warning(disable:6031)
    std::string generateUuidString()
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
        std::srand((unsigned int)(std::time(0) ^ nanos));

        GUID_t uuid;
        uuid.Data1 = (uint16_t)std::rand();
        uuid.Data2 = (uint16_t)std::rand();
        uuid.Data3 = (uint16_t)std::rand();
        for (size_t i = 0; i < sizeof(uuid.Data4); i++)
            uuid.Data4[i] = (uint8_t)(std::rand());

        // TODO: [MG] - replace this sprintf by more robust GUID to string converter
        char buf[40] = { 0 };
        sprintf(buf,
            "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            uuid.Data1, uuid.Data2, uuid.Data3,
            uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3],
            uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]);
        return buf;
    }
#pragma warning(pop)

    int64_t getUtcSystemTimeMs()
    {
        return std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
    }

    int64_t getUtcSystemTime()
    {
        return getUtcSystemTimeMs() / 1000;
    }

    int64_t getUtcSystemTimeinTicks()
    {
#ifdef _WIN32
        FILETIME tocks;
        ::GetSystemTimeAsFileTime(&tocks);
        ULONGLONG ticks = (ULONGLONG(tocks.dwHighDateTime) << 32) | tocks.dwLowDateTime;
        // number of days from beginning to 1601 multiplied by ticks per day
        return ticks + 0x701ce1722770000ULL;
#else
        // FIXME: [MG] - add millis remainder to ticks
        std::time_t now = time(0);
        MAT::time_ticks_t ticks(&now);
        return ticks.ticks;
#endif
    }

    std::string formatUtcTimestampMsAsISO8601(int64_t timestampMs)
    {
#if 0
        __time64_t seconds = static_cast<__time64_t>(timestampMs / 1000);
        int milliseconds = static_cast<int>(timestampMs % 1000);

        tm tm;
        if (::_gmtime64_s(&tm, &seconds) != 0) {
            memset(&tm, 0, sizeof(tm));
        }

        char buf[sizeof("YYYY-MM-DDTHH:MM:SS.sssZ") + 1] = { 0 };
        ::_snprintf_s(buf, _TRUNCATE, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
            1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec, milliseconds);
#else
        time_t seconds = static_cast<time_t>(timestampMs / 1000);
        int milliseconds = static_cast<int>(timestampMs % 1000);

        tm tm;
#ifdef _MSC_VER
        bool valid = (gmtime_s(&tm, &seconds) == 0);
#else
        bool valid = (gmtime_r(&seconds, &tm) != NULL);
#endif
        if (!valid) {
            memset(&tm, 0, sizeof(tm));
        }

        char buf[sizeof("YYYY-MM-DDTHH:MM:SS.sssZ") + 1] = { 0 };
        int rc = snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
            1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec, milliseconds);
        (rc);
#endif
        return buf;
    }


    int64_t getMonotonicTimeMs()
    {

#if 1 /* ifdef _WIN32 */
        return std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
#else
        long            ms; // Milliseconds
        time_t          s;  // Seconds
        struct timespec spec;
        clock_gettime(CLOCK_MONOTONIC, &spec);
        s = spec.tv_sec;
        ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
        return ms;
#endif
    }

    static ISystemInformation* g_SystemInformation;
    static INetworkInformation* g_NetworkInformation;
    static IDeviceInformation*  g_DeviceInformation;

    void registerSemanticContext(ISemanticContext* context)
    {
        if (g_DeviceInformation != nullptr)
        {
            context->SetDeviceId(g_DeviceInformation->GetDeviceId());
            context->SetDeviceModel(g_DeviceInformation->GetModel());
            context->SetDeviceMake(g_DeviceInformation->GetManufacturer());
        }

        if (g_SystemInformation != nullptr)
        {
            // Get SystemInfo common fields
            context->SetOsVersion(g_SystemInformation->GetOsMajorVersion());
            context->SetOsName(g_SystemInformation->GetOsName());
            context->SetOsBuild(g_SystemInformation->GetOsFullVersion());
            context->SetDeviceClass(g_SystemInformation->GetDeviceClass());

            // AppInfo fields
            context->SetAppId(g_SystemInformation->GetAppId());
            context->SetAppVersion(g_SystemInformation->GetAppVersion());
            context->SetAppLanguage(g_SystemInformation->GetAppLanguage());

            // UserInfo fields.
            context->SetUserLanguage(g_SystemInformation->GetUserLanguage());
            context->SetUserTimeZone(g_SystemInformation->GetUserTimeZone());
            //context->SetUserAdvertisingId(g_SystemInformation->GetUserAdvertisingId());
        }
        if (g_NetworkInformation != nullptr)
        {
            // Get NetworkInfo common fields
            context->SetNetworkProvider(g_NetworkInformation->GetNetworkProvider());
            context->SetNetworkCost(g_NetworkInformation->GetNetworkCost());
            context->SetNetworkType(g_NetworkInformation->GetNetworkType());
        }
    }

    void unregisterSemanticContext(ISemanticContext* context)
    {
        UNREFERENCED_PARAMETER(context);
        OACR_USE_PTR(this);
    }

    //---
    // TODO: [MG] - make it portable...
    std::string getSdkVersion()
    {
        // TODO: [MG] - move this code to common PAL code
        return std::string(ARIASDK_VERSION_PREFIX "-Linux-C++-No-") + BUILD_VERSION_STR;
    }

    //---

    static volatile std::atomic<long> g_palStarted(0);

    void initialize()
    {
        if (g_palStarted.fetch_add(1) == 0) {
            LOG_TRACE("Initializing...");
            g_workerThread = new WorkerThread();
            g_SystemInformation = SystemInformationImpl::Create();
            g_DeviceInformation = DeviceInformationImpl::Create();
            g_NetworkInformation = NetworkInformationImpl::Create();
            LOG_INFO("Initialized");
        }
        else {
            LOG_ERROR("Already initialized: %d", g_palStarted.load());
        }
    }

    INetworkInformation* GetNetworkInformation() { return g_NetworkInformation; }
    IDeviceInformation* GetDeviceInformation() { return g_DeviceInformation; }

    void shutdown()
    {
        if (g_palStarted == 0)
        {
            LOG_ERROR("PAL is already shutdown!");
            return;
        }

        if (g_palStarted.fetch_sub(1) == 1) {
            LOG_TRACE("Shutting down...");
            delete g_workerThread;
            g_workerThread = nullptr;
            if (g_SystemInformation) { delete g_SystemInformation; g_SystemInformation = nullptr; }
            if (g_DeviceInformation) { delete g_DeviceInformation; g_DeviceInformation = nullptr; }
            if (g_NetworkInformation) { delete g_NetworkInformation; g_NetworkInformation = nullptr; }
            LOG_INFO("Shut down");
        }
        else {
            LOG_ERROR("Shutting down: %d", g_palStarted.load());
        }

        dumpRefCounted();
    }

} PAL_NS_END

#endif
