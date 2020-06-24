// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include "pal/posix/sysinfo_utils_apple.hpp"

using namespace testing;
using namespace MAT;

TEST(SysInfoUtilsTests, GetDeviceOsName_iOS_ReturnsiOS)
{
    ASSERT_EQ(std::string { "iOS" }, GetDeviceOsName());
}
