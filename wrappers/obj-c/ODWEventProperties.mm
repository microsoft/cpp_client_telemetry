#import <Foundation/Foundation.h>
#import "ODWEventProperties.h"

@implementation ODWEventProperties {
NSMutableDictionary<NSString *, id> * _properties;
}

@dynamic properties;

-(instancetype)initWithName:(nonnull NSString *)name
{
    return [self initWithName:name properties:[[NSMutableDictionary alloc] init]];
}

-(instancetype)initWithName:(nonnull NSString *)name
     properties:(NSDictionary<NSString*,id>*) properties
{
    self = [super init];
    if (self)
    {
        _name = [name copy];
        _properties = [properties mutableCopy];
        _priority = ODWEventPriorityUnspecified;
    }
    
    return self;
}

-(NSDictionary<NSString *, id> *)properties
{
    return [_properties copy];
}

-(void)setProperty:(NSString*)name withValue:(id)value
{
    [_properties setValue:value forKey:name];
}

-(void)setProperty:(NSString*)name withDoubleValue:(double)value
{
    [_properties setValue:@(value) forKey:name];
}

-(void)setProperty:(NSString*)name withInt64Value:(int64_t)value
{
    [_properties setValue:@(value) forKey:name];
}

-(void)setProperty:(NSString*)name withBoolValue:(BOOL)value
{
    [_properties setValue:@(value) forKey:name];
}

@end
