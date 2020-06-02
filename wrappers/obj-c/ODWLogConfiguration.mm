#import <Foundation/Foundation.h>
#import "ODWLogConfiguration.h"
#import "ODWLogger_private.h"
#import "LogManager.hpp"

using namespace Microsoft::Applications::Events;

@implementation ODWLogConfiguration
    static bool _enableTrace;
    static bool _surfaceCppExceptions;

+(void)setMaxTeardownUploadTimeInSec:(int)maxTeardownUploadTimeInSec
{
    try
    {
        auto& config = LogManager::GetLogConfiguration();
        config[CFG_INT_MAX_TEARDOWN_TIME] = maxTeardownUploadTimeInSec;
	}
    catch (const std::exception &e)
    {
        if ([ODWLogConfiguration surfaceCppExceptions])
        {
            throw;
        }
        [ODWLogger traceException: e.what()];
    }
    catch (const std::exception *e)
    {
        if ([ODWLogConfiguration surfaceCppExceptions])
        {
            throw;
        }
        [ODWLogger traceException: e->what()];
    }
}

+(void)setEnableTrace:(bool)enableTrace
{
    try
    {
        auto& config = LogManager::GetLogConfiguration();
        config[CFG_BOOL_ENABLE_TRACE] = enableTrace;
        _enableTrace = enableTrace;
    }
    catch (const std::exception &e)
    {
        if ([ODWLogConfiguration surfaceCppExceptions])
        {
            throw;
        }
        [ODWLogger traceException: e.what()];
    }
    catch (const std::exception *e)
    {
        if ([ODWLogConfiguration surfaceCppExceptions])
        {
            throw;
        }
        [ODWLogger traceException: e->what()];
    }
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
