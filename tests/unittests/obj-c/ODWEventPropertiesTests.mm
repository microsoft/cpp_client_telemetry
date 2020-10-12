//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
// ODWEventPropertiesTests.mm
// Tests
//
// Created by David Brown on 7/28/20.
//

#import <XCTest/XCTest.h>
#import "ODWEventProperties.h"

@interface ODWLogManagerTests : XCTestCase

@end

@implementation ODWLogManagerTests

- (void)testInitWithName {
    ODWEventProperties* eventProps = [[ODWEventProperties alloc] initWithName:@"MyEvent"];
    XCTAssertEqual(eventProps.name, @"MyEvent");
}

- (void)testInitWithName_Properties {
    ODWEventProperties* eventProps = [[ODWEventProperties alloc] initWithName:@"MyEventWithProperties" properties:@{@"myString":@"hello", @"myInt":@5}];
    XCTAssertEqual(eventProps.name, @"MyEventWithProperties");
    XCTAssertEqual(eventProps.properties.count, 2);
    XCTAssertEqual(eventProps.properties[@"myString"], @"hello");
    XCTAssertEqual(eventProps.properties[@"myInt"], @5);
}

- (void)testInitWithName_PropertiesAndPii {
    ODWEventProperties* eventProps = [[ODWEventProperties alloc] initWithName:@"MyEventWithProperties" properties:@{@"myString":@"hello", @"myInt":@5} piiTags:@{@"myTag":@(ODWPiiKindFqdn)}];
    XCTAssertEqual(eventProps.name, @"MyEventWithProperties");
    XCTAssertEqual(eventProps.properties.count, 2);
    XCTAssertEqual(eventProps.properties[@"myString"], @"hello");
    XCTAssertEqual(eventProps.properties[@"myInt"], @5);
    XCTAssertEqual(eventProps.piiTags.count, 1);
    XCTAssertEqual(eventProps.piiTags[@"myTag"], @(ODWPiiKindFqdn));
}

- (void)testSetProperty_String {
    ODWEventProperties* props = [[ODWEventProperties alloc] initWithName:@"myEvent"];
    [props setProperty:@"myString" withValue:@"someString"];
    XCTAssertEqual(props.properties.count, 1);
    XCTAssertEqual(props.properties[@"myString"], @"someString");
}

- (void)testSetProperty_PiiString {
    ODWEventProperties* props = [[ODWEventProperties alloc] initWithName:@"myEvent"];
    [props setProperty:@"myString" withValue:@"someString" withPiiKind:ODWPiiKindUri];
    XCTAssertEqual(props.properties.count, 1);
    XCTAssertEqual(props.properties[@"myString"], @"someString");
    XCTAssertEqual(props.piiTags.count, 1);
    XCTAssertEqual(props.piiTags[@"myString"], @(ODWPiiKindUri));
}

- (void)testSetProperty_Double {
    ODWEventProperties* props = [[ODWEventProperties alloc] initWithName:@"myEvent"];
    [props setProperty:@"myDouble" withDoubleValue:2.5];
    XCTAssertEqual(props.properties.count, 1);
    XCTAssertEqual([props.properties[@"myDouble"] floatValue], 2.5);
}

- (void)testSetProperty_PiiDouble {
    ODWEventProperties* props = [[ODWEventProperties alloc] initWithName:@"myEvent"];
    [props setProperty:@"myDouble" withDoubleValue:2.5 withPiiKind:ODWPiiKindIdentity];
    XCTAssertEqual(props.properties.count, 1);
    XCTAssertEqual([props.properties[@"myDouble"] floatValue], 2.5);
    XCTAssertEqual(props.piiTags.count, 1);
    XCTAssertEqual(props.piiTags[@"myDouble"], @(ODWPiiKindIdentity));
}

- (void)testSetProperty_Int64 {
    ODWEventProperties* props = [[ODWEventProperties alloc] initWithName:@"myEvent"];
    [props setProperty:@"myInt64" withInt64Value:-123456789];
    XCTAssertEqual(props.properties.count, 1);
    XCTAssertEqual([props.properties[@"myInt64"] longLongValue], -123456789);
}

- (void)testSetProperty_PiiInt64 {
    ODWEventProperties* props = [[ODWEventProperties alloc] initWithName:@"myEvent"];
    [props setProperty:@"myInt64" withInt64Value:-123456789 withPiiKind:ODWPiiKindSipAddress];
    XCTAssertEqual(props.properties.count, 1);
    XCTAssertEqual([props.properties[@"myInt64"] longLongValue], -123456789);
    XCTAssertEqual(props.piiTags.count, 1);
    XCTAssertEqual(props.piiTags[@"myInt64"], @(ODWPiiKindSipAddress));
}

- (void)testSetProperty_UInt8 {
    ODWEventProperties* props = [[ODWEventProperties alloc] initWithName:@"myEvent"];
    [props setProperty:@"myUInt8" withInt64Value:255];
    XCTAssertEqual(props.properties.count, 1);
    XCTAssertEqual([props.properties[@"myUInt8"] integerValue], 255);
}

- (void)testSetProperty_PiiUInt8 {
    ODWEventProperties* props = [[ODWEventProperties alloc] initWithName:@"myEvent"];
    [props setProperty:@"myUInt8" withInt64Value:255 withPiiKind:ODWPiiKindIPv4Address];
    XCTAssertEqual(props.properties.count, 1);
    XCTAssertEqual([props.properties[@"myUInt8"] integerValue], 255);
    XCTAssertEqual(props.piiTags.count, 1);
    XCTAssertEqual(props.piiTags[@"myUInt8"], @(ODWPiiKindIPv4Address));
}

- (void)testSetProperty_UInt64 {
    ODWEventProperties* props = [[ODWEventProperties alloc] initWithName:@"myEvent"];
    [props setProperty:@"myUInt64" withInt64Value:123456789];
    XCTAssertEqual(props.properties.count, 1);
    XCTAssertEqual([props.properties[@"myUInt64"] longLongValue], 123456789);
}

- (void)testSetProperty_PiiUInt64 {
    ODWEventProperties* props = [[ODWEventProperties alloc] initWithName:@"myEvent"];
    [props setProperty:@"myUInt64" withInt64Value:123456789 withPiiKind:ODWPiiKindIPv6Address];
    XCTAssertEqual(props.properties.count, 1);
    XCTAssertEqual([props.properties[@"myUInt64"] longLongValue], 123456789);
    XCTAssertEqual(props.piiTags.count, 1);
    XCTAssertEqual(props.piiTags[@"myUInt64"], @(ODWPiiKindIPv6Address));
}

- (void)testSetProperty_Bool {
    ODWEventProperties* props = [[ODWEventProperties alloc] initWithName:@"myEvent"];
    [props setProperty:@"myBool" withBoolValue:true];
    XCTAssertEqual(props.properties.count, 1);
    XCTAssertEqual([props.properties[@"myBool"] boolValue], true);
}

- (void)testSetProperty_PiiBool {
    ODWEventProperties* props = [[ODWEventProperties alloc] initWithName:@"myEvent"];
    [props setProperty:@"myBool" withBoolValue:true withPiiKind:ODWPiiKindGenericData];
    XCTAssertEqual(props.properties.count, 1);
    XCTAssertEqual([props.properties[@"myBool"] boolValue], true);
    XCTAssertEqual(props.piiTags.count, 1);
    XCTAssertEqual(props.piiTags[@"myBool"], @(ODWPiiKindGenericData));
}

- (void)testSetProperty_UUID {
    ODWEventProperties* props = [[ODWEventProperties alloc] initWithName:@"myEvent"];
    [props setProperty:@"myUUID" withUUIDValue:[[NSUUID alloc] initWithUUIDString:@"DEADBEEF-1234-2345-3456-123456789ABC"]];
    XCTAssertEqual(props.properties.count, 1);
    XCTAssert([[(NSUUID*)props.properties[@"myUUID"] UUIDString] isEqualToString:@"DEADBEEF-1234-2345-3456-123456789ABC"]);
}

- (void)testSetProperty_PiiUUID{
    ODWEventProperties* props = [[ODWEventProperties alloc] initWithName:@"myEvent"];
    [props setProperty:@"myUUID" withUUIDValue:[[NSUUID alloc] initWithUUIDString:@"DEADBEEF-1234-2345-3456-123456789ABC"] withPiiKind:ODWPiiKindIdentity];
    XCTAssertEqual(props.properties.count, 1);
    XCTAssert([[(NSUUID*)props.properties[@"myUUID"] UUIDString] isEqualToString:@"DEADBEEF-1234-2345-3456-123456789ABC"]);
    XCTAssertEqual(props.piiTags.count, 1);
    XCTAssertEqual(props.piiTags[@"myUUID"], @(ODWPiiKindIdentity));
}

- (void)testSetProperty_Date {
    ODWEventProperties* props = [[ODWEventProperties alloc] initWithName:@"myEvent"];
    [props setProperty:@"myDate" withDateValue:[[NSDate alloc] initWithTimeIntervalSince1970:1234]];
    XCTAssertEqual(props.properties.count, 1);
    XCTAssertEqual([(NSDate*)props.properties[@"myDate"] timeIntervalSince1970], 1234);
}

- (void)testSetProperty_PiiDate {
    ODWEventProperties* props = [[ODWEventProperties alloc] initWithName:@"myEvent"];
    [props setProperty:@"myDate" withDateValue:[[NSDate alloc] initWithTimeIntervalSince1970:1234] withPiiKind:ODWPiiKindQueryString];
    XCTAssertEqual(props.properties.count, 1);
    XCTAssertEqual([(NSDate*)props.properties[@"myDate"] timeIntervalSince1970], 1234);
    XCTAssertEqual(props.piiTags.count, 1);
    XCTAssertEqual(props.piiTags[@"myDate"], @(ODWPiiKindQueryString));
}

@end
