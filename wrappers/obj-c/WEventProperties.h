#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface WEventProperties : NSObject

@property (nonatomic, strong) NSString* name;
@property (nonatomic, strong) NSMutableDictionary<NSString*, NSObject*> * propertiesStorage;

-(id) init;

-(id) initWithName: (nonnull NSString*) name;

-(id) initWithName: (nonnull NSString*) name 
     andProperties: (NSDictionary<NSString*,NSObject*>*) properties;

-(void) setName: (nonnull NSString*) name;

-(NSString*) getName;

-(NSMutableDictionary<NSString*, NSObject*> *) getProperties;

-(void) setPropertyWithName: (NSString*) name withStringValue: (NSString*) value;
-(void) setPropertyWithName: (NSString*) name withDoubleValue: (double) value;
-(void) setPropertyWithName: (NSString*) name withInt64Value: (int64_t) value;
-(void) setPropertyWithName: (NSString*) name withBoolValue: (BOOL) value;


@end

NS_ASSUME_NONNULL_END
