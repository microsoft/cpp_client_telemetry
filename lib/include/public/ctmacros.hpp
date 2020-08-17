///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2020 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CTMACROS_HPP
#define CTMACROS_HPP

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
#define UNREFERENCED_PARAMETER(...)
#define OACR_USE_PTR(...)
#define _Out_writes_bytes_(...)

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
