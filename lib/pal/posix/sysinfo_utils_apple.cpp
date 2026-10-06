//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "sysinfo_utils_apple.hpp"
#include <sys/sysctl.h>

std::string get_sysctl_value(const char* key)
{
    std::string result{};
    // OS version will not be longer than 9 chars (XX.YY.ZZ + null), and the OS build is also 9 (nnXmmmmY + null)
    char value[11] = {};
    size_t size = sizeof(value) - 1;
    if (sysctlbyname(key, value, &size, NULL, 0) == 0)
    {
        result.assign(value);
    }

    return result;
}

std::string GetDeviceOsBuild()
{
    return get_sysctl_value("kern.osversion");
}
