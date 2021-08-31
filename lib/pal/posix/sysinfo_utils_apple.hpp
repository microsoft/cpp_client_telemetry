#ifndef LIB_PAL_POSIX_SYSINFO_UTILS_IOS_HPP_
#define LIB_PAL_POSIX_SYSINFO_UTILS_IOS_HPP_
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include <string>

std::string get_sysctl_value(const char* key);

std::string GetDeviceOsName();

std::string GetDeviceOsVersion();

std::string GetDeviceOsRelease();

std::string GetDeviceOsBuild();

std::string GetDeviceModel();

std::string GetDeviceId();

#endif /* LIB_PAL_POSIX_SYSINFO_UTILS_IOS_HPP_ */

