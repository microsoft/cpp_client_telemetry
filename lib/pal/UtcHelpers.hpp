#pragma once
#include "utils/Utils.hpp"

namespace ARIASDK_NS_BEGIN {
namespace PAL {

bool IsUtcRegistrationEnabledinWindows();
bool RegisterIkeyWithWindowsTelemetry(std::string const& ikeyin, int storageSize, int uploadQuotaSize);
std::string GetAppLocalTempDirectory();
} // namespace PAL
} ARIASDK_NS_END