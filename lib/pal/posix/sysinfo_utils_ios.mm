//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "sysinfo_utils_apple.hpp"
#include "utils/Utils.hpp"
#import <Foundation/Foundation.h>
#include <TargetConditionals.h>
#import <sys/utsname.h>
#import <UIKit/UIKit.h>

#if defined(TARGET_OS_VISION) && TARGET_OS_VISION
#define MATSDK_TARGET_OS_VISION 1
#else
#define MATSDK_TARGET_OS_VISION 0
#endif

#if defined(__VISION_OS_VERSION_MAX_ALLOWED) || (defined(__IPHONE_OS_VERSION_MAX_ALLOWED) && (__IPHONE_OS_VERSION_MAX_ALLOWED >= 170000))
#define MATSDK_HAS_UI_USER_INTERFACE_IDIOM_VISION 1
#else
#define MATSDK_HAS_UI_USER_INTERFACE_IDIOM_VISION 0
#endif

#if defined(__IPHONE_OS_VERSION_MAX_ALLOWED) && (__IPHONE_OS_VERSION_MAX_ALLOWED >= 140000)
#define MATSDK_HAS_UI_USER_INTERFACE_IDIOM_MAC 1
#else
#define MATSDK_HAS_UI_USER_INTERFACE_IDIOM_MAC 0
#endif

std::string GetDeviceModel()
{
    @autoreleasepool {
#if TARGET_IPHONE_SIMULATOR
        NSString* modelId = NSProcessInfo.processInfo.environment[@"SIMULATOR_MODEL_IDENTIFIER"];
        if (modelId.length > 0)
        {
            return MAT::boundedSystemInfo([modelId UTF8String]);
        }

        NSString* fallbackModel = [[UIDevice currentDevice] model];
        if (fallbackModel.length > 0)
        {
            return MAT::boundedSystemInfo([fallbackModel UTF8String]);
        }

        return {};
#else
        std::string deviceModel { };
        struct utsname systemInfo;
        if (uname(&systemInfo) < 0)
        {
		    // Fallback to UIDevice in case of error
		    deviceModel = MAT::boundedSystemInfo([[[UIDevice currentDevice] model] UTF8String]);
        }
        else
        {
		    deviceModel = systemInfo.machine;
        }

        return deviceModel;
#endif
    }
}

std::string GetDeviceOsName()
{
#if MATSDK_TARGET_OS_VISION
    return std::string("visionOS");
#else
    return std::string("iOS");
#endif
}

std::string GetDeviceId()
{
#ifndef MATSDK_DISABLE_DEVICE_ID
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
#else
    return {};
#endif
}

std::string GetDeviceOsVersion()
{
    // Previous implementation pointed to "ProductVersion" on SystemVersion.plist, returning version string in format <major>.<minor>.<patch>
    // systemVersion returns string in this same format
    return MAT::boundedSystemInfo([[[UIDevice currentDevice] systemVersion] UTF8String]);
}

std::string GetDeviceOsRelease()
{
    // Previous implementation pointed to "ProductUserVisibleVersion" on SystemVersion.plist, returning version string in format <major>.<minor>.<patch>
    // systemVersion returns string in this same format
    return MAT::boundedSystemInfo([[[UIDevice currentDevice] systemVersion] UTF8String]);
}

std::string GetDeviceClass() {
#if MATSDK_TARGET_OS_VISION
#if defined(TARGET_OS_SIMULATOR) && TARGET_OS_SIMULATOR
    return "visionOS.Emulator";
#else
    return "visionOS.Vision";
#endif
#elif defined(TARGET_IPHONE_SIMULATOR) && TARGET_IPHONE_SIMULATOR
    return "iOS.Emulator";
#else
    switch (UIDevice.currentDevice.userInterfaceIdiom) {
        case UIUserInterfaceIdiomPhone:
            return "iOS.Phone";
        case UIUserInterfaceIdiomPad:
            return "iOS.Tablet";
        case UIUserInterfaceIdiomTV:
            return "iOS.AppleTV";
#if MATSDK_HAS_UI_USER_INTERFACE_IDIOM_MAC
        case UIUserInterfaceIdiomMac:
            return "iOS.Desktop";
#endif
#if MATSDK_HAS_UI_USER_INTERFACE_IDIOM_VISION
        case UIUserInterfaceIdiomVision:
            return "visionOS.Vision";
#endif
        default:
            return {};
    }
#endif
}
