#pragma once
#include "Enums.hpp"
#include <string>
#include <stdint.h>

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
#define SPL_PATH_SEPARATOR_CHAR '\\'
            class WindowsEnvironmentInfo
            {
            public:
                static OsArchitectureType GetProcessorArchitecture();
                static std::string GetTimeZone();			

			};
        }
    }
}