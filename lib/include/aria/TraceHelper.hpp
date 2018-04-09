#pragma once

#include "fwd-ct.hpp"

#include "ctsdk/ctlog.hpp"
#include <stddef.h>
#include <stdexcept>

#include <fnoexception.h>

#ifndef LOG_MODULE
#define LOG_MODULE DBG_GLOBAL
#endif

// This macro is kept here for backwards-compatiblity
#define TRACEMSG(x, y, ...)     do { PAL::Trace(x, LOG_MODULE, y, __VA_ARGS__); }   while (0);

extern const char * PAL::dbg_module_names[DBG_MAX];

// Please use these LOG_* or TRACE macro
#define LOG_TRACE(fmt, ...)     do { PAL::Trace(TL_TRACE, LOG_MODULE, "[%s] %s(%d):\t" fmt, ::PAL::dbg_module_names[LOG_MODULE], __FILENAME__, __LINE__ , ##__VA_ARGS__); } while (0);
#define LOG_DEBUG(fmt, ...)     do { PAL::Trace(TL_DEBUG, LOG_MODULE, "[%s] %s(%d):\t" fmt, ::PAL::dbg_module_names[LOG_MODULE], __FILENAME__, __LINE__ , ##__VA_ARGS__); } while (0);
#define LOG_INFO(fmt,  ...)     do { PAL::Trace(TL_INFO,  LOG_MODULE, "[%s] %s(%d):\t" fmt, ::PAL::dbg_module_names[LOG_MODULE], __FILENAME__, __LINE__ , ##__VA_ARGS__); } while (0);

#define LOG_WARN(fmt,  ...)     do { PAL::Trace(TL_WARN,  LOG_MODULE, "%s(%d): %s WARNING: " fmt, __FILE__, __LINE__, __FUNCSIG__ , ##__VA_ARGS__); } while (0);
#define LOG_ERROR(fmt, ...)     do { PAL::Trace(TL_ERROR, LOG_MODULE, "%s(%d): %s ERROR  : " fmt, __FILE__, __LINE__, __FUNCSIG__ , ##__VA_ARGS__); } while (0);
#define LOG_FATAL(fmt, ...)     do { PAL::Trace(TL_FATAL, LOG_MODULE, "%s(%d): %S FATAL  : " fmt, __FILE__, __LINE__, __FUNCSIG__ , ##__VA_ARGS__); } while (0);

#ifndef TRACE
// Visual Studio-style TRACE macro: https://msdn.microsoft.com/en-us/library/6w95a4ha.aspx
// Scope: LOG_MODULE
#define TRACE                   LOG_TRACE
#endif

// If BREAK_ON_EXCEPTIONS is set, then each exception would trigger the debug break
// before rethrown to higher level exception handler (helps to debug)
#if defined(_WIN32) && defined(BREAK_ON_EXCEPTIONS)
#include <intrin.h>
#define __DEBUGBREAK __debugbreak
#else
#ifndef __DEBUGBREAK
#define __DEBUGBREAK()
#endif
#endif

#define ACT_RETHROW_EXCEPTION(title, message, ...)                              \
        LOG_ERROR("[Exception rethrown]: %s - %s", title, message);             \
        __DEBUGBREAK();                                                         \
        throw;

#define ACT_TRACE_EXCEPTION(title, message, ...)                                \
        LOG_ERROR("[Exception caught]: %s - %s", title, message);

#ifdef _DEBUG
#define ACT_HANDLE_EXCEPTION(title, message, ...)                               \
        ACT_RETHROW_EXCEPTION(title, message, ...)
#else
#define ACT_HANDLE_EXCEPTION(title, message, ...)                               \
        ACT_TRACE_EXCEPTION(title, message, ...)
#endif

// Note that below we intentionally always want to re-throw bad_alloc exception
// for OS platform to handle this situation of the application. This is better
// than for our shared library to catch and return null pointer, which might lead
// client application into access violation unless it checks the pointer prior to
// every usage of the pointer, which we don't expect client app to do.
// Also std::invalid_argument isn't caught explicitly here given std::logic_error
// is used instead in MapDataRVErrorToException for binary size reduction reason
#ifdef ANDROID
#define CATCH_EXCEPTION_TRACE_OR_RETHROW catch(...) { /* do nothing */ }
#else
#ifdef _WIN32
#pragma warning( disable : 4101)
#endif
#endif
