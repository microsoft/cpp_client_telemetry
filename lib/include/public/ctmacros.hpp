/*
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef CTMACROS_HPP
#define CTMACROS_HPP

#ifdef  HAVE_MAT_SHORT_NS
#define MAT_NS_BEGIN  MAT
#define MAT_NS_END
#define PAL_NS_BEGIN  PAL
#define PAL_NS_END
#else
#define MAT_NS_BEGIN  Microsoft { namespace Applications { namespace Events
#define MAT_NS_END    }}
#define MAT           ::Microsoft::Applications::Events
#define PAL_NS_BEGIN  Microsoft { namespace Applications { namespace Events { namespace PlatformAbstraction
#define PAL_NS_END    }}}
#define PAL           ::Microsoft::Applications::Events::PlatformAbstraction
#endif

#define MAT_v1        ::Microsoft::Applications::Telemetry

#ifdef _WIN32       // Windows platforms

#ifndef MATSDK_SPEC // we use __cdecl by default
#define MATSDK_SPEC __cdecl
#define MATSDK_LIBABI_CDECL __cdecl
#  if defined(MATSDK_SHARED_LIB)
#    define MATSDK_LIBABI __declspec(dllexport)
#  elif defined(MATSDK_IMPORT_LIB)
// Consumer importing the public API from a shared mat.dll. The installed
// MSTelemetry::mat CMake target propagates this automatically when the SDK was
// built shared (see lib/CMakeLists.txt).
#    define MATSDK_LIBABI __declspec(dllimport)
#  elif defined(MATSDK_STATIC_LIB)
#    define MATSDK_LIBABI
#  else // Header file included by client; linkage unspecified
#    ifndef MATSDK_LIBABI
#    define MATSDK_LIBABI
#    endif
#  endif
#endif

#else               // Non-windows platforms

#ifndef MATSDK_SPEC 
#define MATSDK_SPEC
#endif

#ifndef MATSDK_LIBABI_CDECL
#define MATSDK_LIBABI_CDECL
#endif

#ifndef MATSDK_LIBABI
// Mark the public API as default-visibility ONLY in shared builds, so it stays
// exported when the SDK is compiled with -fvisibility=hidden (see CMakeLists.txt).
// This mirrors the __declspec(dllexport) gating above: in a static build the
// attribute is omitted, so the public symbols inherit -fvisibility=hidden and are
// NOT re-exported when a consumer .so/.dylib statically absorbs libmat. (When a
// consumer includes this header, MATSDK_SHARED_LIB is not defined either, which
// is fine: the symbols are exported by the shared libmat they link against.)
#  if (defined(__GNUC__) || defined(__clang__)) && defined(MATSDK_SHARED_LIB)
#    define MATSDK_LIBABI __attribute__((visibility("default")))
#  else
#    define MATSDK_LIBABI
#  endif
#endif

// Cast the argument(s) to void so the parameter is genuinely referenced. An empty
// expansion left the parameter unused under -Wunused-parameter, which broke
// consumers compiling the SDK headers with -Wextra -Werror. On Windows the Win32
// SDK provides its own UNREFERENCED_PARAMETER, so this definition only applies
// where that macro is not already defined.
#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(...) (void)(__VA_ARGS__)
#endif

#define OACR_USE_PTR(...)

#ifndef _Out_writes_bytes_
#define _Out_writes_bytes_(...)
#endif

#endif

#ifdef MATSDK_UNUSED
#elif defined(__GNUC__) || defined(__clang__)
# define MATSDK_UNUSED(x) (x) /* __attribute__((unused)) */
#elif defined(__LCLINT__)
# define MATSDK_UNUSED(x) /*@unused@*/ x
#elif defined(__cplusplus)
# define MATSDK_UNUSED(x)
#else
# define MATSDK_UNUSED(x) x
#endif

#define STRINGIZE_DETAIL(x)         #x
#define STRINGIZE(x)                STRINGIZE_DETAIL(x)
#define STRINGIFY(x)                #x
#define TOKENPASTE(x, y)            x ## y
#define TOKENPASTE2(x, y)           TOKENPASTE(x, y)

// Macro for mutex issues debugging. Supports both std::mutex and std::recursive_mutex
#define LOCKGUARD(macro_mutex)      LOG_DEBUG("LOCKGUARD   lockin at %s:%d", __FILE__, __LINE__); std::lock_guard<decltype(macro_mutex)> TOKENPASTE2(__guard_, __LINE__) (macro_mutex); LOG_DEBUG("LOCKGUARD   locked at %s:%d", __FILE__, __LINE__);

#if defined(_WIN32) || defined(_WIN64)
#ifdef _WIN64
#define ARCH_64BIT
#else
#define ARCH_32BIT
#endif
#endif

#if __GNUC__
#if defined(__x86_64__) || defined(__ppc64__)
#define ARCH_64BIT
#else
#define ARCH_32BIT
#endif
#endif

/* Exceptions support is optional */
#if (__cpp_exceptions) || defined(__EXCEPTIONS)
#define HAVE_EXCEPTIONS 1
#else
#define HAVE_EXCEPTIONS 0
#endif

// allow to disable exceptions
#if (HAVE_EXCEPTIONS)
# define MATSDK_TRY            try
# define MATSDK_CATCH          catch
# define MATSDK_THROW          throw
#else
# define MATSDK_TRY            if (true)
# define MATSDK_CATCH(...)     if (false)
# define MATSDK_THROW(...)     std::abort()
#endif

#if defined(__arm__) || defined(_M_ARM) || defined(_M_ARMT)
/* TODO: add support for 64-bit aarch64 */
#define ARCH_ARM
#endif

#define EVTSDK_LIBABI       MATSDK_LIBABI
#define EVTSDK_LIBABI_CDECL MATSDK_LIBABI_CDECL
#define EVTSDK_SPEC         MATSDK_SPEC

#endif
