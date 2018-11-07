#pragma once
// WARNING: DO NOT MODIFY THIS FILE!
// Copyright (c) Microsoft. All rights reserved.
// This file has been automatically generated, manual changes will be lost.
#define BUILD_VERSION_STR "3.0.309.0"
#define BUILD_VERSION 3,0,309,0

#ifndef RESOURCE_COMPILER_INVOKED
#include <stdint.h>

#define ARIASDK_NS_BEGIN Microsoft { namespace Applications { namespace Events
#define ARIASDK_NS_END   }}
#define ARIASDK_NS       Microsoft::Applications::Events
#define MAT              ::ARIASDK_NS

#define PAL_NS_BEGIN     Microsoft { namespace Applications { namespace Events { namespace PlatformAbstraction
#define PAL_NS_END       }}}
#define PAL              ::Microsoft::Applications::Events::PlatformAbstraction

#define MAT_v1           ::Microsoft::Applications::Telemetry

namespace ARIASDK_NS_BEGIN {

uint64_t const Version =
	((uint64_t)3 << 48) |
	((uint64_t)0 << 32) |
	((uint64_t)309 << 16) |
	((uint64_t)0);

// TODO: [MG] - move declaration of ARIA_SDK_UNUSED to separate include file
#ifdef ARIASDK_UNUSED
#elif defined(__GNUC__) || defined(__clang__)
# define ARIASDK_UNUSED(x) (x) /* __attribute__((unused)) */
#elif defined(__LCLINT__)
# define ARIASDK_UNUSED(x) /**/ x
#elif defined(__cplusplus)
# define ARIASDK_UNUSED(x)
#else
# define ARIASDK_UNUSED(x) x
#endif

# define ARIASDK_PAL_WIN32 1

} ARIASDK_NS_END

namespace PAL_NS_BEGIN { } PAL_NS_END

#endif // RESOURCE_COMPILER_INVOKED
