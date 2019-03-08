#pragma once
// WARNING: DO NOT MODIFY THIS FILE!
// Copyright (c) Microsoft. All rights reserved.
// This file has been automatically generated, manual changes will be lost.
#define BUILD_VERSION_STR "3.1.64.1"
#define BUILD_VERSION 3,1,64,1

#ifndef RESOURCE_COMPILER_INVOKED
#include <stdint.h>

#ifdef HAVE_MAT_SHORT_NS
/* Short NS name reduces .DLL size */
#define ARIASDK_NS_BEGIN MAT
#define ARIASDK_NS       MAT
#define ARIASDK_NS_END   
#define PAL_NS_BEGIN     PAL
#define PAL_NS_END       
#else
#define ARIASDK_NS_BEGIN Microsoft { namespace Applications { namespace Events
#define ARIASDK_NS       Microsoft::Applications::Events
#define ARIASDK_NS_END   }}

#define PAL_NS_BEGIN     Microsoft { namespace Applications { namespace Events { namespace PlatformAbstraction
#define PAL_NS_END       }}}
#define PAL              ::Microsoft::Applications::Events::PlatformAbstraction
#define MAT              ::ARIASDK_NS
#endif

#define MAT_v1           ::Microsoft::Applications::Telemetry

namespace ARIASDK_NS_BEGIN {

uint64_t const Version =
	((uint64_t)3 << 48) |
	((uint64_t)1 << 32) |
	((uint64_t)64 << 16) |
	((uint64_t)1);

// TODO: [MG] - move declaration of ARIA_SDK_UNUSED to separate include file
#ifdef ARIASDK_UNUSED
#elif defined(__GNUC__) || defined(__clang__)
# define ARIASDK_UNUSED(x) (x) /* __attribute__((unused)) */
#elif defined(__LCLINT__)
# define ARIASDK_UNUSED(x) /*@unused@*/ x
#elif defined(__cplusplus)
# define ARIASDK_UNUSED(x)
#else
# define ARIASDK_UNUSED(x) x
#endif

# define ARIASDK_PAL_WIN32 1

} ARIASDK_NS_END

namespace PAL_NS_BEGIN { } PAL_NS_END

#endif // RESOURCE_COMPILER_INVOKED

