// Copyright (c) Microsoft. All rights reserved.
#pragma once
#include "Enums.hpp"
#include <string>
#include <stdint.h>

namespace ARIASDK_NS_BEGIN {

    class WindowsEnvironmentInfo
    {
    public:
        static OsArchitectureType GetProcessorArchitecture();
        static std::string GetTimeZone();
    };

} ARIASDK_NS_END
