#include "objc_begin.h"

NS_ASSUME_NONNULL_BEGIN

/*!
 @brief The ODWSemanticContext class manages the inclusion of semantic context values on logged events
*/
@interface ODWSemanticContext : NSObject

/*!
 @brief Specifies an application id to be included with every event
 @param appId A string that contains an application identifier
 */
-(void)setAppId:(NSString *)appId;

/*!
 @brief Specifies a unique user id to be included with every event
 @param userId A string that contains the unique user identifier.
 */
-(void)setUserId:(nonnull NSString *)userId;

/*!
 @brief Specifies a unique user advertising id to be included with every event
 @param userAdvertisingId AA string that contains the unique user advertising identifier.
 */
-(void)setUserAdvertisingId:(nonnull NSString *)userAdvertisingId;

/*!
 @brief Sets the experiment tag (experiment configuration) context information for telemetry events.
 <b>Note:</b> This method removes any previously stored experiment IDs that were set using setAppExperimentETag.
 @param eTag A string that contains the ETag which is a hash of the set of experiments.
 */
-(void)setAppExperimentETag:(nonnull NSString *)eTag;

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
