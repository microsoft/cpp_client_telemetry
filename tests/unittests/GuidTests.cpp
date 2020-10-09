//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "utils/Utils.hpp"
#include "EventProperties.hpp"

using namespace testing;
using namespace MAT;

TEST(GuidTests, BuildWithUpperCaseAndBraces_ToString_ReturnsUpperCaseWithNoBraces)
{
    GUID_t guid("{AAF333AA-28E3-4DEF-871E-4BBAA3EABE69}");

    ASSERT_EQ("AAF333AA-28E3-4DEF-871E-4BBAA3EABE69", guid.to_string());
}

TEST(GuidTests, BuildWithUpperCaseAndNoBraces_ToString_ReturnsUpperCaseWithNoBraces)
{
    GUID_t guid("BEE391C8-72B0-464F-93C3-1B27879AD103");

    ASSERT_EQ("BEE391C8-72B0-464F-93C3-1B27879AD103", guid.to_string());
}

TEST(GuidTests, BuildWithMixedCaseAndBraces_ToString_ReturnsUpperCaseWithNoBraces)
{
    GUID_t guid("{3647d48c-9FE5-46ec-9D37-f037c7945bf0}");

    ASSERT_EQ("3647D48C-9FE5-46EC-9D37-F037C7945BF0", guid.to_string());
}

TEST(GuidTests, BuildWithMixedCaseAndNoBraces_ToString_ReturnsUpperCaseWithNoBraces)
{
    GUID_t guid("3a6c48EE-F550-4961-8222-720032635afF");

    ASSERT_EQ("3A6C48EE-F550-4961-8222-720032635AFF", guid.to_string());
}

TEST(GuidTests, Ctor_TotallyMalformedInput_CreatesEmptyGuid)
{
    GUID_t guid("This isn't even close to a GUID");
    ASSERT_EQ("00000000-0000-0000-0000-000000000000", guid.to_string());
}

TEST(GuidTests, Ctor_PartiallyMalformedInput_CreatesEmptyGuid)
{
    GUID_t guid("{2BA7ABF7-B025-4375-oops-709203141}");
    ASSERT_EQ("00000000-0000-0000-0000-000000000000", guid.to_string());
}


TEST(GuidTests, Ctor_InputIsTooLong_IgnoresExtraData)
{
    GUID_t guid("{9D016D64-372E-4DCE-9FA3-0D0772217C54CoCougs");
    ASSERT_EQ("9D016D64-372E-4DCE-9FA3-0D0772217C54", guid.to_string());
}

