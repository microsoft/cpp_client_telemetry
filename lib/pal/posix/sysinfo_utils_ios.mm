// Copyright (c) Microsoft Corporation. All rights reserved.

#include "sysinfo_utils_ios.hpp"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <sys/utsname.h>

std::string GetDeviceModel()
{
#if TARGET_IPHONE_SIMULATOR
    NSString* modelId = NSProcessInfo.processInfo.environment[@"SIMULATOR_MODEL_IDENTIFIER"];
    return std::string([modelId UTF8String]);
#else
    utsname name;
    uname(&name);
    return std::string(name.machine);
#endif
}

std::string GetDeviceId()
{
    NSUUID *nsuuid = [[UIDevice currentDevice] identifierForVendor];
    if (nsuuid)
    {
        std::string deviceId { [[nsuuid UUIDString] UTF8String] };
        return deviceId;
    }
    else
    {
        std::string emptyString;
        return emptyString;
    }
}
