// Copyright (c) Microsoft. All rights reserved.

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
            [m_tests recordFailureWithDescription:[NSString stringWithUTF8String:test.test_case_name()] inFile:[NSString stringWithUTF8String:test.file()] atLine:test.line() expected:true];
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
