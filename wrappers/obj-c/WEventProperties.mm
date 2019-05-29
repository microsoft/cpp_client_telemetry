#import "WEventProperties.h"

@implementation WEventProperties

-(id) init {
    self = [super init];
    if(self){
        _name = @"";
        _propertiesStorage = [[NSMutableDictionary<NSString*,NSObject*> alloc] init];
        NSLog(@"EventProperties initialized successfully");
    }
    return self;
}

-(id) initWithName: (nonnull NSString*) name {
    [self init];
    _name = name;
    return self;
}

-(id) initWithName: (nonnull NSString*) name 
     andProperties: (NSDictionary<NSString*,NSObject*>*) properties {
    [self initWithName: name];
    _propertiesStorage = [properties mutableCopy];
    return self;
}

-(void) setName: (nonnull NSString*) name {
    _name = name;
}

-(NSString*) getName{
    return _name;
}

-(NSMutableDictionary<NSString*, NSObject*> *) getProperties{
    return _propertiesStorage;
}

-(void) setPropertyWithName: (NSString*) name withStringValue: (NSString*) value{
    [_propertiesStorage setValue:value forKey:name];
}

-(void) setPropertyWithName: (NSString*) name withDoubleValue: (double) value{
    [_propertiesStorage setValue:@(value) forKey:name];
}

-(void) setPropertyWithName: (NSString*) name withInt64Value: (int64_t) value{
    [_propertiesStorage setValue:@(value) forKey:name];
}

-(void) setPropertyWithName: (NSString*) name withBoolValue: (BOOL) value{
    [_propertiesStorage setValue:[NSNumber numberWithBool:value] forKey:name];
}

@end
