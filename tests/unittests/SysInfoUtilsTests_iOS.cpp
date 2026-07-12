//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "pal/posix/sysinfo_utils_apple.hpp"
#include <TargetConditionals.h>

#if defined(TARGET_OS_VISION) && TARGET_OS_VISION
#define MATSDK_TEST_TARGET_OS_VISION 1
#else
#define MATSDK_TEST_TARGET_OS_VISION 0
#endif

using namespace testing;
using namespace MAT;

TEST(SysInfoUtilsTests, GetDeviceOsName_AppleMobile_ReturnsExpectedName)
{
#if MATSDK_TEST_TARGET_OS_VISION
    ASSERT_EQ(std::string { "visionOS" }, GetDeviceOsName());
#else
    ASSERT_EQ(std::string { "iOS" }, GetDeviceOsName());
#endif
}

#if MATSDK_TEST_TARGET_OS_VISION
TEST(SysInfoUtilsTests, GetDeviceClass_visionOS_ReturnsVisionClass)
{
#if defined(TARGET_OS_SIMULATOR) && TARGET_OS_SIMULATOR
    ASSERT_EQ(std::string { "visionOS.Emulator" }, GetDeviceClass());
#else
    ASSERT_EQ(std::string { "visionOS.Vision" }, GetDeviceClass());
#endif
}
#endif
