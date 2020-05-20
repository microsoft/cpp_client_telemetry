#include "objc_begin.h"

NS_ASSUME_NONNULL_BEGIN

/*!
 @brief The ODWSemanticContext class manages the inclusion of semantic context values on logged events
*/
@interface ODWSemanticContext : NSObject

/*!
 @brief Specfies an application id to be included with every event
 @param appId A string that contains an application identifier
 */
-(void)setAppId:(NSString *)appId;

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
