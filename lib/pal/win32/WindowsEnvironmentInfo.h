#pragma once
#include "Enums.hpp"
#include <string>
#include <stdint.h>

namespace Microsoft {
    namespace Applications {
        namespace Events  {
            class WindowsEnvironmentInfo
            {
            public:
                static OsArchitectureType GetProcessorArchitecture();
                static std::string GetTimeZone();			

			};
        }
    }
}