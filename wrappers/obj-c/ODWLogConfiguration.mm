#import <Foundation/Foundation.h>
#import "ODWLogConfiguration.h"
#import "ODWLogger_private.h"
#import "LogManager.hpp"

using namespace Microsoft::Applications::Events;

@implementation ODWLogConfiguration
    static bool _enableTrace;
    static bool _surfaceCppExceptions;

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
    return [NSString stringWithUTF8String:strCollectorUri.c_str()];
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

+(void)setTraceLevel:(int)traceLevel
{
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_INT_TRACE_LEVEL_MIN] = traceLevel;
}

+(bool)enableTrace
{
    return _enableTrace;
}

+(void)setSurfaceCppExceptions:(bool)surfaceCppExceptions
{
    _surfaceCppExceptions = surfaceCppExceptions;
}

+(bool)surfaceCppExceptions
{
    return _surfaceCppExceptions;
}

@end
