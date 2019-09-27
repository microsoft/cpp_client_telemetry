//
//  iOSTestAppTests.m
//  iOSTestAppTests
//
//  Created by David Brown on 9/26/19.
//  Copyright © 2019 David Brown. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

@interface iOSTestAppTests : XCTestCase

@end

@implementation iOSTestAppTests

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    NSArray *arguments = [[NSProcessInfo processInfo] arguments];
    int i = 0;
    int argc = (int)[arguments count];
    const char **argv = (const char **)calloc((unsigned int)argc + 1, sizeof(const char *));
    for (NSString *arg in arguments) {
        argv[i++] = [arg UTF8String];
    }
    
    ::testing::InitGoogleMock(&argc, (char **)argv);
    auto googletest = ::testing::UnitTest::GetInstance();
    auto foo = googletest->total_test_count();
    (void)RUN_ALL_TESTS();
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
