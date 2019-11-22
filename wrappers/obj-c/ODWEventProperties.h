#include "objc_begin.h"

NS_ASSUME_NONNULL_BEGIN

/*!
 @enum ODWEventPriority
 @brief The <b>ODWEventPriority</b> enumeration contains a set of values that specify event priority.
 */
typedef NS_ENUM(NSInteger, ODWEventPriority)
{
    ODWEventPriorityUnspecified = -1,
    ODWEventPriorityOff = 0,
    ODWEventPriorityLow = 1,
    ODWEventPriorityNormal = 2,
    ODWEventPriorityHigh = 3,
    ODWEventPriorityImmediate = 4
};

/*!
 The <b>ODWEventProperties</b> class represents an event's properties.
*/
@interface ODWEventProperties : NSObject

/*!
@brief Event name.
*/
@property(readwrite, copy, nonatomic) NSString* name;

/*!
@brief Event priority.
*/
@property(readwrite, nonatomic) ODWEventPriority priority;

/*!
 @brief Event properties.
*/
@property(readonly, copy, nonatomic) NSDictionary<NSString *, id> * properties;

/*!
 @brief Constructs an ODWEventProperties object, taking an event name.
 @param name A string that contains the name of the event.
 @return An instance of the ODWEventProperties interface.
 */
-(instancetype)initWithName:(NSString *)name;

/*!
 @brief Constructs an ODWEventProperties object, taking an event name and properties.
 @param name A string that contains the name of the event.
 @param properties A dictionary containing property name and value pairs.
 @return An instance of the ODWEventProperties interface.
 */
-(instancetype)initWithName:(NSString *)name
     properties:(NSDictionary<NSString *,id>*)properties NS_DESIGNATED_INITIALIZER;

-(instancetype)init NS_UNAVAILABLE;

/*!
 @brief Sets a string property for an event.
 @param name A string that contains the name of the property.
 @param value A string that contains the property value.
 */
-(void)setProperty:(NSString *)name withValue:(id)value;

/*!
 @brief Sets a double property for an event.
 @param name A string that contains the name of the property.
 @param value A double that contains the property value.
 */
-(void)setProperty:(NSString*)name withDoubleValue:(double)value;

/*!
 @brief Sets an integer property for an event.
 @param name A string that contains the name of the property.
 @param value An integer that contains the property value.
 */
-(void)setProperty:(NSString*)name withInt64Value:(int64_t)value;

/*!
 @brief Sets a BOOL property for an event.
 @param name A string that contains the name of the property.
 @param value A BOOL that contains the property value.
 */
-(void)setProperty:(NSString*)name withBoolValue:(BOOL)value;

@end

NS_ASSUME_NONNULL_END
#include "objc_end.h"
