//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef DEBUGLOGGER_HPP
#define DEBUGLOGGER_HPP
/// <summary>
/// C++11 implementation of a cross-platform debug logging facility.
/// </summary>

/// Macros below specify the default namespace of the Debug Logger.
/// Namespace of the class can be overridden at compile time using
/// 'NS_DBG' and 'NS_DBG_BEGIN' macros as follows:
/// 
/// #define NS_DBG          My::Own::Namespace
/// #define NS_DBG_BEGIN    namespace My { namespace Own { namespace Namespace
/// 
/// If only NS_DBG macro is specified, then compiler would attempt
/// to use nested namespace definition. Nested namespace definition
/// feature is available in:
/// - GCC since version 6 enable using - std = c++1z
/// - Visual C++ since 2015 update 3 enable using / std:c++latest
/// - Clang since version 3.6 enable using - std = c++1z
#ifndef NS_DBG
#define NS_DBG        Microsoft::Applications::Telemetry
#define NS_DBG_BEGIN  namespace Microsoft { namespace Applications { namespace Telemetry
#define NS_DBG_END    } }
#else
#ifndef NS_DBG_BEGIN
#define NS_DBG_BEGIN  namespace NS_DBG
#define NS_DBG_END
#else
#define NS_DBG_END  } }
#endif
#endif

#ifndef NDEBUG
// Debug build
#define DBG_LOG_LEVEL_DEFAULT     DebugLevel::Debug
#else
// Release build
#define DBG_LOG_LEVEL_DEFAULT     DebugLevel::Fatal
#endif

#define DBG_LOG_BUFFER_LEN        1024
#define DBG_LOG_NULL              "NUL"
#define DBG_LOG_MAX_SIZE          16000000 // ~16 MB

#include <string>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <type_traits>

#ifndef _MANAGED
#include <thread>
#endif

#include <algorithm>
#include <locale>
#include <codecvt>
#include <map>
#include <unordered_map>

#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

#ifdef _WIN32
#include <Windows.h>
#include <Evntprov.h>
#include <Rpc.h>
#pragma comment (lib, "Rpcrt4.lib")
#endif

#ifdef HAVE_SYSLOG
#include <syslog.h>
#include <stdarg.h>
#endif

/// <summary>
/// Each module gets an integer tag generated from module name at compile time.
/// DJB2 hash function below is near-perfect hash used by several systems.
/// Ref. http://www.cse.yorku.ca/~oz/hash.html
/// </summary>
/// <param name="str">String to hash</param>
/// <param name="h">Initial offset</param>
/// <returns></returns>
constexpr uint32_t mul(const uint32_t a, const uint32_t b)
{
    return (uint32_t)((uint64_t)a*(uint64_t)b);
}

constexpr uint32_t djb2_constexpr_hash(const char* const str, const int h = 0)
{
    return (uint32_t)(!str[h] ? 5381 : mul(djb2_constexpr_hash(str, h + 1), 33) ^ str[h]);
}

static uint32_t djb2_hash(const char* str, const int h = 0)
{
    return (uint32_t)(!str[h] ? 5381 : ((uint32_t)djb2_hash(str, h + 1) * (uint32_t)33) ^ str[h]);
}

// Unconventional, but ultimate way of enforcing constexpr at compile-time.
#define CONST_UINT32_T(x)           std::integral_constant<uint32_t, (uint32_t)x>::value

// Hash function below is computing module tag from literal at compile-time using C++11 constexpr
#define DBG_MODULE_CTAG(name)       CONST_UINT32_T(djb2_constexpr_hash(#name))
#define DBG_MODULE_TAG(name)        djb2_hash(name)
#define DBG_LOG_NAMETAG(name)       { DBG_MODULE_CTAG(name) , #name } ,

/***********************************************************************************************************/

#include "DebugProviders.hpp"

#ifndef _MANAGED
#include <mutex>
#define USE_RECURSIVE_MUTEX
#endif

/// <summary>DebugLogger namespace</summary>
NS_DBG_BEGIN {

#ifdef USE_RECURSIVE_MUTEX
    /* Use recursive mutex for native C++ build */
    extern std::recursive_mutex debugLogMutex;

#define DBG_LOCK        std::lock_guard<std::recursive_mutex> lock(debugLogMutex)
#else
    /* Do not provide thread-safety for methods invoked from /clr compiled modules */
#define DBG_LOCK
#endif

    /// <summary>
    /// SDK module tag constant
    /// </summary>
    typedef uint32_t Tag;

    /// <summary>
    /// SDK trace level constant. Internal definition for use within Client Telemetry SDK.
    /// MAT::ACTTraceLevel declares its own type with one-to-one mapping to MAT::DebugLevel.
    /// The intent is to shield DebugLogger.hpp from any external SDK definitions, keeping
    /// it as a standalone product not bound to particular SDK header definitions.
    /// </summary>
    typedef enum {
        Debug,    /// <summary>Debug messages</summary>
        Trace,    /// <summary>Trace messages</summary>
        Info,     /// <summary>Informational messages</summary>
        Warn,     /// <summary>Warnings</summary>
        Error,    /// <summary>Errors</summary>
        Fatal     /// <summary>Fatal errors that lead to process termination</summary>
    } DebugLevel;

    /// <summary>
    /// SDK module descriptor:
    /// * tag
    /// * name
    /// </summary>
    typedef struct ModuleDescriptor {
        Tag         tag;
        const char *name;
    } ModuleDescriptor;

    /// <summary>
    /// DebugContext (scope) - describes the location that emitted the log (optional)
    /// </summary>
    typedef struct {
        Tag         tag;        // Module tag
        const char *file;       // Filename
        int         line;       // Line #
        const char *func;       // Function / method
        const char *type;       // Class type
        void       *extras;     // Optional exta, e.g. callstack / backtrace ptr
        DebugLevel  level;
    } DebugContext;

/// <summary>
/// This datastructure allows to place each module tag (djb2 hash) and name to array
/// of structures. SDK enumerates the list of modules on start by name, then assigns
/// some log verbosity to each component. Debug mask can be used to assign debug log
/// verbosity of several modules at once.
/// </summary>
static const ModuleDescriptor dbg_modules[] =
{
#include "module_list"
};

#define DBG_LOG_MODULE_MAX          (sizeof(dbg_modules) / sizeof(dbg_modules[0]))

static constexpr Tag MODULE_ALL = DBG_MODULE_CTAG(*);

/// <summary>
/// Debug events logger
/// </summary>
class DebugLogger
{

public:

    // Shortcut to global verbosity level for fast execution of Trace wrapper macros
    DebugLevel      level;

    /// <summary>
    /// Bitmask definitions for log format
    /// </summary>
    enum LogFormat
    {
        FMT_NONE = 0,               // No decoration
        FMT_TIME = 1,               // Prepend time
        FMT_CTX  = 2,               // Prepend context [tag|file:line|function|class|stack]
        FMT_PID  = 4,               // Prepend $PID
        FMT_TID  = 8,               // Prepend $TID
        FMT_ENDL = 16,              // Append \n
    };

    /// <summary>
    /// Trace provider
    /// </summary>
    enum TraceProvider {
        TP_NULL,                    // /dev/null or NUL
        TP_File,                    // file handle
        TP_OutputDebugString,       // OutputDebugString
        TP_ETW,                     // ETW API [Windows only]
        TP_SYSLOG,                  // syslog [*nix only]
        TP_CONSOLE,                 // stdout / stderr
        TP_USER,                    // custom user implementation
        TP_MAX
    };

protected:

    /// <summary>
    /// [optional] Trace provider-specific logging destination path:
    /// * For TP_File: complete filename of destination log file
    /// * For TP_ETW:  provider name or GUID (if starts with '{')
    /// </summary>
    std::string     path;

    /// <summary>
    /// [optional] Custom log provider sink. This parameter is only used by TP_USER
    /// </summary>
    std::ostream    *logsink;

    /// <summary>
    /// Maximum log file size for TP_File
    /// </summary>
    size_t          maxFileSize;

    /// <summary>
    /// Log entry format specifier
    /// </summary>
    unsigned        logFormat;

    /// <summary>
    /// Map of Tag to DebugLevel - controls debug verbosity per module
    /// </summary>
    std::map<Tag, DebugLevel>   tagLevels;

    /// <summary>
    /// Map of Tag to Name - integer tag hash to name mapping
    /// </summary>
    std::unordered_map<Tag, std::string> tagNames;

    /// <summary>
    /// Trace provider
    /// </summary>
    TraceProvider   traceProvider;

public:

    /// <summary>
    /// Retrieve the tag value of a module.
    /// This function is invoked on start for modules listed in "module_list" header.
    /// Custom unlisted modules may invoke this from their module init routine.
    /// </summary>
    /// <param name="module"></param>
    /// <returns></returns>
    Tag RegisterTag(const char *module)
    {
        DBG_LOCK;
        Tag tag = (Tag)DBG_MODULE_TAG(module);
        tagNames[tag] = module;
        return tag;
    }

    /// <summary>
    /// Specify user-supplied log sink, e.g. output file stream, pipe or UDP channel.
    /// </summary>
    /// <param name="newLogsink"></param>
    void SetLogSink(std::ostream *newLogsink)
    {
        DBG_LOCK;
        SetTraceProvider(TP_USER, newLogsink);
    }

    /// <summary>
    /// Set log format specifier.
    /// </summary>
    /// <param name="format"></param>
    void SetLogFormat(unsigned format)
    {
        DBG_LOCK;
        logFormat = format;
    }

    /// <summary>
    /// Set the maximum file size.
    /// This parameter used only by TP_FILE trace provider.
    /// </summary>
    /// <param name="maxFileSize"></param>
    void SetMaxFileSize(size_t maxFileSize)
    {
        DBG_LOCK;
        this->maxFileSize = maxFileSize;
    }

    /// <summary>
    /// Get current log format
    /// </summary>
    /// <returns></returns>
    unsigned GetLogFormat()
    {
        DBG_LOCK;
        return logFormat;
    }

    /// <summary>
    /// Set the debug log sink path / destination:
    /// * Filename for file provider
    /// * Provider GUID for ETW
    /// * syslog log name for syslog
    /// etc.
    /// </summary>
    /// <param name="newPath"></param>
    void SetPath(const char* newPath)
    {
        DBG_LOCK;
        this->path = newPath;
    }

    /// <summary>
    /// Get the current debug log sink path.
    /// </summary>
    /// <returns></returns>
    const char* GetPath()
    {
        DBG_LOCK;
        return path.c_str();
    }

    /// <summary>
    /// Set trace provider and open corresponding output stream.
    /// Note that this method is not thread-safe.
    /// </summary>
    /// <param name="provider"></param>
    /// <param name="newLogsink">Optional: custom log sink for TP_USER</param>
    void SetTraceProvider(TraceProvider provider, std::ostream *userLogsink = nullptr)
    {
        DBG_LOCK;

        if ((logsink != nullptr)&&(traceProvider!=TP_CONSOLE)&&(traceProvider!=TP_USER))
            // Never delete a ptr to std::cout or to user-provided logsink
            delete logsink;

        logsink = nullptr;

        traceProvider = provider;

        // Update logsink depending on provider type
        switch (traceProvider)
        {

        case TP_NULL:
            break;

        case TP_File:
        {
            auto filestream = new LogFileStream();
            filestream->reopen(path.c_str()); // TODO: UTF-8 safety. Worst-case scenario: log is not opened.
            logsink = filestream;
            break;
        }

#ifdef _WIN32
        case TP_OutputDebugString:
            logsink = new OutputDebugStringStream();
            break;

        case TP_ETW:
            logsink = new ETWStringStream(path.c_str());
            break;
#endif

#ifdef HAVE_SYSLOG
        case TP_SYSLOG:
            // syslog [*nix only]
            break;
#endif

        case TP_CONSOLE:
            logsink = &(std::cout);
            break;

        case TP_USER:
            logsink = userLogsink;
            break;

        default:
            break;
        }
    }

    /// <summary>
    /// Create a debug logger.
    /// * Default provider is NULL (no logging)
    /// * Default path is empty ("")
    /// * Default max log file size is DBG_LOG_MAX_SIZE
    /// * Default custom log sink is nullptr (no custom log sink)
    /// * Default log format includes time, pid, tid and EOL (FMT_TIME | FMT_CTX | FMT_PID | FMT_TID | FMT_ENDL),
    /// </summary>
    DebugLogger(
        TraceProvider traceProvider = TraceProvider::TP_NULL,
        const char* path            = "",
        size_t maxFileSize          = DBG_LOG_MAX_SIZE,
        unsigned logFormat          = (FMT_TIME | FMT_CTX | FMT_PID | FMT_TID | FMT_ENDL),
        std::ostream *logsink       = nullptr
    )
    {

        // First we set the trace provider common settings
        SetLogFormat(logFormat);
        SetPath(path);
        SetMaxFileSize(maxFileSize);

        // don't call SetLogSink because it'd also adjust the provider to TP_USER
        this->logsink = logsink;

        // Then we apply the trace provider type
        SetTraceProvider(traceProvider);

        // level is set to "Debug" on Debug builds and "Fatal" on Release builds.
        level = DBG_LOG_LEVEL_DEFAULT;
        // Populate all tags with default log level
        for (size_t i = 0; i < DBG_LOG_MODULE_MAX; i++)
        {
            // TODO: allow to pass down a custom value for DBG_LOG_LEVEL_DEFAULT
            Tag tag = dbg_modules[i].tag;
            // Register all predefined hardcoded tag names
            RegisterTag(dbg_modules[i].name);
            SetTraceLevel(DBG_LOG_LEVEL_DEFAULT, tag);
        }
    }

    /// <summary>
    /// Apply trace level for several modules by mask. If a module is not enabled
    /// by the mask, then its debug verbosity level is downgraded to minimum verbosity.
    /// </summary>
    /// <param name="level"></param>
    /// <param name="mask"></param>
    void SetTraceLevelMask(DebugLevel level, unsigned int mask)
    {
        for (size_t i = 0; i < DBG_LOG_MODULE_MAX; i++)
        {
            Tag tag = dbg_modules[i].tag;
            // Don't let to alter MODULE_ALL(*) global verbosity via this API
            if (tag != MODULE_ALL)
                SetTraceLevel((mask & 1) ?
                    level :
                    DebugLevel::Fatal,
                    tag
                );
            mask >>= 1;
        }
    }

    /// <summary>
    /// Apply trace level for a module tag
    /// </summary>
    /// <param name="level"></param>
    /// <param name="tag"></param>
    inline void SetTraceLevel(DebugLevel level, Tag tag = MODULE_ALL)
    {
        if (tag == MODULE_ALL)
            this->level = level;
        tagLevels[tag] = level;
    }

    /// <summary>
    /// Get trace level for a given tag
    /// </summary>
    /// <param name="tag"></param>
    /// <returns></returns>
    inline DebugLevel GetTraceLevel(Tag tag = MODULE_ALL)
    {
        if (tag == MODULE_ALL)
            return this->level;

        return (DebugLevel)
            std::min(
            (unsigned)(tagLevels[MODULE_ALL]),
            (unsigned)(tagLevels[tag])
        );
    }

    /// <summary>
    /// Check if tag logging is enabled at specified level
    /// </summary>
    /// <param name="tag"></param>
    /// <param name="level"></param>
    /// <returns></returns>
    inline virtual bool isTagEnabled(Tag tag, DebugLevel level)
    {
        return (level>=this->level)
            && (level>=tagLevels[tag]);
    }

    /// <summary>
    /// Performs periodic truncation of logs. This rotation is done not more often than
    /// once in 1024 log lines. If each log entry is 1024 bytes, then we consume up to
    /// 1 MB, but not more than that. The reason to perform only periodic truncation is
    /// x10 times performance difference.
    /// </summary>
    inline void rotateLogs()
    {
        static unsigned logs = 0;
        if (logs++ & 1024) // bitwise and, not logical!
        {
            logs = 0;
            LogFileStream *ofs = (LogFileStream*)logsink;
            auto pos = ofs->tellp();
            // If ofs returns -1, that means we didn't open or something happened
            // to file descriptor, so we'd attempt to truncate-reopen the file
            if (((int)pos == -1) || ((size_t)pos > maxFileSize))
            {
                // Seek back to fpos=0 on original file descriptor
                ofs->seekp(0);
            }
        }
    }

    /// <summary>
    /// Send ASCII or UTF-8 formatted buffer to debug logger.
    /// Custom implementation can be provided for logsink or
    /// by overriding the log method implementation.
    /// </summary>
    /// <param name="buff">ASCII or UTF-8 buffer</param>
    /// <returns></returns>
    virtual int log(const char* buff, DebugContext *ctx = nullptr)
    {
        if (logsink != nullptr)
        {
            if (traceProvider == TP_File)
            {
#ifndef _MSC_VER
                // ofstream<< operator is atomic on MS Visual C++, however, there is no general
                // guarantee that it is atomic on other platforms. TODO: review this lock on
                Linux to determine if we require it or not.
                DBG_LOCK;
#endif
                (*logsink) << buff;
                rotateLogs();
            }
            else
            {
                // Other providers use kernel API / syscalls and wouldn't need a lock.
                // The locking is provided by the underlying logsink implementation in
                // some shape or form.
                (*logsink) << buff;
            }
        }
        int result = 0;
        return result;
    }
    
    /// <summary>
    /// Convert DebugLevel enum to prefix letter
    /// </summary>
    /// <param name="level"></param>
    /// <returns>Either of ['D','T','I','W','E','F']</returns>
    static inline char TraceLevelName(DebugLevel level)
    {
        static char names[] = { 'D','T','I','W','E','F' };
        return names[level];
    }

    /// <summary>
    /// Append optional format: Time, PID, TID
    /// </summary>
    /// <param name="buff"></param>
    virtual int logFmt(const char* buff, DebugContext *ctx = nullptr)
    {
        int result = 0;
        if (logFormat != FMT_NONE)
        {
            std::stringstream ss;

            // Append formatted timestamp
            if (logFormat & FMT_TIME)
            {
                auto now = std::chrono::system_clock::now();
                int64_t millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
                auto in_time_t = std::chrono::system_clock::to_time_t(now);
#pragma warning(push)
#pragma warning(disable: 4996)
                // FIXME: this has been the case for all v1 builds and v2 Linux, but we should improve this because
                // the static structure returned by localtime function may change its contents if another thread
                // invokes localtime. Generally that is not an issue, as time won't change that fast, but it may
                // potentially result an invalid log event timestamp.
                //
                // warning C4996: 'localtime': This function or variable may be unsafe. Consider using localtime_s instead
                ss << std::put_time(localtime(&in_time_t), "%Y-%m-%d %X");
#pragma warning(pop)
                ss << "." << std::setfill('0') << std::setw(3) << (unsigned)(millis % 1000);
                ss << "|";
            }

            // Append context information, e.g.
            // T|MAIN    |        main.cpp:123  |test_etw        |Hola ETW! i=40
            //
            if (logFormat & FMT_CTX)
            {
                ss << TraceLevelName((ctx != nullptr) ? ctx->level : DebugLevel::Debug);
                ss << '|' << std::setfill(' ') << std::setw(8)  << std::left  << ((ctx!=nullptr)?tagNames[ctx->tag].c_str():"");
                ss << '|' << std::setfill(' ') << std::setw(16) << std::right << ((ctx!=nullptr)?ctx->file:"");
                ss << ':' << std::setfill(' ') << std::setw(5)  << std::left  << ((ctx!=nullptr)?ctx->line:0);
                ss << '|' << std::setfill(' ') << std::setw(16) << std::left  << ((ctx!=nullptr)?ctx->func : "");
                ss << '|';
            }

            // Append TID
            if (logFormat & FMT_TID)
            {
                ss << std::setfill('0') << std::setw(8);
#ifndef _MANAGED
                /* capture TID only for native compiled modules. <thread> can't be used with /CLI modules */
                ss << std::this_thread::get_id();
#else
                ss << 0;
#endif
                ss << "|";
            }
            ss << buff;

            // Append end-of-line \n
            if (logFormat & FMT_ENDL)
                ss << "\n";

            if (traceProvider == TP_ETW)
            {
                // Append UTF-16 NUL to the end
                ss.seekg(0, std::ios::end);
                auto size = ss.tellg();
                if (size % 2)
                    ss << " ";
                ss << (char)0 << (char)0 << (char)0;
            }

            result = log(ss.str().c_str(), ctx);
        }
        else
        {
            result = log(buff, ctx);
        }
        return result;
    }

    /// <summary>
    /// Trace formatted string with variable arguments with optional context
    /// </summary>
    /// <param name="format"></param>
    /// <param name="args"></param>
    /// <returns></returns>
    virtual int operator()(const char* format, va_list args, DebugContext *ctx = nullptr)
    {
        // TODO: allocation of this buffer on stack each time might be overkill.
        // One potential alternate solution to this is to maintain a thread-safe
        // static buffer instead.
        char buffer[DBG_LOG_BUFFER_LEN] = { 0 };
        int result = vsnprintf(buffer, sizeof(buffer), format, args);
        return logFmt(buffer, ctx);
    }

    /// <summary>
    /// Trace formatted string buffer with optional context
    /// </summary>
    /// <param name="format"></param>
    /// <param name="args"></param>
    /// <returns></returns>
    virtual int operator()(const char* buffer, DebugContext *ctx = nullptr)
    {
        return logFmt(buffer, ctx);
    }

    /// <summary>
    /// Emits a debug log message without any extra frills
    /// </summary>
    /// <param name="format"></param>
    /// <param name=""></param>
    /// <returns></returns>
    virtual int operator()(const char *format, ...)
    {
        int result;
        va_list ap;
        va_start(ap, format);
        result = (*this)(format, ap); // Simple call with NULL DebugContext *ctx
        va_end(ap);
        return result;
    }

    /// <summary>
    /// Emits a debug log message for specific tag and level
    /// </summary>
    /// <param name="tag"></param>
    /// <param name="level"></param>
    /// <param name="format"></param>
    /// <param name=""></param>
    /// <returns></returns>
    virtual int operator()(DebugContext ctx, DebugLevel level, const char *format, ...)
    {
        if (isTagEnabled(ctx.tag, level))
        {
            ctx.level = level;
            int result;
            va_list ap;
            va_start(ap, format);
            result = (*this)(format, ap, &ctx);
            va_end(ap);
            return result;
        }
        return 0;
    }

    virtual ~DebugLogger()
    {
        // Clean-up the logsink object
        if ((logsink != nullptr) && (traceProvider != TP_CONSOLE) && (traceProvider != TP_USER))
        {
            logsink->flush();
            // Never delete a ptr to std::cout or to user-provided logsink
            delete logsink;
        }

        if (traceProvider == TP_File)
        {
#ifdef NDEBUG
            /* Release bits always clean-up the log on process on shutdown. */
            if (!path.empty())
            {
                std::remove(path.c_str()); // delete file
            }
#endif
        }
    }
};

} NS_DBG_END

#ifndef DEBUG_LOG_MACROS
#define DEBUG_LOG_MACROS
#ifndef PATH_SEPARATOR
#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#ifndef __FUNCSIG__
#define __FUNCSIG__ __FUNCTION__
#endif
#endif
#endif
#endif

constexpr int basename_index_const(const char * const path, const int index = 0, const int slash_index = -1)
{
    return path[index]
        ? (path[index] == PATH_SEPARATOR
            ? basename_index_const(path, index + 1, index)
            : basename_index_const(path, index + 1, slash_index)
            )
        : (slash_index + 1)
        ;
}

constexpr const char* const BASENAME(const char * const path)
{
    return path + basename_index_const(path);
}

#define __FILENAME__ (const char * const)(__FILE__+CONST_UINT32_T(basename_index_const(__FILE__)))

#endif

