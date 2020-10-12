#ifndef LIB_PAL_POSIX_SYSINFO_UTILS_IOS_HPP_
#define LIB_PAL_POSIX_SYSINFO_UTILS_IOS_HPP_
//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
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

