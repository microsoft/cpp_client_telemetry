#ifdef _WIN32
#pragma once
#include "utils/Utils.hpp"

namespace PAL_NS_BEGIN {

    bool IsUtcRegistrationEnabledinWindows();
    bool RegisterIkeyWithWindowsTelemetry(std::string const& ikeyin, int storageSize, int uploadQuotaSize);

} PAL_NS_END
#endif
