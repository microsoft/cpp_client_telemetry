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
#  elif defined(MATSDK_STATIC_LIB)
#    define MATSDK_LIBABI
#  else // Header file included by client
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
#define MATSDK_LIBABI
#endif

// TODO: [MG] - ideally we'd like to use __attribute__((unused)) with gcc/clang
#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(...)
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

/* Implement struct packing for stable FFI C API to allow for C# apps
 * written in Mono and .NET Standard 2.x to call into 1DS C API.
 */
#ifdef HAVE_MAT_ABI_V3_1_0
/* Legacy v3.1 struct ABI. Not compatible with cross-plat C# projection */
#define MATSDK_PACKED_STRUCT
#define MATSDK_PACK_PUSH
#define MATSDK_PACK_POP
#define MATSDK_ALIGN64(x)   x
/* Modern v3.7 struct ABI. Compatible with cross-plat C# callers on both
 * 32-bit and 64-bit Intel and ARM architectures - on Windows, Android
 * and Mac. This should ideally be the default going forward, as it
 * ensures predictable, compiler optimization level-agnostic C API FFI.
 */
#elif __clang__
# define MATSDK_PACKED_STRUCT __attribute__((packed))
# define MATSDK_PACK_PUSH
# define MATSDK_PACK_POP
#define MATSDK_ALIGN64(x) union { x; uint64_t padding; }
#elif __GNUC__
# define MATSDK_PACKED_STRUCT __attribute__((packed))
# define MATSDK_PACK_PUSH
# define MATSDK_PACK_POP
#define MATSDK_ALIGN64(x) union { x; uint64_t padding; }
#elif _MSC_VER
# define MATSDK_PACKED_STRUCT
# define MATSDK_PACK_PUSH     __pragma(pack(push, 1))
# define MATSDK_PACK_POP      __pragma(pack(pop))
#define MATSDK_ALIGN64(x) union { x; uint64_t padding; }
#else
/* Fallback to HAVE_MAT_ABI_V3_1_0 : compatible with prebuilt shared libraries
 * that used the old C API only within the same arch/compiler domain. Unfortunately
 * the old layout is not usable if you'd like to invoke C API from Mono (e.g. Unity)
 * or cross-platform .NET Standard apps.
 */
#ifndef HAVE_MAT_ABI_V3_1_0
#define HAVE_MAT_ABI_V3_1_0
#endif
# define MATSDK_PACKED_STRUCT
# define MATSDK_PACK_PUSH
# define MATSDK_PACK_POP
#define MATSDK_ALIGN64(x) x
#endif

#endif
