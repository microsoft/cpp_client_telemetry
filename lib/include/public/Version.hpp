// Copyright (c) Microsoft. All rights reserved.
#ifndef MAT_VERSION_HPP
#define MAT_VERSION_HPP
// WARNING: DO NOT MODIFY THIS FILE!
// This file has been automatically generated, manual changes will be lost.
#define BUILD_VERSION_STR "3.4.183.1"
#define BUILD_VERSION 3,4,183,1

#ifndef RESOURCE_COMPILER_INVOKED
#include <stdint.h>

#ifdef HAVE_MAT_SHORT_NS
#define ARIASDK_NS_BEGIN  MAT
#define ARIASDK_NS_END
#define PAL_NS_BEGIN  PAL
#define PAL_NS_END
#else
#define ARIASDK_NS_BEGIN  Microsoft { namespace Applications { namespace Events
#define ARIASDK_NS_END    }}
#define MAT           ::Microsoft::Applications::Events
#define PAL_NS_BEGIN  Microsoft { namespace Applications { namespace Events { namespace PlatformAbstraction
#define PAL_NS_END    }}}
#define PAL           ::Microsoft::Applications::Events::PlatformAbstraction
#endif

#define MAT_v1        ::Microsoft::Applications::Telemetry

namespace ARIASDK_NS_BEGIN {

uint64_t const Version =
    ((uint64_t)3 << 48) |
    ((uint64_t)4 << 32) |
    ((uint64_t)183 << 16) |
    ((uint64_t)1);

} ARIASDK_NS_END

namespace PAL_NS_BEGIN { } PAL_NS_END

#endif // RESOURCE_COMPILER_INVOKED
#endif

