//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef DEBUGTRACE_HPP
#define DEBUGTRACE_HPP

#include "mat/config.h"

#include "ctmacros.hpp"
#include "typename.hpp"

namespace PAL_NS_BEGIN
{

#ifndef HAVE_MAT_LOGGING
    /* No logging */
#define LOG_DEBUG(fmt, ...)
#define LOG_TRACE(fmt, ...)
#define LOG_INFO(fmt,  ...)
#define LOG_WARN(fmt,  ...)
#define LOG_ERROR(fmt, ...)
#else
#ifdef NDEBUG
#define LOG_DEBUG(fmt_, ...)
#else
#define LOG_DEBUG(fmt_, ...)    MATSDK_LOG_(PAL::Detail,  getMATSDKLogComponent(), fmt_, ##__VA_ARGS__)
#endif
#define LOG_TRACE(fmt_, ...)    MATSDK_LOG_(PAL::Detail,  getMATSDKLogComponent(), fmt_, ##__VA_ARGS__)
#define LOG_INFO(fmt_, ...)     MATSDK_LOG_(PAL::Info,    getMATSDKLogComponent(), fmt_, ##__VA_ARGS__)
#define LOG_WARN(fmt_, ...)     MATSDK_LOG_(PAL::Warning, getMATSDKLogComponent(), fmt_, ##__VA_ARGS__)
#define LOG_ERROR(fmt_, ...)    MATSDK_LOG_(PAL::Error,   getMATSDKLogComponent(), fmt_, ##__VA_ARGS__)
#endif

    // Declare/define log component for a namespace
#define MATSDK_LOG_DECL_COMPONENT_NS()                        const char* getMATSDKLogComponent()
#define MATSDK_LOG_INST_COMPONENT_NS(_name, _desc)            char const* getMATSDKLogComponent() { return _name; }

// Declare/define log component for a class
#define MATSDK_LOG_DECL_COMPONENT_CLASS()                     static char const* getMATSDKLogComponent()
#define MATSDK_LOG_INST_COMPONENT_CLASS(_class, _name, _desc) char const* _class::getMATSDKLogComponent() { return _name; }

    // Declare log component for the PAL namespace
    MATSDK_LOG_DECL_COMPONENT_NS();

    enum LogLevel {
        Error = 1,
        Warning = 2,
        Info = 3,
        Detail = 4
    };

    namespace detail {
        extern LogLevel g_logLevel;
        extern void log(LogLevel level, char const* component, char const* fmt, ...);
    } // namespace detail

#define MATSDK_SET_LOG_LEVEL_(level_) (PAL::detail::g_logLevel = (level_))

// Check if logging is enabled on a specific level
#define MATSDK_LOG_ENABLED_(level_)   (PAL::detail::g_logLevel >= (level_))

#define MATSDK_LOG_ENABLED_DETAIL()   MATSDK_LOG_ENABLED_(PAL::Detail)
#define MATSDK_LOG_ENABLED_INFO()     MATSDK_LOG_ENABLED_(PAL::Info)
#define MATSDK_LOG_ENABLED_WARNING()  MATSDK_LOG_ENABLED_(PAL::Warning)
#define MATSDK_LOG_ENABLED_ERROR()    MATSDK_LOG_ENABLED_(PAL::Error)

// Log a message on a specific level, which is checked efficiently before evaluating arguments
#define MATSDK_LOG_(level_, comp_, fmt_, ...)                                    \
    if (MATSDK_LOG_ENABLED_(level_)) {                                           \
        PAL::detail::log((level_), (comp_), (fmt_), ##__VA_ARGS__); \
    } else static_cast<void>(0)

} PAL_NS_END // namespace PAL

#endif
