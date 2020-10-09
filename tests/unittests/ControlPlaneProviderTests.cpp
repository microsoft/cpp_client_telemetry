#if 0
//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "IControlPlane.hpp"

using namespace testing;
using namespace MAT;

static const size_t ConfigV0Size = sizeof(size_t) + sizeof(LPCSTR);

TEST(ControlPlaneProviderTests, ControlPlaneConfig_CtorSetsExpectedDefaults)
{
    // Note that this test will need to be updated every time the ControlPlaneConfiguration
    // structure changes
    ControlPlaneConfiguration config;

    ASSERT_EQ(ConfigV0Size, config.structSize);
    ASSERT_EQ(nullptr, config.cacheFilePathRoot);
}

TEST(ControlPlaneProviderTests, GetControlPlane_ConfigHasInvalidSize_ReturnsNullptr)
{
    ControlPlaneConfiguration config;
    config.structSize = 0;

    ASSERT_EQ(nullptr, ControlPlaneProvider::GetControlPlane(config));
}

TEST(ControlPlaneProviderTests, GetControlPlane_ConfigHasV0Size_ConfigHasNoCachePath_ReturnsNullptr)
{
    ControlPlaneConfiguration config;
    config.structSize = ConfigV0Size;

    ASSERT_EQ(nullptr, ControlPlaneProvider::GetControlPlane(config));
}

TEST(ControlPlaneProviderTests, GetControlPlane_ConfigHasV0Size_ConfigHasCachePath_ReturnsControlPlane)
{
    ControlPlaneConfiguration config;
    config.structSize = ConfigV0Size;
    config.cacheFilePathRoot = "A Path";

    ASSERT_NE(nullptr, ControlPlaneProvider::GetControlPlane(config));
}
#endif

