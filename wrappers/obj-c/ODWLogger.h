#include "objc_begin.h"
#import "ODWEventProperties.h"

NS_ASSUME_NONNULL_BEGIN

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


@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
