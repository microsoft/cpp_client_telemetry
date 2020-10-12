//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#import <Foundation/Foundation.h>
#import "ODWEventProperties.h"

@implementation ODWEventProperties {
NSMutableDictionary<NSString *, id> * _properties;
NSMutableDictionary<NSString*, NSNumber*> * _piiTags;
}

@dynamic properties;
@dynamic piiTags;

-(instancetype)initWithName:(nonnull NSString *)name
{
    return [self initWithName:name properties:[[NSMutableDictionary<NSString*,id> alloc] init] piiTags:[[NSMutableDictionary<NSString*, NSNumber*> alloc] init]];
}

-(instancetype)initWithName:(nonnull NSString *)name
     properties:(NSDictionary<NSString*,id>*) properties
{
    return [self initWithName:name properties:properties piiTags:[[NSMutableDictionary<NSString*, NSNumber*> alloc] init]];
}

-(instancetype)initWithName:(nonnull NSString *)name
     properties:(NSDictionary<NSString*,id>*) properties
     piiTags:(NSDictionary<NSString*,NSNumber*>*) piiTags
{
    self = [super init];
    if (self)
    {
        _name = [name copy];
        _properties = [properties mutableCopy];
        _piiTags = [piiTags mutableCopy];
        _priority = ODWEventPriorityUnspecified;
    }
    
    return self;
}

-(NSDictionary<NSString *, id> *)properties
{
    return [_properties copy];
}

-(NSDictionary<NSString *, NSNumber *> *)piiTags
{
    return [_piiTags copy];
}

-(void)setPiiTag:(NSString*)name withPiiKind:(ODWPiiKind)piiKind
{
    [_piiTags setValue:@(piiKind) forKey:name];
}

-(void)setProperty:(NSString*)name withValue:(id)value
{
    [_properties setValue:value forKey:name];
}

-(void)setProperty:(NSString*)name withValue:(id)value withPiiKind:(ODWPiiKind)piiKind
{
    [self setProperty:name withValue:value];
    [self setPiiTag:name withPiiKind:piiKind];
}

-(void)setProperty:(NSString*)name withDoubleValue:(double)value
{
    [_properties setValue:@(value) forKey:name];
}

-(void)setProperty:(NSString*)name withDoubleValue:(double)value withPiiKind:(ODWPiiKind)piiKind
{
    [self setProperty:name withDoubleValue:value];
    [self setPiiTag:name withPiiKind:piiKind];
}

-(void)setProperty:(NSString*)name withInt64Value:(int64_t)value
{
    [_properties setValue:@(value) forKey:name];
}

-(void)setProperty:(NSString*)name withInt64Value:(int64_t)value withPiiKind:(ODWPiiKind)piiKind
{
    [self setProperty:name withInt64Value:value];
    [self setPiiTag:name withPiiKind:piiKind];
}

-(void)setProperty:(NSString*)name withUInt8Value:(uint8_t)value
{
    [_properties setValue:@(value) forKey:name];
}

-(void)setProperty:(NSString*)name withUInt8Value:(uint8_t)value withPiiKind:(ODWPiiKind)piiKind
{
    [self setProperty:name withUInt8Value:value];
    [self setPiiTag:name withPiiKind:piiKind];
}

-(void)setProperty:(NSString*)name withUInt64Value:(uint64_t)value
{
    [_properties setValue:@(value) forKey:name];
}

-(void)setProperty:(NSString*)name withUInt64Value:(uint64_t)value withPiiKind:(ODWPiiKind)piiKind
{
    [self setProperty:name withUInt64Value:value];
    [self setPiiTag:name withPiiKind:piiKind];
}

-(void)setProperty:(NSString*)name withBoolValue:(BOOL)value
{
    [_properties setValue:@(value) forKey:name];
}

-(void)setProperty:(NSString*)name withBoolValue:(BOOL)value withPiiKind:(ODWPiiKind)piiKind
{
    [self setProperty:name withBoolValue:value];
    [self setPiiTag:name withPiiKind:piiKind];
}

-(void)setProperty:(NSString*)name withUUIDValue:(NSUUID*)value
{
    [_properties setValue:value forKey:name];
}

-(void)setProperty:(NSString*)name withUUIDValue:(NSUUID*)value withPiiKind:(ODWPiiKind)piiKind
{
    [self setProperty:name withUUIDValue:value];
    [self setPiiTag:name withPiiKind:piiKind];
}

-(void)setProperty:(NSString*)name withDateValue:(NSDate*)value
{
    [_properties setValue:value forKey:name];
}

-(void)setProperty:(NSString*)name withDateValue:(NSDate*)value withPiiKind:(ODWPiiKind)piiKind
{
    [self setProperty:name withDateValue:value];
    [self setPiiTag:name withPiiKind:piiKind];
}

-(void)setPrivacyMetadata:(ODWPrivacyDataType)privTags withODWDiagLevel:(ODWDiagLevel)privLevel
{
    [self setProperty:@"EventInfo.PrivTags" withUInt64Value:privTags];
    [self setProperty:@"EventInfo.Level" withUInt8Value:privLevel];
}

@end
