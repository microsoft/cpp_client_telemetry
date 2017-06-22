#pragma once
#include "Enums.hpp"
#include <string>
#include <stdint.h>

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
            class WindowsEnvironmentInfo
            {
            public:
                static OsArchitectureType GetProcessorArchitecture();
                static std::string GetTimeZone();			

			};
        }
    }
}