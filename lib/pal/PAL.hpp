// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <Version.hpp>

#include "typename.hpp"

#include "ctmacros.hpp"

namespace PAL_NS_BEGIN {

    void sleep(unsigned delayMs);

    // Declare log component for the PAL namespace
    // ARIASDK_LOG_DECL_COMPONENT_NS();

    //
    // Debug logging
    //

    // *INDENT-OFF*

    // Declare/define log component for a namespace
#define ARIASDK_LOG_DECL_COMPONENT_NS()                        extern const char* getAriaSdkLogComponent()
#define ARIASDK_LOG_INST_COMPONENT_NS(_name, _desc)            char const* getAriaSdkLogComponent() { return _name; }

// Declare/define log component for a class
#define ARIASDK_LOG_DECL_COMPONENT_CLASS()                     static char const* getAriaSdkLogComponent()
#define ARIASDK_LOG_INST_COMPONENT_CLASS(_class, _name, _desc) char const* _class::getAriaSdkLogComponent() { return _name; }

// *INDENT-ON*

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

#define ARIASDK_SET_LOG_LEVEL_(level_) (PAL::detail::g_logLevel = (level_))

// Check if logging is enabled on a specific level
#define ARIASDK_LOG_ENABLED_(level_)   (PAL::detail::g_logLevel >= (level_))

#define ARIASDK_LOG_ENABLED_DETAIL()   ARIASDK_LOG_ENABLED_(PAL::Detail)
#define ARIASDK_LOG_ENABLED_INFO()     ARIASDK_LOG_ENABLED_(PAL::Info)
#define ARIASDK_LOG_ENABLED_WARNING()  ARIASDK_LOG_ENABLED_(PAL::Warning)
#define ARIASDK_LOG_ENABLED_ERROR()    ARIASDK_LOG_ENABLED_(PAL::Error)

// Log a message on a specific level, which is checked efficiently before evaluating arguments
#define ARIASDK_LOG_(level_, comp_, fmt_, ...)                                    \
    if (ARIASDK_LOG_ENABLED_(level_)) {                                           \
        PAL::detail::log((level_), (comp_), (fmt_), ##__VA_ARGS__); \
    } else static_cast<void>(0)

#define LOG_DEBUG(fmt_, ...)    ARIASDK_LOG_(PAL::Detail,  getAriaSdkLogComponent(), fmt_, ##__VA_ARGS__)
#define LOG_TRACE(fmt_, ...)    ARIASDK_LOG_(PAL::Detail,  getAriaSdkLogComponent(), fmt_, ##__VA_ARGS__)
#define LOG_INFO(fmt_, ...)     ARIASDK_LOG_(PAL::Info,    getAriaSdkLogComponent(), fmt_, ##__VA_ARGS__)
#define LOG_WARN(fmt_, ...)     ARIASDK_LOG_(PAL::Warning, getAriaSdkLogComponent(), fmt_, ##__VA_ARGS__)
#define LOG_ERROR(fmt_, ...)    ARIASDK_LOG_(PAL::Error,   getAriaSdkLogComponent(), fmt_, ##__VA_ARGS__)

} PAL_NS_END // namespace PAL

// See docs/PAL.md for details about the classes, functions and macros a PAL implementation must support.

#ifdef ARIASDK_PAL_SKYPE
#include "PAL_Skype.hpp"
#elif ARIASDK_PAL_WIN32
#include "PAL_Win32.hpp"
#elif ARIASDK_PAL_CPP11
#include "PAL_CPP11.hpp"
#else
#error No platform abstraction library configured. Set one of the ARIASDK_PAL_xxx macros.
#endif
