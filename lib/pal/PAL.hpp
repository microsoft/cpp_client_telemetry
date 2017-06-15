// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <Version.hpp>

// See docs/PAL.md for details about the classes, functions and macros a PAL implementation must support.


#ifdef ARIASDK_PAL_SKYPE
    #include "PAL_Skype.hpp"
#elif ARIASDK_PAL_WIN32
    #include "PAL_Win32.hpp"
#else
    #error No platform abstraction library configured. Set one of the ARIASDK_PAL_xxx macros.
#endif


namespace ARIASDK_NS_BEGIN {
namespace PAL {


ARIASDK_LOG_DECL_COMPONENT_NS();


} // namespace PAL
} ARIASDK_NS_END
