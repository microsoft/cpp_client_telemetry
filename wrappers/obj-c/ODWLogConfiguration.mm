#import <Foundation/Foundation.h>
#import "ODWLogConfiguration.h"
#import "LogManager.hpp"

using namespace Microsoft::Applications::Events;

@implementation ODWLogConfiguration

+(void)setMaxTeardownUploadTimeInSec:(int)maxTeardownUploadTimeInSec
{
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_INT_MAX_TEARDOWN_TIME] = maxTeardownUploadTimeInSec;
}

@end
