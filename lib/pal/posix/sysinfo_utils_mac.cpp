// Copyright (c) Microsoft Corporation. All rights reserved.

#include "sysinfo_utils_apple.hpp"
#include <sys/sysctl.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <vector>

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
    uuid_t uuidBytes;
    const struct timespec spec = {1, 0};
    int hostUUIDResult = gethostuuid(uuidBytes, &spec);
    if (hostUUIDResult == 0)
    {
        char deviceGuid[37];
        uuid_unparse(uuidBytes, deviceGuid);
        std::string deviceId{deviceGuid};
        return deviceId;
    }

    return { };
}
