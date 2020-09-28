#include "objc_begin.h"
#include "ILogger.hpp"
#import "ODWCommonDataContext.h"

NS_ASSUME_NONNULL_BEGIN

/*!
 The <b>ODWPrivacyGuard</b> class represents Privacy Guard Hook.
*/
@interface ODWPrivacyGuard : NSObject

#pragma mark Initialization methods

/*!
 @brief Initializes Privacy Guard
 @param logger Logger used for reporting concerns
 @param commonDataContextsObject Common Data Contexts
 */
+(void)initializePrivacyGuard:(ILogger *)logger withODWCommonDataContext:(ODWCommonDataContext *)commonDataContextsObject;
@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
