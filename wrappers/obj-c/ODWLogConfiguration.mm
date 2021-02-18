//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#import <Foundation/Foundation.h>
#import "ODWLogConfiguration.h"
#import "ODWLogConfiguration_private.h"
#import "ODWLogger_private.h"
#import "LogManager.hpp"

using namespace Microsoft::Applications::Events;

@implementation ODWLogConfiguration
    static bool _enableTrace;
    static bool _enableConsoleLogging;
    static bool _enableSessionReset;
    static bool _surfaceCppExceptions;
    ILogConfiguration* _wrappedConfiguration;

-(instancetype)initWithILogConfiguration:(ILogConfiguration*)config
{
    self = [super init];
    if(self) {
        _wrappedConfiguration = config;
    }
    return self;
}

-(nullable ILogConfiguration*)getWrappedConfiguration
{
    return _wrappedConfiguration;
}

+(nullable ODWLogConfiguration *)getLogConfigurationCopy
{
    auto& config = LogManager::GetLogConfiguration();
    ILogConfiguration configCopy(config);
    return [[ODWLogConfiguration alloc] initWithILogConfiguration: &configCopy];
}

+(void)setEventCollectorUri:(nonnull NSString *)eventCollectorUri
{
    std::string strCollectorUri = std::string([eventCollectorUri UTF8String]);
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_STR_COLLECTOR_URL] = strCollectorUri;
}

+(nullable NSString *)eventCollectorUri
{
    auto& config = LogManager::GetLogConfiguration();
    std::string strCollectorUri = config[CFG_STR_COLLECTOR_URL];
    if (strCollectorUri.empty())
    {
        return nil;
    }
    return [NSString stringWithCString:strCollectorUri.c_str() encoding:NSUTF8StringEncoding];
}

+(void)setCacheMemorySizeLimitInBytes:(uint64_t)cacheMemorySizeLimitInBytes
{
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_INT_RAM_QUEUE_SIZE] = cacheMemorySizeLimitInBytes;
}

+(uint64_t)cacheMemorySizeLimitInBytes
{
    auto& config = LogManager::GetLogConfiguration();
    return config[CFG_INT_RAM_QUEUE_SIZE];
}

+(void)setCacheFileSizeLimitInBytes:(uint64_t)cacheFileSizeLimitInBytes
{
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_INT_CACHE_FILE_SIZE] = cacheFileSizeLimitInBytes;
}

+(uint64_t)cacheFileSizeLimitInBytes
{
    auto& config = LogManager::GetLogConfiguration();
    return config[CFG_INT_CACHE_FILE_SIZE];
}

+(void)setMaxTeardownUploadTimeInSec:(int)maxTeardownUploadTimeInSec
{
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_INT_MAX_TEARDOWN_TIME] = maxTeardownUploadTimeInSec;
}

+(void)setEnableTrace:(bool)enableTrace
{
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_BOOL_ENABLE_TRACE] = enableTrace;
    _enableTrace = enableTrace;
}

+(void)setEnableConsoleLogging:(bool)enableConsoleLogging
{
    _enableConsoleLogging = enableConsoleLogging;
}

+(void)setTraceLevel:(int)traceLevel
{
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_INT_TRACE_LEVEL_MIN] = traceLevel;
}

+(bool)enableTrace
{
    return _enableTrace;
}

+(bool)enableConsoleLogging
{
    return _enableConsoleLogging;
}

+(void)setSurfaceCppExceptions:(bool)surfaceCppExceptions
{
    _surfaceCppExceptions = surfaceCppExceptions;
}

+(bool)surfaceCppExceptions
{
    return _surfaceCppExceptions;
}

+(void)setEnableSessionReset:(bool)enableSessionReset
{
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_BOOL_SESSION_RESET_ENABLED] = enableSessionReset;
    _enableSessionReset = enableSessionReset;
}

+(bool)enableSessionReset
{
    return _enableSessionReset;
}

+(void)setCacheFilePath:(nonnull NSString *)cacheFilePath
{
    std::string strCacheFilePath = std::string([cacheFilePath UTF8String]);
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_STR_CACHE_FILE_PATH] = strCacheFilePath;
}

+(nullable NSString *)cacheFilePath
{
    auto& config = LogManager::GetLogConfiguration();
    std::string strCacheFilePath = config[CFG_STR_CACHE_FILE_PATH];
    if (strCacheFilePath.empty())
    {
        return nil;
    }
    return [NSString stringWithCString:strCacheFilePath.c_str() encoding:NSUTF8StringEncoding];
}

@end
