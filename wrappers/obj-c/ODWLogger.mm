#include "ILogger.hpp"
#import <Foundation/Foundation.h>
#import "ODWLogger_private.h"
#import "ODWLogConfiguration.h"
#import "ODWSemanticContext.h"
#import "ODWSemanticContext_private.h"
#import "ODWPrivacyGuard_private.h"

#include "EventProperties.hpp"
#include "LogSessionData.hpp"

using namespace MAT;

@implementation ODWLogger
{
    ILogger* _wrappedLogger;
}

@synthesize semanticContext = _semanticContext;

-(instancetype)initWithILogger:(ILogger*)logger
{
    self = [super init];
    if(self){
        _wrappedLogger = logger;
		if([ODWLogConfiguration enableTrace])
		{
	        NSLog(@"Logger initialized successfully");
		}
        _semanticContext = [[ODWSemanticContext alloc] initWithISemanticContext:_wrappedLogger->GetSemanticContext()];
    }
    return self;
}

-(void) logEventWithName:(NSString *)name
{
    std::string eventName = std::string([name UTF8String]);
    EventProperties event(eventName);
    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->LogEvent(event);
    });
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
            else if( (strcmp([num objCType], @encode(float))==0) || (strcmp([num objCType], @encode(double))==0) || (strcmp([num objCType], @encode(long double))==0) ){
                event.SetProperty(strPropertyName, [num floatValue], piiKind);
            }else{
                event.SetProperty(strPropertyName, [num longLongValue], piiKind);
            }
        }
        else if([value isKindOfClass: [NSDate class]])
        {
            time_ticks_t ticks { static_cast<uint64_t>((static_cast<NSDate*>(value).timeIntervalSince1970 * ticksPerSecond) + ticksUnixEpoch) };
            event.SetProperty(strPropertyName, ticks, piiKind);
        }
        else if([value isKindOfClass: [NSUUID class]])
        {
            uuid_t uuidBytes;
            NSUUID* nsuuid = (NSUUID*)value;
            [nsuuid getUUIDBytes:uuidBytes];
            GUID_t guid { uuidBytes, true /*bigEndian*/ };
            event.SetProperty(strPropertyName, guid, piiKind);
        }
        else
        {
            NSString* str = (NSString*)value;
            event.SetProperty(strPropertyName, [str UTF8String], piiKind);
        }
    }
}

-(void) logEventWithEventProperties: (nonnull ODWEventProperties*) properties
{
    EventProperties event;
    [self unwrapEventProperties: properties onEvent: event];
    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->LogEvent(event);
    });
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

    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->LogFailure(strSignature, strDetail, event);
    });
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

    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->LogFailure(strSignature, strDetail, strCategory, strId, event);
    });
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

    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->LogPageView(strId, strPageName, event);
    });
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

    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->LogPageView(strId, strPageName, strCategory, strUri, strReferrerUri, event);
    });
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

    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->LogTrace((TraceLevel)traceLevel, strMessage, event);
    });
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
    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->LogSession((SessionState)state, event);
    });
    if([ODWLogConfiguration enableTrace])
    {
        NSLog(@"Log session with state: %@, name: %@", @(state), [properties name]);
    }
}

-(void)setContextWithName:(nonnull NSString*)name
              stringValue:(nonnull NSString*)value
{
    std::string strName = std::string([name UTF8String]);
    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->SetContext(strName, value);
    });
}

-(void)setContextWithName:(nonnull NSString*)name
              stringValue:(nonnull NSString*)value
                  piiKind:(enum ODWPiiKind)piiKind
{
    std::string strName = std::string([name UTF8String]);
    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->SetContext(strName, value, (PiiKind)piiKind);
    });
}

-(void)setContextWithName:(nonnull NSString*)name
                boolValue:(BOOL)value
{
    std::string strName = std::string([name UTF8String]);
    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->SetContext(strName, value);
    });
}

-(void)setContextWithName:(nonnull NSString*)name
                boolValue:(BOOL)value
                  piiKind:(enum ODWPiiKind)piiKind
{
    std::string strName = std::string([name UTF8String]);
    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->SetContext(strName, value, (PiiKind)piiKind);
    });
}

-(void)setContextWithName:(nonnull NSString*)name
                dateValue:(nonnull NSDate*)value
{
    std::string strName = std::string([name UTF8String]);
    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->SetContext(strName, value);
    });
}

-(void)setContextWithName:(nonnull NSString*)name
                dateValue:(nonnull NSDate*)value
                  piiKind:(enum ODWPiiKind)piiKind
{
    std::string strName = std::string([name UTF8String]);
    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->SetContext(strName, value, (PiiKind)piiKind);
    });
}

-(void)setContextWithName:(nonnull NSString*)name
              doubleValue:(double)value
{
    std::string strName = std::string([name UTF8String]);
    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->SetContext(strName, value);
    });
}

-(void)setContextWithName:(nonnull NSString*)name
              doubleValue:(double)value
                  piiKind:(enum ODWPiiKind)piiKind
{
    std::string strName = std::string([name UTF8String]);
    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->SetContext(strName, value, (PiiKind)piiKind);
    });
}

-(void)setContextWithName:(nonnull NSString*)name
                longValue:(int64_t)value
{
    std::string strName = std::string([name UTF8String]);
    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->SetContext(strName, value);
    });
}

-(void)setContextWithName:(nonnull NSString*)name
                longValue:(int64_t)value
                  piiKind:(enum ODWPiiKind)piiKind
{
    std::string strName = std::string([name UTF8String]);
    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->SetContext(strName, value, (PiiKind)piiKind);
    });
}

-(void)setContextWithName:(nonnull NSString*)name
                UUIDValue:(nonnull NSUUID*)value
{
    std::string strName = std::string([name UTF8String]);
    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->SetContext(strName, value);
    });
}

-(void)setContextWithName:(nonnull NSString*)name
                UUIDValue:(nonnull NSUUID*)value
                  piiKind:(enum ODWPiiKind)piiKind
{
    std::string strName = std::string([name UTF8String]);
    PerformActionWithCppExceptionsCatch(^(void) {
        _wrappedLogger->SetContext(strName, value, (PiiKind)piiKind);
    });
}

+(void)traceException:(const char *)message
{
    if([ODWLogConfiguration enableTrace])
    {
        NSLog(EXCEPTION_TRACE_FORMAT, message);
    }
}

+(void)raiseException:(const char *)message
{
    [NSException raise:@"1DSSDKException" format:[NSString stringWithFormat:@"%s", message]];
}

void PerformActionWithCppExceptionsCatch(void (^block)())
{
    try
    {
        block();
    }
    catch (const std::exception &e)
    {
        if ([ODWLogConfiguration surfaceCppExceptions])
        {
            [ODWLogger raiseException: e.what()];
        }
        [ODWLogger traceException: e.what()];
    }
}

-(void)initializePrivacyGuardWithODWCommonDataContext:(ODWCommonDataContext *)commonDataContextsObject
{    
    [ODWPrivacyGuard initializePrivacyGuard:_wrappedLogger withODWCommonDataContext:commonDataContextsObject];
}

@end
