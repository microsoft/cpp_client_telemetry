// Copyright (c) Microsoft Corporation. All rights reserved.

#include "sysinfo_utils_apple.hpp"
#include <sys/sysctl.h>
#include <vector>
#import <Foundation/Foundation.h>

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

std::string GetDeviceId()
{
    @autoreleasepool {
        uuid_t uuidBytes;
        const struct timespec spec = {1, 0};
        int hostUUIDResult = gethostuuid(uuidBytes, &spec);
        if (hostUUIDResult == 0)
        {
            std::string deviceId { [[[[NSUUID alloc] initWithUUIDBytes:uuidBytes] UUIDString] UTF8String] };
            return deviceId;
        }

        return { };
    }
}
