//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
// ODWLogConfigurationTests.mm
// Tests
//
// Created by David Brown on 7/29/20.
//

#import <XCTest/XCTest.h>
#import "ODWLogConfiguration.h"
#include "LogManager.hpp"

using namespace Microsoft::Applications::Events;

@interface ODWLogConfigurationTests : XCTestCase

@end

@implementation ODWLogConfigurationTests

- (void)testSetMaxTeardownTime {
    [ODWLogConfiguration setMaxTeardownUploadTimeInSec:3];
    XCTAssertEqual(static_cast<uint8_t>(LogManager::GetLogConfiguration()[CFG_INT_MAX_TEARDOWN_TIME]), 3);
}

- (void)testSetEnableTrace_True {
    [ODWLogConfiguration setEnableTrace:true];
    XCTAssertEqual([ODWLogConfiguration enableTrace], true);
    XCTAssertEqual(static_cast<bool>(LogManager::GetLogConfiguration()[CFG_BOOL_ENABLE_TRACE]), true);
}

- (void)testSetEnableTrace_False {
    [ODWLogConfiguration setEnableTrace:false];
    XCTAssertEqual([ODWLogConfiguration enableTrace], false);
    XCTAssertEqual(static_cast<bool>(LogManager::GetLogConfiguration()[CFG_BOOL_ENABLE_TRACE]), false);
}

- (void)testSetSurfaceCppExceptions_True {
    [ODWLogConfiguration setSurfaceCppExceptions:true];
    XCTAssertEqual([ODWLogConfiguration surfaceCppExceptions], true);
}

- (void)testSetSurfaceCppExceptions_False {
    [ODWLogConfiguration setSurfaceCppExceptions:false];
    XCTAssertEqual([ODWLogConfiguration surfaceCppExceptions], false);
}

@end
