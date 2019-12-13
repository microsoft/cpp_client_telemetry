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

    PlatformAbstractionLayer& GetPAL() noexcept
    {
        static PlatformAbstractionLayer pal;
        return pal;
    }

    /**
     * Sleep for certain duration of milliseconds
     */
    void PlatformAbstractionLayer::sleep(unsigned delayMs)
    {
#ifdef _WIN32
        ::Sleep(delayMs);
#else
        std::this_thread::sleep_for(ms(delayMs));
#endif
    }

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
        std::recursive_mutex          debugLogMutex;
        std::string                   debugLogPath;
        std::unique_ptr<std::fstream> debugLogStream;
        
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

            debugLogStream = std::unique_ptr<std::fstream>(new std::fstream());
            debugLogStream->open(debugLogPath, std::fstream::out);
            if (!debugLogStream->is_open())
            {
                // If file cannot be created, log to /dev/null
                debugLogStream->open(DEBUG_LOG_NULL);
                result = false;
            }
            debugLogMutex.unlock();
            return result;
        }
        
        void log_done()
        {
            debugLogMutex.lock();
            if (debugLogStream)
            {
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

    std::shared_ptr<ITaskDispatcher> PlatformAbstractionLayer::getDefaultTaskDispatcher()
    {
        if (m_taskDispatcher == nullptr)
        {
            // Default implementation of task dispatcher is a single-threaded worker thread task queue
            LOG_TRACE("Initializing PAL worker thread");
            m_taskDispatcher.reset(PAL::WorkerThreadFactory::Create());
        }
        return m_taskDispatcher;
    }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:6031)
#endif
    std::string PlatformAbstractionLayer::generateUuidString()
    {
#ifdef _WIN32
        GUID uuid = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
        CoCreateGuid(&uuid);
        return MAT::to_string(uuid);
#else
        static std::once_flag flag;
        std::call_once(flag, [](){
            auto now = std::chrono::high_resolution_clock::now();
            auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
            std::srand(static_cast<unsigned int>(std::time(0) ^ nanos));
        });

        GUID_t uuid;
        uuid.Data1 = (static_cast<uint16_t>(std::rand()) << 16) | static_cast<uint16_t>(std::rand());
        uuid.Data2 = static_cast<uint16_t>(std::rand());
        uuid.Data3 = static_cast<uint16_t>(std::rand());
        for (size_t i = 0; i < sizeof(uuid.Data4); i++)
            uuid.Data4[i] = static_cast<uint8_t>(std::rand());

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

    int64_t PlatformAbstractionLayer::getUtcSystemTimeMs()
    {
#ifdef _WIN32
        ULARGE_INTEGER now;
        ::GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&now));
        return (now.QuadPart - 116444736000000000ull) / 10000;
#else
        return std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
#endif
    }

    int64_t PlatformAbstractionLayer::getUtcSystemTime()
    {
        return getUtcSystemTimeMs() / 1000;
    }

    int64_t PlatformAbstractionLayer::getUtcSystemTimeinTicks()
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

    std::string PlatformAbstractionLayer::formatUtcTimestampMsAsISO8601(int64_t timestampMs)
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
    uint64_t PlatformAbstractionLayer::getMonotonicTimeMs()
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
        return static_cast<uint64_t>(now.QuadPart / ticksPerMillisecond);
#else
        /* Cross-platform C++11 implementation */
        return std::chrono::steady_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
#endif
    }

    void PlatformAbstractionLayer::registerSemanticContext(ISemanticContext* context)
    {
        if (m_DeviceInformation != nullptr)
        {
            context->SetDeviceId(m_DeviceInformation->GetDeviceId());
            context->SetDeviceModel(m_DeviceInformation->GetModel());
            context->SetDeviceMake(m_DeviceInformation->GetManufacturer());
        }

        if (m_SystemInformation != nullptr)
        {
            // Get SystemInfo common fields
            context->SetOsVersion(m_SystemInformation->GetOsMajorVersion());
            context->SetOsName(m_SystemInformation->GetOsName());
            context->SetOsBuild(m_SystemInformation->GetOsFullVersion());
            context->SetDeviceClass(m_SystemInformation->GetDeviceClass());

            // AppInfo fields
            context->SetAppId(m_SystemInformation->GetAppId());
            context->SetAppVersion(m_SystemInformation->GetAppVersion());
            context->SetAppLanguage(m_SystemInformation->GetAppLanguage());

            // UserInfo fields.
            context->SetUserLanguage(m_SystemInformation->GetUserLanguage());
            context->SetUserTimeZone(m_SystemInformation->GetUserTimeZone());
            //context->SetUserAdvertisingId(m_SystemInformation->GetUserAdvertisingId());

            context->SetCommercialId(m_SystemInformation->GetCommercialId());
        }
        if (m_NetworkInformation != nullptr)
        {
            // Get NetworkInfo common fields
            context->SetNetworkProvider(m_NetworkInformation->GetNetworkProvider());
            context->SetNetworkCost(m_NetworkInformation->GetNetworkCost());
            context->SetNetworkType(m_NetworkInformation->GetNetworkType());
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

    const std::string& PlatformAbstractionLayer::getSdkVersion()
    {
        static const std::string sdkVersion(EVTSDK_VERSION_PREFIX "-" OS_NAME "-C++-" ECS_SUPP "-" BUILD_VERSION_STR);
        return sdkVersion;
    }

    static volatile std::atomic<long> g_palStarted(0);

    void PlatformAbstractionLayer::initialize(IRuntimeConfig& configuration)
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
            m_SystemInformation = SystemInformationImpl::Create();
            m_DeviceInformation = DeviceInformationImpl::Create();
            m_NetworkInformation = NetworkInformationImpl::Create(configuration[CFG_BOOL_ENABLE_NET_DETECT]);
            LOG_INFO("Initialized");
        }
        else
        {
            LOG_ERROR("Already initialized: %d", g_palStarted.load());
        }
    }

    INetworkInformation* PlatformAbstractionLayer::GetNetworkInformation() { return m_NetworkInformation; }
    IDeviceInformation* PlatformAbstractionLayer::GetDeviceInformation() { return m_DeviceInformation; }
    ISystemInformation* PlatformAbstractionLayer::GetSystemInformation() { return m_SystemInformation; }

    void PlatformAbstractionLayer::shutdown()
    {
        if (g_palStarted == 0)
        {
            LOG_ERROR("PAL is already shutdown!");
            return;
        }

        if (g_palStarted.fetch_sub(1) == 1)
        {
            LOG_TRACE("Shutting down...");
            if (m_taskDispatcher) { m_taskDispatcher = nullptr; }
            if (m_SystemInformation) { delete m_SystemInformation; m_SystemInformation = nullptr; }
            if (m_DeviceInformation) { delete m_DeviceInformation; m_DeviceInformation = nullptr; }
            if (m_NetworkInformation) { delete m_NetworkInformation; m_NetworkInformation = nullptr; }
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
    bool PlatformAbstractionLayer::IsUtcRegistrationEnabledinWindows()
    {
        return false;
    }

    bool PlatformAbstractionLayer::RegisterIkeyWithWindowsTelemetry(std::string const&, int, int)
    {
        return false;
    }
#endif

} PAL_NS_END
