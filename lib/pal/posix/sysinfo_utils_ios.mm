//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "sysinfo_utils_apple.hpp"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

std::string GetDeviceModel()
{
    @autoreleasepool {
#if TARGET_IPHONE_SIMULATOR
        NSString* modelId = NSProcessInfo.processInfo.environment[@"SIMULATOR_MODEL_IDENTIFIER"];
        return std::string([modelId UTF8String]);
#else
        std::string deviceModel { [[[UIDevice currentDevice] model] UTF8String] };
        return deviceModel;
#endif
    }
}

std::string GetDeviceOsName()
{
    return std::string("iOS");
}

std::string GetDeviceId()
{
    @autoreleasepool {
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
}

std::string GetDeviceOsVersion()
{
    // Previous implementation pointed to "ProductVersion" on SystemVersion.plist, returning version string in format <major>.<minor>.<patch>
    // systemVersion returns string in this same format
    return std::string { [[[UIDevice currentDevice] systemVersion] UTF8String] };
}

std::string GetDeviceOsRelease()
{
    // Previous implementation pointed to "ProductUserVisibleVersion" on SystemVersion.plist, returning version string in format <major>.<minor>.<patch>
    // systemVersion returns string in this same format
    return std::string { [[[UIDevice currentDevice] systemVersion] UTF8String] };
}

