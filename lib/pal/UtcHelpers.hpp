#pragma once
// Copyright (c) Microsoft Corporation. All rights reserved.
#include "mat/config.h"
#ifdef HAVE_MAT_UTC

#include "utils/Utils.hpp"

namespace PAL_NS_BEGIN {

    bool IsUtcRegistrationEnabledinWindows();
    bool RegisterIkeyWithWindowsTelemetry(std::string const& ikeyin, int storageSize, int uploadQuotaSize);

} PAL_NS_END
#endif
