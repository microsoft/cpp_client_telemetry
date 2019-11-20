// Copyright (c) Microsoft Corporation. All rights reserved.

#include "sysinfo_utils_ios.hpp"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

std::string GetDeviceModel()
{
    std::string deviceModel { [[[UIDevice currentDevice] model] UTF8String] };
    return deviceModel;
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
