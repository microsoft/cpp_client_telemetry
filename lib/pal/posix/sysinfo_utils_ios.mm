// Copyright (c) Microsoft Corporation. All rights reserved.

#include "sysinfo_utils_ios.hpp"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

std::string get_device_model()
{
    std::string deviceModel { [[[UIDevice currentDevice] model] UTF8String] };
    return deviceModel;
}

std::string get_device_id()
{
    std::string deviceId { [[[[UIDevice currentDevice] identifierForVendor] UUIDString] UTF8String] };
    return deviceId;
}
