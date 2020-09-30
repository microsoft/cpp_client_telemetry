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
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_STR_COLLECTOR_URL] = eventCollectorUri;
}

+(void)setCacheMemorySizeLimitInBytes:(int)cacheMemorySizeLimitInBytes;
{
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_INT_RAM_QUEUE_SIZE] = cacheMemorySizeLimitInBytes;
}

+(void)setCacheFileSizeLimitInBytes:(int)cacheFileSizeLimitInBytes;
{
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_INT_CACHE_FILE_SIZE] = cacheFileSizeLimitInBytes;
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
