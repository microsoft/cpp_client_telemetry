#include "objc_begin.h"
#import "ODWEventProperties.h"
#import "ODWSemanticContext.h"

NS_ASSUME_NONNULL_BEGIN

/*!
 @enum ODWTraceLevel
 @brief The <b>ODWTraceLevel</b> enumeration contains a set of values that specify trace level.
 */
typedef NS_ENUM(NSInteger, ODWTraceLevel)
{
    ODWTraceLevelNone = 0,          /**< Turn off all trace messages. */
    ODWTraceLevelError = 1,         /**< Error. */
    ODWTraceLevelWarning = 2,       /**< Warning. */
    ODWTraceLevelInformation = 3,   /**< Information. */
    ODWTraceLevelVerbose = 4,       /**< Verbose. */
};

/*!
 @enum ODWSessionState
 @brief The <b>ODWSessionState</b> enumeration contains a set of values that specify the session state.
 */
typedef NS_ENUM(NSInteger, ODWSessionState)
{
    ODWSessionStateStarted = 0, /**< Session started. */
    ODWSessionStateEnded = 1    /**< Session ended. */
};

/*!
 The <b>ODWLogger</b> class represents an event's properties.
*/
@interface ODWLogger : NSObject

#pragma mark Basic LogEvent methods

/*!
 @brief Logs a custom event with a specified name.
 @param name A string that contains the name of the event.
 */
-(void)logEventWithName:(NSString *)name;

/*!
 @brief Logs a custom event with a specified set of properties.
 @param properties The custom event properties, encapsulated within an ODWEventProperties object.
 */
-(void)logEventWithEventProperties:(ODWEventProperties *)properties;

#pragma mark Semantic LogEvent methods
/*!
 @brief Logs a failure event (such as an application exception), taking a signature, failure details, and event properties.
 @param signature A string that identifies the bucket of the failure.
 @param detail A string that contains a description of the failure.
 @param properties Properties of the failure event, encapsulated within an ODWEventProperties object. <b>Note:</b> This value can be null.
 */
-(void)logFailureWithSignature:(NSString *)signature
                         detail:(NSString *)detail
                eventproperties:(ODWEventProperties *)properties;

/*!
 @brief Logs a failure event&mdash;such as an application exception.
 @param signature A string that identifies the bucket of the failure.
 @param detail A string that contains a description of the failure.
 @param category A string that contains the category of the failure&mdash;such as an application error, the spplication stops responding, or a crash.
 @param identifier A string that contains that uniquely identifies this failure.
 @param properties Properties of the failure event, encapsulated within an ODWEventProperties object.
 */
-(void)logFailureWithSignature:(NSString *)signature
                         detail:(NSString *)detail
                       category:(NSString *)category
                             id:(NSString *)identifier
                eventProperties:(ODWEventProperties *)properties;

/*!
 @brief Logs a page action event.
 @param identifier A string that contains that uniquely identifies the page view.
 @param pageName The page name.
 @param properties Properties of the page action event, encapsulated within an ODWEventProperties object. Properties could be used to log the action name.
 */
-(void)logPageViewWithId:(NSString *)identifier
                 pageName:(NSString *)pageName
          eventProperties:(ODWEventProperties *)properties;

/*!
 @brief Logs a page action event.
 @param identifier A string that contains that uniquely identifies the page view.
 @param pageName The page name.
 @param category A string that contains the name of the category this page belongs to.
 @param uri A string that contains the URI of this page.
 @param referrerUri A string that contains the URI that refers to this page.
 @param properties Properties of the page action event, encapsulated within an ODWEventProperties object. Properties could be used to log the action name.
 */
-(void)logPageViewWithId:(NSString *)identifier
                 pageName:(NSString *)pageName
                 category:(NSString *)category
                      uri:(NSString *)uri
              referrerUri:(NSString *)referrerUri
          eventProperties:(ODWEventProperties *)properties;

/*!
 @brief Logs a diagnostic trace event to help you troubleshoot problems.
 @param traceLevel The level of the trace as one of the ::ODWTraceLevel enumeration values.
 @param message A string that contains a description of the trace.
 @param properties Properties of the trace event, encapsulated within an ODWEventProperties object.
 */
-(void)logTraceWithTraceLevel:(enum ODWTraceLevel)traceLevel
                       message:(NSString *)message
               eventProperties:(ODWEventProperties *)properties;

/*!
 @brief Logs the session state.
 <b>Note:</b> Logging <i>session start</i> while a session already exists produces a no-op.
 Similarly, logging <i>session end</i> while a session doesn't exist also produces a no-op.
 @param state The session's state as one of the ::ODWSessionState enumeration values.
 @param properties Properties of the session event, encapsulated within an ODWEventProperties object.
 */
-(void)logSessionWithState:(enum ODWSessionState)state
            eventProperties:(ODWEventProperties *)properties;

/*!
 @brief Sets a string context property for an event.
 @param name A string that contains the name of the context property.
 @param value A string that contains the context property value.
 */
-(void)setContext:(NSString *)name
  withStringValue:(NSString *)value;

/*!
 @brief Sets a string context property for an event.
 @param name A string that contains the name of the context property.
 @param value A string that contains the context property value.
 @param piiKind The kind of Personal Identifiable Information (PII), as one of the ::ODWPiiKind enumeration values.
*/
- (void)setContext:(NSString*)name
   withStringValue:(NSString *)value
       withPiiKind:(ODWPiiKind)piiKind;

/*!
 @brief Sets a double context property for an event.
 @param name A string that contains the name of the context property.
 @param value A double that contains the context property value.
 */
-(void)setContext:(NSString*)name
  withDoubleValue:(double)value;

/*!
 @brief Sets a double context property for an event.
 @param name A string that contains the name of the context property.
 @param value A double that contains the context property value.
 @param piiKind The kind of Personal Identifiable Information (PII), as one of the ::ODWPiiKind enumeration values.
 */
-(void)setContext:(NSString*)name
  withDoubleValue:(double)value
      withPiiKind:(ODWPiiKind)piiKind;

/*!
 @brief Sets an integer context property for an event.
 @param name A string that contains the name of the context property.
 @param value An integer that contains the context property value.
 */
-(void)setContext:(NSString*)name
   withInt64Value:(int64_t)value;

/*!
 @brief Sets an integer context property for an event.
 @param name A string that contains the name of the context property.
 @param value An integer that contains the context property value.
 @param piiKind The kind of Personal Identifiable Information (PII), as one of the ::ODWPiiKind enumeration values.
 */
-(void)setContext:(NSString*)name
   withInt64Value:(int64_t)value
      withPiiKind:(ODWPiiKind)piiKind;

/*!
 @brief Sets an integer context property for an event.
 @param name A string that contains the name of the context property.
 @param value An integer that contains the context property value.
 */
-(void)setContext:(NSString*)name
   withInt32Value:(int32_t)value;

/*!
 @brief Sets an integer context property for an event.
 @param name A string that contains the name of the context property.
 @param value An integer that contains the context property value.
 @param piiKind The kind of Personal Identifiable Information (PII), as one of the ::ODWPiiKind enumeration values.
 */
-(void)setContext:(NSString*)name
   withInt32Value:(int32_t)value
      withPiiKind:(ODWPiiKind)piiKind;

/*!
 @brief Sets a BOOL context property for an event.
 @param name A string that contains the name of the context property.
 @param value A BOOL that contains the context property value.
 */
-(void)setContext:(NSString*)name
    withBoolValue:(BOOL)value;

/*!
 @brief Sets a BOOL context property for an event.
 @param name A string that contains the name of the context property.
 @param value A BOOL that contains the context property value.
 @param piiKind The kind of Personal Identifiable Information (PII), as one of the ::ODWPiiKind enumeration values.
 */
-(void)setContext:(NSString*)name
    withBoolValue:(BOOL)value
      withPiiKind:(ODWPiiKind)piiKind;

/*!
 @brief Sets a UUID context property for an event.
 @param name A string that contains the name of the context property.
 @param value A UUID that contains the context property value.
 */
-(void)setContext:(NSString*)name
    withUUIDValue:(NSUUID*)value;

/*!
 @brief Sets a UUID context property for an event.
 @param name A string that contains the name of the context property.
 @param value A UUID that contains the context property value.
 @param piiKind The kind of Personal Identifiable Information (PII), as one of the ::ODWPiiKind enumeration values.
 */
-(void)setContext:(NSString*)name
    withUUIDValue:(NSUUID*)value
      withPiiKind:(ODWPiiKind)piiKind;

/*!
 @brief Sets a date context property for an event.
 @param name A string that contains the name of the context property.
 @param value A date that contains the context property value.
 */
-(void)setContext:(NSString*)name
    withDateValue:(NSDate*)value;

/*!
 @brief Sets a date context property for an event.
 @param name A string that contains the name of the context property.
 @param value A date that contains the context property value.
 @param piiKind The kind of Personal Identifiable Information (PII), as one of the ::ODWPiiKind enumeration values.
 */
-(void)setContext:(NSString*)name
    withDateValue:(NSDate*)value
      withPiiKind:(ODWPiiKind)piiKind;

/*!
Semantic context for this ODWLogger
 */
@property (NS_NONATOMIC_IOSONLY, readonly, strong, nonnull) ODWSemanticContext* semanticContext;

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
