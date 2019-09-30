//
//  iOSTestAppTests.m
//  iOSTestAppTests
//
//  Created by David Brown on 9/26/19.
//  Copyright © 2019 David Brown. All rights reserved.
//

#import <XCTest/XCTest.h>

#include "common/Common.hpp"
#include "config/RuntimeConfig_Default.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "utils/Utils.hpp"

@interface iOSTestAppTests : XCTestCase

@end

@implementation iOSTestAppTests

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testAll {
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
    ILogConfiguration logConfig;
    RuntimeConfig_Default runtimeConfig(logConfig);
    PAL::initialize(runtimeConfig);
    (void)RUN_ALL_TESTS();
    PAL::shutdown();
}

@end
