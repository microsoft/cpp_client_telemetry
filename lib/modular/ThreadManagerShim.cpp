#ifndef CUSTOM_ARIA_THREADMANAGER
// Copyright (c) Microsoft. All rights reserved.
#include "Version.hpp"

#include "pal/PAL.hpp"

#include "ModularShims.hpp"

namespace ARIASDK_NS_BEGIN
{
    // Let our customer override the implementation of GetThreadManager routine if necessary
    IThreadManager* GetThreadManager(const IDebugLogger* debugLogger)
    {
        return new PAL::DefaultThreadManager(debugLogger);
    }

} ARIASDK_NS_END
#endif