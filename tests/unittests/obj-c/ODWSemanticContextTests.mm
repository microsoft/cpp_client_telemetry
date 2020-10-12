//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
// ODWSemanticContextTests.mm
// Tests
//
// Created by David Brown on 7/29/20.
//

#import <XCTest/XCTest.h>
#import "ODWSemanticContext.h"
#import "ODWSemanticContext_private.h"
#include "ISemanticContext.hpp"

using namespace Microsoft::Applications::Events;

class TestSemanticContext : public ISemanticContext
{
public:
    virtual void SetAppId(const std::string& appId) override { m_appId = appId; }
    virtual void SetUserId(const std::string& userId, PiiKind) override { m_userId = userId; }
    virtual void SetUserAdvertisingId(const std::string& userAdvertisingId) override { m_userAdvertisingId = userAdvertisingId; }
    virtual void SetAppExperimentETag(const std::string& eTag) override { m_eTag = eTag; }

    std::string m_appId {};
    std::string m_userId {};
    std::string m_userAdvertisingId {};
    std::string m_eTag {};
};

@interface ODWSemanticContextTests : XCTestCase

@end

@implementation ODWSemanticContextTests

- (void)testSetAppId {
    TestSemanticContext nativeContext;
    ODWSemanticContext* context = [[ODWSemanticContext alloc] initWithISemanticContext:&nativeContext];
    [context setAppId:@"myAppId"];
    XCTAssertEqual(nativeContext.m_appId, "myAppId");
}

- (void)testSetUserId {
    TestSemanticContext nativeContext;
    ODWSemanticContext* context = [[ODWSemanticContext alloc] initWithISemanticContext:&nativeContext];
    [context setUserId:@"myUserId"];
    XCTAssertEqual(nativeContext.m_userId, "myUserId");
}

- (void)testSetUserAdvertisingId {
    TestSemanticContext nativeContext;
    ODWSemanticContext* context = [[ODWSemanticContext alloc] initWithISemanticContext:&nativeContext];
    [context setUserAdvertisingId:@"myUserAdvertisingId"];
    XCTAssertEqual(nativeContext.m_userAdvertisingId, "myUserAdvertisingId");
}

- (void)testSetETag {
    TestSemanticContext nativeContext;
    ODWSemanticContext* context = [[ODWSemanticContext alloc] initWithISemanticContext:&nativeContext];
    [context setAppExperimentETag:@"myETag"];
    XCTAssertEqual(nativeContext.m_eTag, "myETag");
}

@end
