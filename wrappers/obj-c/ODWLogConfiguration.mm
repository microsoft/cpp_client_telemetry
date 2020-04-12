#import <Foundation/Foundation.h>
#import "ODWLogConfiguration.h"
#import "LogManager.hpp"

using namespace Microsoft::Applications::Events;

@implementation ODWLogConfiguration
    static bool _enableTrace;

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

+(bool)enableTrace
{
    return _enableTrace;
}

@end
