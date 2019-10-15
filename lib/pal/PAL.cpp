// Copyright (c) Microsoft Corporation. All rights reserved.
#include "PAL.hpp"

#include "ILogManager.hpp"
#include "ISemanticContext.hpp"
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
#pragma comment(lib, "Ole32.Lib")   /* CoCreateGuid */
#include <oacr.h>
#endif

#include <ctime>

namespace PAL_NS_BEGIN {

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
            MAT::replace(key, "Microsoft::Applications::Telemetry::", "MAT::");
            MAT::replace(key, "Microsoft::Applications::Telemetry::PAL::", "PAL::");
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

    MATSDK_LOG_INST_COMPONENT_NS("MATSDK.PAL", "MSTel client - platform abstraction layer");

    namespace detail {

#ifdef NDEBUG
        LogLevel g_logLevel = LogLevel::Error;
#else
        LogLevel g_logLevel = LogLevel::Detail;
#endif

#define DBG_BUFFER_LEN      2048

#ifndef _WIN32
#define DEBUG_LOG_NULL      "/dev/null"
#else
#define DEBUG_LOG_NULL      "NUL"
#endif

        bool isLoggingInited = false;
        
#ifdef HAVE_MAT_LOGGING
        std::recursive_mutex debugLogMutex;
        std::string          debugLogPath;
        std::ostream*        debugLogStream = nullptr;
        
        bool log_init(bool isTraceEnabled, const std::string& traceFolderPath)
        {
            if (!isTraceEnabled)
            {
                return false;
            }

            bool result = true;
            if (debugLogStream != nullptr)
            {
                return result;
            }

            debugLogMutex.lock();
            debugLogPath = traceFolderPath;
            debugLogPath += "mat-debug-";
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
            debugLogMutex.unlock();
            return result;
        }
        
        void log_done()
        {
            debugLogMutex.lock();
            if (debugLogStream)
            {
                delete debugLogStream;
                debugLogStream = nullptr;
                isLoggingInited = false;
            }
            debugLogMutex.unlock();
        }
#else
        bool log_init(bool /*isTraceEnabled*/, const std::string& /*traceFolderPath*/)
        {
            return false;
        }

        void log_done()
        {
        }
#endif

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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif
        void log(LogLevel level, char const* component, char const* fmt, ...)
        {
#ifdef HAVE_MAT_LOGGING
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
#else       /* Avoid unused parameter warning */
            (void)(level);
            (void)(component);
            (void)(fmt);
#endif /* of #ifdef HAVE_MAT_LOGGING */
        }
#ifdef _MSC_VER
#pragma warning(pop)
#endif

    } // namespace detail

    static std::shared_ptr<ITaskDispatcher> g_taskDispatcher = nullptr;

    std::shared_ptr<ITaskDispatcher> getDefaultTaskDispatcher()
    {
        if (g_taskDispatcher == nullptr)
        {
            // Default implementation of task dispatcher is a single-threaded worker thread task queue
            LOG_TRACE("Initializing PAL worker thread");
            g_taskDispatcher.reset(PAL::WorkerThreadFactory::Create());
        }
        return g_taskDispatcher;
    }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:6031)
#endif
    std::string generateUuidString()
    {
#ifdef _WIN32
        GUID uuid = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
        CoCreateGuid(&uuid);
        return MAT::to_string(uuid);
#else
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
#endif
    }
#ifdef _MSC_VER
#pragma warning(pop)
#endif

    int64_t getUtcSystemTimeMs()
    {
#ifdef _WIN32
        ULARGE_INTEGER now;
        ::GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&now));
        return (now.QuadPart - 116444736000000000ull) / 10000;
#else
        return std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
#endif
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
#ifdef _WIN32
        __time64_t seconds = static_cast<__time64_t>(timestampMs / 1000);
        int milliseconds = static_cast<int>(timestampMs % 1000);

        tm tm;
        if (::_gmtime64_s(&tm, &seconds) != 0)
        {
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
        bool valid = (gmtime_r(&seconds, &tm) != NULL);

        if (!valid)
        {
            memset(&tm, 0, sizeof(tm));
        }

        char buf[sizeof("YYYY-MM-DDTHH:MM:SS.sssZ") + 1] = { 0 };
        (void) snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
            1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec, milliseconds);
#endif
        return buf;
    }

    /**
     * Get monotonic time in milliseconds.
     *
     * PAL function used by the following comonents:
     * - HttpClientManager request perf counter
     * - PAL WorkItem scheduler
     * - SQLiteWrapper perf counter
     */
    int64_t getMonotonicTimeMs()
    {
#ifdef USE_WIN32_PERFCOUNTER
        /* Win32 API implementation */
        static bool frequencyQueried = false;
        static int64_t ticksPerMillisecond;
        if (!frequencyQueried)
        {
            // There is no harm in querying twice in case of a race condition.
            LARGE_INTEGER ticksInOneSecond;
            ::QueryPerformanceFrequency(&ticksInOneSecond);
            ticksPerMillisecond = ticksInOneSecond.QuadPart / 1000;
            frequencyQueried = true;
        }

        LARGE_INTEGER now;
        ::QueryPerformanceCounter(&now);
        return now.QuadPart / ticksPerMillisecond;
#else
        /* Cross-platform C++11 implementation */
        return std::chrono::steady_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
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

            context->SetCommercialId(g_SystemInformation->GetCommercialId());
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
    }

#undef OS_NAME
#if defined(_WIN32)
    #define OS_NAME     "Windows"
#elif defined(__APPLE__)
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
        #define OS_NAME    "iOSSim"
    #elif TARGET_OS_IPHONE
        #define OS_NAME    "iOS"
    #elif TARGET_OS_MAC
        #define OS_NAME    "MacOSX"
    #else
        #define OS_NAME    "UnknownApple"
    #endif
#elif defined(__linux__) || defined(LINUX) || defined(linux)
    #define OS_NAME     "Linux"
#else
    #define OS_NAME     "Unknown"
#endif

    static const std::string sdkVersion(EVTSDK_VERSION_PREFIX "-" OS_NAME "-C++-" ECS_SUPP "-" BUILD_VERSION_STR);

    const std::string& getSdkVersion()
    {
        return sdkVersion;
    }

    static volatile std::atomic<long> g_palStarted(0);

    void initialize(IRuntimeConfig& configuration)
    {
        if (g_palStarted.fetch_add(1) == 0)
        {
            std::string traceFolderPath = MAT::GetTempDirectory();
            if (configuration.HasConfig(CFG_STR_TRACE_FOLDER_PATH))
            {
                traceFolderPath = static_cast<std::string&>(configuration[CFG_STR_TRACE_FOLDER_PATH]);
            }

            detail::isLoggingInited = detail::log_init(configuration[CFG_BOOL_ENABLE_TRACE], traceFolderPath);
            LOG_TRACE("Initializing...");
            g_SystemInformation = SystemInformationImpl::Create();
            g_DeviceInformation = DeviceInformationImpl::Create();
            g_NetworkInformation = NetworkInformationImpl::Create(configuration[CFG_BOOL_ENABLE_NET_DETECT]);
            LOG_INFO("Initialized");
        }
        else
        {
            LOG_ERROR("Already initialized: %d", g_palStarted.load());
        }
    }

    INetworkInformation* GetNetworkInformation() { return g_NetworkInformation; }
    IDeviceInformation* GetDeviceInformation() { return g_DeviceInformation; }
    ISystemInformation* GetSystemInformation() { return g_SystemInformation; }

    void shutdown()
    {
        if (g_palStarted == 0)
        {
            LOG_ERROR("PAL is already shutdown!");
            return;
        }

        if (g_palStarted.fetch_sub(1) == 1)
        {
            LOG_TRACE("Shutting down...");
            if (g_taskDispatcher) { g_taskDispatcher = nullptr; }
            if (g_SystemInformation) { delete g_SystemInformation; g_SystemInformation = nullptr; }
            if (g_DeviceInformation) { delete g_DeviceInformation; g_DeviceInformation = nullptr; }
            if (g_NetworkInformation) { delete g_NetworkInformation; g_NetworkInformation = nullptr; }
            LOG_INFO("Shut down");
            detail::log_done();
        }
        else
        {
            LOG_ERROR("Shutting down: %d", g_palStarted.load());
        }

        dumpRefCounted();
    }

#ifndef HAVE_MAT_UTC
    bool IsUtcRegistrationEnabledinWindows()
    {
        return false;
    }

    bool RegisterIkeyWithWindowsTelemetry(std::string const&, int, int)
    {
        return false;
    }
#endif

} PAL_NS_END
