//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#import <XCTest/XCTest.h>

#include "common/Common.hpp"
#include "config/RuntimeConfig_Default.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "utils/Utils.hpp"

@interface iOSTestAppTests : XCTestCase

@end


class TestStatusLogger : public testing::EmptyTestEventListener
{
public:
    TestStatusLogger(iOSTestAppTests* tests) : m_tests(tests) {}
    virtual void OnTestStart(testing::TestInfo const& test) override
    {
        LOG_INFO("--- %s.%s", test.test_case_name(), test.name());
    }
    
    virtual void OnTestEnd(testing::TestInfo const& test) override
    {
        LOG_INFO("=== %s.%s [%s]", test.test_case_name(), test.name(), test.result()->Passed() ? "OK" : "FAILED");
        if(!test.result()->Passed())
        {
#if 0
// use something like this code after we switch to Xcode 12 and can
// delete the deprecated code in the else block below. ADO 4251996
            XCTSourceCodeLocation *location = [[[XCTSourceCodeLocation alloc] initWithFilePath:[NSString stringWithUTF8String:test.file()] lineNumber:test.line()] autorelease];
            XCTSourceCodeContext *context = [[[XCTSourceCodeContext alloc] initWithLocation:location] autorelease];
            XCTIssue *issue = [[[XCTIssue alloc] initWithType:XCTIssueTypeAssertionFailure compactDescription:[NSString stringWithUTF8String:test.test_case_name()] detailedDescription:nil sourceCodeContext:context associatedError:nil attachments:@[]] autorelease];

            [m_tests recordIssue:issue];
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            [m_tests recordFailureWithDescription:[NSString stringWithUTF8String:test.test_case_name()] inFile:[NSString stringWithUTF8String:test.file()] atLine:test.line() expected:true];
#pragma clang diagnostic pop
#endif
        }
    }
private:
    iOSTestAppTests* m_tests;
};


@implementation iOSTestAppTests

- (void)setUp {
    NSArray *args = [[NSProcessInfo processInfo] arguments];
    
    int argc = static_cast<int>([args count]);
    std::vector<char*> argVector;
    for (NSString *arg in args) {
        char value[static_cast<size_t>([arg length])];
        strcpy(value, [arg UTF8String]);
        argVector.push_back(value);
    }

    ::testing::InitGoogleMock(&argc, argVector.data());
    TestStatusLogger* logger = new TestStatusLogger(self);
    auto googletest = ::testing::UnitTest::GetInstance();
    googletest->listeners().Append(logger);

    ILogConfiguration logConfig;
    RuntimeConfig_Default runtimeConfig(logConfig);
    PAL::initialize(runtimeConfig);
}

- (void)tearDown {
    PAL::shutdown();
}

- (void)testAll {
    XCTAssert(RUN_ALL_TESTS() == 0);
}

@end

