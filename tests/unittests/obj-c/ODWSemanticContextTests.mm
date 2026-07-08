//
// Copyright (c) Microsoft Corporation. All rights reserved.
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
    virtual void SetOsVersion(const std::string& osVersion) override { m_osVersion = osVersion; }
    virtual void SetOsBuild(const std::string& osBuild) override { m_osBuild = osBuild; }

    std::string m_appId {};
    std::string m_userId {};
    std::string m_userAdvertisingId {};
    std::string m_eTag {};
    std::string m_osVersion {};
    std::string m_osBuild {};
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

- (void)testSetOsVersion {
    TestSemanticContext nativeContext;
    ODWSemanticContext* context = [[ODWSemanticContext alloc] initWithISemanticContext:&nativeContext];
    [context setOsVersion:@"10.0.19041"];
    XCTAssertEqual(nativeContext.m_osVersion, "10.0.19041");
}

- (void)testSetOsBuild {
    TestSemanticContext nativeContext;
    ODWSemanticContext* context = [[ODWSemanticContext alloc] initWithISemanticContext:&nativeContext];
    [context setOsBuild:@"19041.1234"];
    XCTAssertEqual(nativeContext.m_osBuild, "19041.1234");
}

@end
