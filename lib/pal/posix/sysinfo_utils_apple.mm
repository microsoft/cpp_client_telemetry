// Copyright (c) Microsoft Corporation. All rights reserved.

#import <Foundation/Foundation.h>
#include "sysinfo_utils_apple.hpp"

NSDictionary* copy_system_plist_dictionary(void)
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSData *versionData = nil;
    NSString *systemVersFile = @"/System/Library/CoreServices/SystemVersion.plist";
    NSString *serverVersFile = @"/System/Library/CoreServices/ServerVersion.plist";
    
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

std::string GetDeviceOsName()
{
    NSString* value = get_system_value(@"ProductName");
    return std::string([value UTF8String]);
}

std::string GetDeviceOsVersion()
{
    NSString* value = get_system_value(@"ProductVersion");
    return std::string([value UTF8String]);
}

std::string GetDeviceOsRelease()
{
    NSString* value = get_system_value(@"ProductUserVisibleVersion");
    return std::string([value UTF8String]);
}

std::string GetDeviceOsBuild()
{
    NSString* value = get_system_value(@"ProductBuildVersion");
    return std::string([value UTF8String]);
}
