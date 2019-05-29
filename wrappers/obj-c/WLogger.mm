#import "WLogger.h"

@implementation WLogger

-(id) initWithILogger : (ILogger*) logger{
    self = [super init];
    if(self){
        wrappedLogger = logger;
        NSLog(@"Logger initialized successfully");
    }
    return self;
}

-(void) logEventWithName:(NSString *)name{
    std::string eventName = std::string([name UTF8String]);
    EventProperties event(eventName);
    wrappedLogger->LogEvent(event);
    NSLog(@"Log event with name: %@",name);
}

-(void) unwrapWEventProperties: (WEventProperties*) wrappedProperties onEvent:(EventProperties&) event{
    std::string strName = std::string([[wrappedProperties getName] UTF8String]);
    event.SetName(strName);
    NSMutableDictionary* props = [wrappedProperties getProperties];
    for(NSString* propertyName in props){
        NSObject* value = [props objectForKey: propertyName];
        std::string strPropertyName = std::string([propertyName UTF8String]);
        if([value isKindOfClass: [NSNumber class]]){
            NSNumber* num = (NSNumber*)value;
            if(strcmp([num objCType], @encode(BOOL))==0 ) {
                event.SetProperty(strPropertyName, [num boolValue] ? true : false);
            }
            else if( strcmp([num objCType], @encode(int))==0 ){
                event.SetProperty(strPropertyName, [num intValue]);
            }else{
                event.SetProperty(strPropertyName, [num floatValue]);
            }
        }else{
            NSString* str = (NSString*)value;
            event.SetProperty(strPropertyName, [str UTF8String]);
        }
    }
}

-(void) logEventWithWEventProperties: (nonnull WEventProperties*) properties{
    EventProperties event;
    [self unwrapWEventProperties: properties onEvent: event];
    wrappedLogger->LogEvent(event);
    NSLog(@"Log event with name: %@",[properties getName]);
}

-(void) logFailureWithSignature: (nonnull NSString*) aSignature 
                         detail: (nonnull NSString*) aDetail
                eventproperties: (nonnull WEventProperties*) properties{
    EventProperties event;
    [self unwrapWEventProperties: properties onEvent: event];

    std::string strSignature = std::string([aSignature UTF8String]);
    std::string strDetail    = std::string([aDetail UTF8String]);

    wrappedLogger->LogFailure(strSignature, strDetail, event);
    NSLog(@"Log failure with signature: %@, detail: %@ and name: %@",aSignature, aDetail, [properties getName]);
}

-(void) logFailureWithSignature: (nonnull NSString*) aSignature 
                         detail: (nonnull NSString*) aDetail
                       category: (nonnull NSString*) aCategory
                             id: (nonnull NSString*) anId
                eventProperties: (nonnull WEventProperties*) properties{
    EventProperties event;
    [self unwrapWEventProperties: properties onEvent: event];

    std::string strSignature = std::string([aSignature UTF8String]);
    std::string strDetail    = std::string([aDetail UTF8String]);
    std::string strCategory  = std::string([aCategory UTF8String]);
    std::string strId        = std::string([anId UTF8String]);

    wrappedLogger->LogFailure(strSignature, strDetail, strCategory, strId, event);
    NSLog(@"Log failure with signature %@, detail %@, category: %@, id: %@ and name: %@",aSignature, aDetail, aCategory, anId, [properties getName]);
}

-(void) logPageViewWithId: (nonnull NSString*) anId
                 pageName: (nonnull NSString*) aPageName
          eventProperties: (nonnull WEventProperties*) properties{
    EventProperties event;
    [self unwrapWEventProperties: properties onEvent: event];
    
    std::string strId        = std::string([anId UTF8String]);
    std::string strPageName  = std::string([aPageName UTF8String]);

    wrappedLogger->LogPageView(strId, strPageName, event);
    NSLog(@"Log page view with id: %@, page name: %@ and name %@", anId, aPageName, [properties getName]);
}

-(void) logPageViewWithId: (nonnull NSString*) anId
                 pageName: (nonnull NSString*) aPageName
                 category: (nonnull NSString*) aCategory
                      uri: (nonnull NSString*) anUri
              referrerUri: (nonnull NSString*) aReferrerUri
          eventProperties: (nonnull WEventProperties*) properties{
    EventProperties event;
    [self unwrapWEventProperties: properties onEvent: event];

    std::string strId          = std::string([anId UTF8String]);
    std::string strPageName    = std::string([aPageName UTF8String]);
    std::string strCategory    = std::string([aCategory UTF8String]);
    std::string strUri         = std::string([anUri UTF8String]);
    std::string strReferrerUri = std::string([aReferrerUri UTF8String]);

    wrappedLogger->LogPageView(strId, strPageName, strCategory, strUri, strReferrerUri, event);
    NSLog(@"Log page view with id: %@, page name: %@, category: %@, uri: %@, referrer uri: %@ and name %@", anId, aPageName, aCategory, anUri, aReferrerUri, [properties getName]);
}



@end
