#import <Foundation/Foundation.h>
#import "WEventProperties.h"
#include "ILogger.hpp"

NS_ASSUME_NONNULL_BEGIN

using namespace MAT;

@interface WLogger : NSObject{
    ILogger* wrappedLogger;
}

-(id) initWithILogger : (ILogger*) logger;

-(void) logEventWithName: (nonnull NSString*) name;

-(void) logEventWithWEventProperties: (nonnull WEventProperties*) properties;

-(void) logFailureWithSignature: (nonnull NSString*) aSignature 
                         detail: (nonnull NSString*) aDetail
                eventproperties: (nonnull WEventProperties*) properties;

-(void) logFailureWithSignature: (nonnull NSString*) aSignature 
                         detail: (nonnull NSString*) aDetail
                       category: (nonnull NSString*) aCategory
                             id: (nonnull NSString*) anId
                eventProperties: (nonnull WEventProperties*) properties;

-(void) logPageViewWithId: (nonnull NSString*) anID
                 pageName: (nonnull NSString*) aPageName
          eventProperties: (nonnull WEventProperties*) properties;

-(void) logPageViewWithId: (nonnull NSString*) anId
                 pageName: (nonnull NSString*) aPageName
          eventProperties: (nonnull WEventProperties*) properties;

-(void) logPageViewWithId: (nonnull NSString*) anId
                 pageName: (nonnull NSString*) aPageName
                 category: (nonnull NSString*) aCategory
                      uri: (nonnull NSString*) anUri
              referrerUri: (nonnull NSString*) aReferrerUri
          eventProperties: (nonnull WEventProperties*) properties;


@end

NS_ASSUME_NONNULL_END