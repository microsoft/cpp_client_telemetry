#include "ILogger.hpp"
#import <Foundation/Foundation.h>
#import "ODWLogger_private.h"
#import "ODWLogConfiguration.h"

using namespace MAT;

@implementation ODWLogger
{
    ILogger* _wrappedLogger;
}

-(instancetype)initWithILogger:(ILogger*)logger
{
    self = [super init];
    if(self){
        _wrappedLogger = logger;
		if([ODWLogConfiguration enableTrace])
		{
	        NSLog(@"Logger initialized successfully");
		}
    }
    return self;
}

-(void) logEventWithName:(NSString *)name
{
    std::string eventName = std::string([name UTF8String]);
    EventProperties event(eventName);
    _wrappedLogger->LogEvent(event);
	if([ODWLogConfiguration enableTrace])
	{
	    NSLog(@"Log event with name: %@",name);
	}
}

-(void) unwrapEventProperties: (ODWEventProperties*) wrappedProperties onEvent:(EventProperties&) event
{
    std::string strName = std::string([[wrappedProperties name] UTF8String]);
    event.SetName(strName);
    ODWEventPriority priority = [wrappedProperties priority];
    if (priority != ODWEventPriorityUnspecified)
    {
        event.SetPriority((EventPriority)priority);
    }

    NSDictionary* props = [wrappedProperties properties];
    NSDictionary* piiTags = [wrappedProperties piiTags];
    for(NSString* propertyName in props){
        NSObject* value = [props objectForKey: propertyName];
		PiiKind piiKind = (PiiKind)[[piiTags objectForKey: propertyName] integerValue];
        std::string strPropertyName = std::string([propertyName UTF8String]);
        if([value isKindOfClass: [NSNumber class]]){
            NSNumber* num = (NSNumber*)value;
            if(strcmp([num objCType], @encode(BOOL))==0 ) {
                event.SetProperty(strPropertyName, [num boolValue] ? true : false, piiKind);
            }
            else if( strcmp([num objCType], @encode(int))==0 ){
                event.SetProperty(strPropertyName, [num intValue], piiKind);
            }else{
                event.SetProperty(strPropertyName, [num floatValue], piiKind);
            }
        }else{
            NSString* str = (NSString*)value;
            event.SetProperty(strPropertyName, [str UTF8String], piiKind);
        }
    }
}

-(void) logEventWithEventProperties: (nonnull ODWEventProperties*) properties
{
    EventProperties event;
    [self unwrapEventProperties: properties onEvent: event];
    _wrappedLogger->LogEvent(event);
    if([ODWLogConfiguration enableTrace])
    {
        NSLog(@"Log event with name: %@",[properties name]);
    }
}

-(void) logFailureWithSignature: (nonnull NSString*) signature
                         detail: (nonnull NSString*) detail
                eventproperties: (nonnull ODWEventProperties*) properties
{
    EventProperties event;
    [self unwrapEventProperties: properties onEvent: event];

    std::string strSignature = std::string([signature UTF8String]);
    std::string strDetail    = std::string([detail UTF8String]);

    _wrappedLogger->LogFailure(strSignature, strDetail, event);
    if([ODWLogConfiguration enableTrace])
    {
        NSLog(@"Log failure with signature: %@, detail: %@ and name: %@",signature, detail, [properties name]);
    }
}

-(void) logFailureWithSignature: (nonnull NSString*) signature
                         detail: (nonnull NSString*) detail
                       category: (nonnull NSString*) category
                             id: (nonnull NSString*) identifier
                eventProperties: (nonnull ODWEventProperties*) properties
{
    EventProperties event;
    [self unwrapEventProperties: properties onEvent: event];

    std::string strSignature = std::string([signature UTF8String]);
    std::string strDetail    = std::string([detail UTF8String]);
    std::string strCategory  = std::string([category UTF8String]);
    std::string strId        = std::string([identifier UTF8String]);

    _wrappedLogger->LogFailure(strSignature, strDetail, strCategory, strId, event);
    if([ODWLogConfiguration enableTrace])
    {
        NSLog(@"Log failure with signature %@, detail %@, category: %@, id: %@ and name: %@",signature, detail, category, identifier, [properties name]);
    }
}

-(void) logPageViewWithId: (nonnull NSString*) identifier
                 pageName: (nonnull NSString*) pageName
          eventProperties: (nonnull ODWEventProperties*) properties
{
    EventProperties event;
    [self unwrapEventProperties: properties onEvent: event];

    std::string strId        = std::string([identifier UTF8String]);
    std::string strPageName  = std::string([pageName UTF8String]);

    _wrappedLogger->LogPageView(strId, strPageName, event);
    if([ODWLogConfiguration enableTrace])
    {
        NSLog(@"Log page view with id: %@, page name: %@ and name %@", identifier, pageName, [properties name]);
    }
}

-(void) logPageViewWithId: (nonnull NSString*) identifier
                 pageName: (nonnull NSString*) pageName
                 category: (nonnull NSString*) category
                      uri: (nonnull NSString*) uri
              referrerUri: (nonnull NSString*) referrerUri
          eventProperties: (nonnull ODWEventProperties*) properties
{
    EventProperties event;
    [self unwrapEventProperties: properties onEvent: event];

    std::string strId          = std::string([identifier UTF8String]);
    std::string strPageName    = std::string([pageName UTF8String]);
    std::string strCategory    = std::string([category UTF8String]);
    std::string strUri         = std::string([uri UTF8String]);
    std::string strReferrerUri = std::string([referrerUri UTF8String]);

    _wrappedLogger->LogPageView(strId, strPageName, strCategory, strUri, strReferrerUri, event);
    if([ODWLogConfiguration enableTrace])
    {
        NSLog(@"Log page view with id: %@, page name: %@, category: %@, uri: %@, referrer uri: %@ and name %@", identifier, pageName, category, uri, referrerUri, [properties name]);
    }
}

-(void) logTraceWithTraceLevel: (enum ODWTraceLevel)traceLevel
                       message: (nonnull NSString *)message
               eventProperties: (nonnull ODWEventProperties *)properties
{
    EventProperties event;
    [self unwrapEventProperties: properties onEvent: event];

    std::string strMessage = std::string([message UTF8String]);

    _wrappedLogger->LogTrace((TraceLevel)traceLevel, strMessage, event);
    if([ODWLogConfiguration enableTrace])
    {
        NSLog(@"Log trace with level: %@, message: %@, name: %@", @(traceLevel), message, [properties name]);
    }
}

-(void) logSessionWithState: (enum ODWSessionState)state
            eventProperties: (nonnull ODWEventProperties *)properties
{
    EventProperties event;
    [self unwrapEventProperties: properties onEvent: event];
    _wrappedLogger->LogSession((SessionState)state, event);
    if([ODWLogConfiguration enableTrace])
    {
        NSLog(@"Log session with state: %@, name: %@", @(state), [properties name]);
    }
}

@end
