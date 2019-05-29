#import <Foundation/Foundation.h>
#include "LogManager.hpp"
#import "WLogger.h"
#include <cstdio>

NS_ASSUME_NONNULL_BEGIN

using namespace MAT;

@interface WLogManager : NSObject

+(nullable id) initForTenant: (nonnull NSString*) tenantToken;
+(nullable id) getLogger;
+(nullable id) getLoggerForSource: (nonnull NSString*) source;

+(void) uploadNow;
+(void) flush;
+(void) pauseTransmission;
+(void) resumeTransmission;
+(void) flushAndTeardown;
+(void) resetTransmitProfiles;

@end

NS_ASSUME_NONNULL_END