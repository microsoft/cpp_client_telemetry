#include "objc_begin.h"
#include "ILogger.hpp"
#import "ODWLogger.h"
#import "ODWEventProperties.h"

NS_ASSUME_NONNULL_BEGIN

using namespace MAT;

/*!
 The <b>ODWLogger</b> class represents an event's properties.
*/
@interface ODWLogger (Private)

/*!
 @brief Constructs an ODWLogger object, taking internal API logger pointer. This method might be only used internally by wrapper. Use ODWLogManager initForTenant instead
 */
-(instancetype)initWithILogger:(ILogger *)logger;

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
