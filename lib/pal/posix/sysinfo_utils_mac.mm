//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "sysinfo_utils_apple.hpp"
#include <sys/sysctl.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <vector>
#import <Foundation/Foundation.h>

#define EMPTY_GUID "00000000-0000-0000-0000-000000000000"

std::string GetDeviceModel()
{
    static const char *query = "hw.model";
    size_t size = 0;
    std::vector<char> deviceModelBuffer;
    std::string deviceModel { };
    if (sysctlbyname(query, nullptr, &size, nullptr, 0) == 0)
    {
        deviceModelBuffer.resize(size);
        if (sysctlbyname(query, deviceModelBuffer.data(), &size, nullptr, 0) == 0)
        {
            deviceModel = deviceModelBuffer.data();
        }
    }

    return deviceModel;
}

std::string GetDeviceOsName()
{
    return std::string("Mac OS X");
}

std::string GetDeviceId()
{
    uuid_t uuidBytes;
    const struct timespec spec = {1, 0};
    int hostUUIDResult = gethostuuid(uuidBytes, &spec);
    if (hostUUIDResult == 0)
    {
        char deviceGuid[37] = {0};
        uuid_unparse(uuidBytes, deviceGuid);
        std::string deviceId{deviceGuid};
        return deviceId;
    }

    return {EMPTY_GUID};
}

std::string GetDeviceOsVersion()
{
    // Previous implementation pointed to "ProductVersion" on SystemVersion.plist, returning version string in format <major>.<minor>.<patch>
    // kern.osproductversion returns string in this same format
    std::string version = get_sysctl_value("kern.osproductversion");
    if (version.empty())
    {
        // kern.osproductversion is not available on 10.10, build it from NSProcessInfo
        @autoreleasepool
        {
            NSOperatingSystemVersion versionOs = [[NSProcessInfo processInfo] operatingSystemVersion];
            if (versionOs.patchVersion != 0)
            {
                version.assign([[NSString stringWithFormat:@"%ld.%ld.%ld", versionOs.majorVersion, versionOs.minorVersion, versionOs.patchVersion] UTF8String]);
            }
            else
            {
                version.assign([[NSString stringWithFormat:@"%ld.%ld", versionOs.majorVersion, versionOs.minorVersion] UTF8String]);
            }
        }
    }
    
    return version;
}

std::string GetDeviceOsRelease()
{
    // Previous implementation pointed to "ProductUserVisibleVersion" on SystemVersion.plist, returning version string in format <major>.<minor>.<patch>
    // GetDeviceOsVersion() returns string in this same format
    return GetDeviceOsVersion();
}

std::string GetDeviceClass() {
  return {};
}
