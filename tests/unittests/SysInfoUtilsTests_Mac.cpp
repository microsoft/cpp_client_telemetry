//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "pal/posix/sysinfo_utils_apple.hpp"

using namespace testing;
using namespace MAT;

TEST(SysInfoUtilsTests, GetDeviceOsName_Mac_ReturnsMacOsX)
{
    ASSERT_EQ(std::string { "Mac OS X" }, GetDeviceOsName());
}

