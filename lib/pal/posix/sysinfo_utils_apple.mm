// Copyright (c) Microsoft Corporation. All rights reserved.

#import <Foundation/Foundation.h>
#include "sysinfo_utils_apple.hpp"

NSDictionary* copy_system_plist_dictionary(void)
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSData *versionData = nil;
#if TARGET_IPHONE_SIMULATOR
    NSString *simulatorRoot = NSProcessInfo.processInfo.environment[@"IPHONE_SIMULATOR_ROOT"] ?: @"";
    NSString *systemVersFile = [simulatorRoot stringByAppendingPathComponent:@"/System/Library/CoreServices/SystemVersion.plist"];
    NSString *serverVersFile = [simulatorRoot stringByAppendingPathComponent:@"/System/Library/CoreServices/ServerVersion.plist"];
#else
    NSString *systemVersFile = @"/System/Library/CoreServices/SystemVersion.plist";
    NSString *serverVersFile = @"/System/Library/CoreServices/ServerVersion.plist";
#endif
    
    if([fileManager isReadableFileAtPath:systemVersFile])
        versionData = [NSData dataWithContentsOfFile:systemVersFile];
    else if([fileManager isReadableFileAtPath:serverVersFile])
        versionData = [NSData dataWithContentsOfFile:serverVersFile];
    
    if(versionData == nil)
        return nil;
    
    return [NSPropertyListSerialization propertyListWithData:versionData options:NSPropertyListImmutable format:nil error:nil];
}

NSString* get_system_value(NSString* key)
{
    static NSDictionary* systemProperties = copy_system_plist_dictionary();
    if (systemProperties)
    {
        NSString* propertyValue = [systemProperties objectForKey:key];
        if (propertyValue)
        {
            return propertyValue;
        }
    }
    
    return @"";
}

std::string GetDeviceOsVersion()
{
    NSString* value = get_system_value(@"ProductVersion");
    return std::string([value UTF8String]);
}

std::string GetDeviceOsRelease()
{
#if TARGET_OS_IPHONE
    NSString* value = get_system_value(@"ProductVersion");
#else
    NSString* value = get_system_value(@"ProductUserVisibleVersion");
#endif
    return std::string([value UTF8String]);
}

std::string GetDeviceOsBuild()
{
    NSString* value = get_system_value(@"ProductBuildVersion");
    return std::string([value UTF8String]);
}
