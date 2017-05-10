// Copyright (c) Microsoft. All rights reserved.
// This file has been automatically generated, manual changes will be lost.

#pragma once
#include <stdint.h>


//#define Telemetry Telemetry_v5

#define ARIASDK_NS_BEGIN Microsoft { namespace Applications { namespace Telemetry
#define ARIASDK_NS_END   }}
#define ARIASDK_NS       Microsoft::Applications::Telemetry


namespace Microsoft { namespace Applications { namespace Telemetry {


uint64_t const Version =
    (5ull << 48) |
    (0ull << 32) |
    (0ull << 16) |
     0ull;

char const VersionString[] =
    "5.0.0.0";


namespace {
#ifdef __GNUC__
    #define ARIASDK_UNUSED __attribute__((unused))
#else
    #define ARIASDK_UNUSED
#endif
}


#define ARIASDK_PAL_WIN32 1


}}} // namespace Microsoft::Applications::Telemetry
