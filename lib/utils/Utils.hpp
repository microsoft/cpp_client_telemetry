//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef LIB_UTILS_HPP
#define LIB_UTILS_HPP

// This file is being used during both C++ native and C++/CX managed compilation with /cli flag
// Certain features, e.g. <mutex> or <thread> - cannot be used while building with /cli
// For this reason this header cannot include any other headers that rely on <mutex> or <thread>

#include "ctmacros.hpp"
#include "Enums.hpp"
#include "StringConversion.hpp"

#include <chrono>
#include <algorithm>
#include <string>
#include <cstdio>

#include "EventProperty.hpp"

/* Lean implementation of SLDC "Annex K" for non-Windows OS */
#include "annex_k.hpp"

#if (__cplusplus < 201402L) && !defined(_MSC_VER)
/* Workaround for lack of std::make_unique in C++11 (gcc-5). N3936 for C++14 support mentions 201402L */
#include <memory>
namespace std
{
    template <typename T, typename... Args> std::unique_ptr<T> make_unique(Args&&... args) { return std::unique_ptr<T>(new T(std::forward<Args>(args)...)); }
}
#endif

#ifdef __unix__
#ifndef ANDROID
#include <execinfo.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#endif

#if defined(WINDOWS_UWP) || defined(__cplusplus_winrt)
#include <Windows.h>
#define _WINRT
#endif

#ifndef EOK
#define EOK 0
#endif

#ifndef EFAIL
#define EFAIL   -1
#endif

namespace MAT_NS_BEGIN {

    const char* getMATSDKLogComponent();

    typedef std::chrono::milliseconds ms;

    /* Obtain a backtrace and print it to stdout. */
    void print_backtrace(void);

    void sleep(unsigned delayMs);

    long GetCurrentProcessId();

    std::string GetTempDirectory();
    std::string GetAppLocalTempDirectory();

    EventRejectedReason validateEventName(std::string const& name);

    EventRejectedReason validatePropertyName(std::string const& name);

    inline std::string tenantTokenToId(std::string const& tenantToken)
    {
        return tenantToken.substr(0, tenantToken.find('-'));
    }

    inline uint64_t GetUptimeMs()
    {
        return std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
    }

#ifdef _WINRT

    Platform::String ^to_platform_string(const std::string& s);

    std::string from_platform_string(Platform::String ^ ps);

#endif

    unsigned hashCode(const char* str, int h = 0);

} MAT_NS_END

#endif

