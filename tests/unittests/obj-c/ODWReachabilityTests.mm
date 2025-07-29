//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
// ODWReachabilityTests.mm
// Tests
//
// Created by Abu Sayem on 13/12/2024.
//

#import <XCTest/XCTest.h>
#import "ODWReachability.h"

#import <sys/socket.h>
#import <netinet/in.h>
#import <netinet6/in6.h>
#import <arpa/inet.h>
#import <ifaddrs.h>
#import <netdb.h>
#import <Foundation/Foundation.h>

@interface ODWReachabilityTests : XCTestCase
@end

@implementation ODWReachabilityTests

- (void)testReachabilityWithHostname
{
    NSString *hostname = @"www.microsoft.com";
    ODWReachability *reachability = [ODWReachability reachabilityWithHostname:hostname];
    
    XCTestExpectation *expectation = [self expectationWithDescription:@"Reachability check"];
    NSString *hostUrl = [NSString stringWithFormat:@"https://%@", hostname];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        XCTAssertNotNil(reachability);
        XCTAssertEqualObjects(reachability.url.absoluteString, hostUrl);
        [expectation fulfill];
    });
    [self waitForExpectationsWithTimeout:10.0 handler:nil];
}

- (void)testReachabilityWithInvalidHostname
{
    NSString *hostname = @"invalid.hostname";
    ODWReachability *reachability = [ODWReachability reachabilityWithHostname:hostname];
    
    XCTestExpectation *expectation = [self expectationWithDescription:@"Reachability check"];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        XCTAssertNil(reachability);
        [expectation fulfill];
    });
    [self waitForExpectationsWithTimeout:10.0 handler:nil];
}

- (void)testReachabilityWithAddress
{
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("8.8.8.8");
    ODWReachability *reachability = [ODWReachability reachabilityWithAddress:&address];
    
    XCTestExpectation *expectation = [self expectationWithDescription:@"Reachability check"];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        XCTAssertNotNil(reachability);
        XCTAssertEqualObjects(reachability.url.absoluteString, @"https://8.8.8.8");
        [expectation fulfill];
    });
    [self waitForExpectationsWithTimeout:10.0 handler:nil];
}

- (void)testReachabilityWithInvalidAddress
{
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("0.0.0.0");
    ODWReachability *reachability = [ODWReachability reachabilityWithAddress:&address];
    
    XCTestExpectation *expectation = [self expectationWithDescription:@"Reachability check"];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        XCTAssertNil(reachability);
        [expectation fulfill];
    });
    [self waitForExpectationsWithTimeout:10.0 handler:nil];
}

- (void)testReachabilityForInternetConnection
{
    ODWReachability *reachability = [ODWReachability reachabilityForInternetConnection];
    
    XCTestExpectation *expectation = [self expectationWithDescription:@"Reachability check"];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        XCTAssertNotNil(reachability);
        XCTAssertEqual(reachability.currentReachabilityStatus, ReachableViaWiFi);
        [expectation fulfill];
    });
    [self waitForExpectationsWithTimeout:10.0 handler:nil];
}

- (void)testReachabilityForLocalWiFi
{
    ODWReachability *reachability = [ODWReachability reachabilityForLocalWiFi];
    
    XCTestExpectation *expectation = [self expectationWithDescription:@"Reachability check"];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        XCTAssertNotNil(reachability);
        XCTAssertEqual(reachability.currentReachabilityStatus, ReachableViaWiFi);
        [expectation fulfill];
    });
    [self waitForExpectationsWithTimeout:10.0 handler:nil];
}

@end

