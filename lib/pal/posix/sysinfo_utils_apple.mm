// Copyright (c) Microsoft Corporation. All rights reserved.

#import <Foundation/Foundation.h>
#include "sysinfo_utils_apple.hpp"

CFDictionaryRef copy_system_plist_dictionary(void)
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSData *versionData = nil;
    NSString *systemVersFile = @"/System/Library/CoreServices/SystemVersion.plist";
    NSString *serverVersFile = @"/System/Library/CoreServices/ServerVersion.plist";
    NSDictionary *properties = nil;
    
    if([fileManager isReadableFileAtPath:systemVersFile])
        versionData = [NSData dataWithContentsOfFile:systemVersFile];
    else if([fileManager isReadableFileAtPath:serverVersFile])
        versionData = [NSData dataWithContentsOfFile:serverVersFile];
    
    if(versionData == nil)
        return nil;
    
    properties = [NSPropertyListSerialization propertyListWithData:versionData options:NSPropertyListImmutable format:nil error:nil];
    
    return (CFDictionaryRef) [properties retain];
}

NSString* get_system_value(CFStringRef key)
{
    CFDictionaryRef systemProperties = copy_system_plist_dictionary();
    if (systemProperties)
    {
        CFStringRef propertyValue = (CFStringRef) CFDictionaryGetValue(systemProperties, key);
        if (propertyValue)
        {
            return (NSString *)propertyValue;
        }
        
        CFRelease(systemProperties);
    }
    
    return @"";
}

std::string get_device_osName()
{
    NSString* value = get_system_value(CFSTR("ProductName"));
    return std::string([value UTF8String]);
}

std::string get_device_osVersion()
{
    NSString* value = get_system_value(CFSTR("ProductVersion"));
    return std::string([value UTF8String]);
}

std::string get_device_osRelease()
{
    NSString* value = get_system_value(CFSTR("ProductUserVisibleVersion"));
    return std::string([value UTF8String]);
}

std::string get_device_osBuild()
{
    NSString* value = get_system_value(CFSTR("ProductBuildVersion"));
    return std::string([value UTF8String]);
}
