#pragma once
#include "Utils.hpp"

namespace ARIASDK_NS_BEGIN {
namespace PAL {

bool __cdecl IsUtcRegistrationEnabledinWindows();
bool __cdecl RegisterIkeyWithWindowsTelemetry(std::string const& ikeyin, int storageSize, int uploadQuotaSize);
} // namespace PAL
} ARIASDK_NS_END