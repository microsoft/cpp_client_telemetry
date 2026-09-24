//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "utils/Utils.hpp"
#include "EventProperties.hpp"

#include <set>

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

TEST(GuidTests, CopyCtor_ValidInput_CopiesCorrectly)
{
    const GUID_t first { "{9D016D64-372E-4DCE-9FA3-0D0772217C54}" };
    const GUID_t second{first};
    ASSERT_EQ("9D016D64-372E-4DCE-9FA3-0D0772217C54", second.to_string());
}

TEST(GuidTests, MoveCtor_ValidInput_MovesCorrectly)
{
    GUID_t first { "9D016D64-372E-4DCE-9FA3-0D0772217C54" };
    const GUID_t second{ std::move(first) };
    ASSERT_EQ("9D016D64-372E-4DCE-9FA3-0D0772217C54", second.to_string());
}

TEST(GuidTests, CopyAssignment_ValidInput_MovesCorrectly)
{
    const GUID_t first{ "9D016D64-372E-4DCE-9FA3-0D0772217C54" };
    GUID_t second{ "BEE391C8-72B0-464F-93C3-1B27879AD103" };
    second = first;
    ASSERT_EQ("9D016D64-372E-4DCE-9FA3-0D0772217C54", second.to_string());
}

TEST(GuidTests, MoveAssignment_ValidInput_MovesCorrectly)
{
    GUID_t first{"9D016D64-372E-4DCE-9FA3-0D0772217C54"};
    GUID_t second{"BEE391C8-72B0-464F-93C3-1B27879AD103"};
    second = std::move(first);
    ASSERT_EQ("9D016D64-372E-4DCE-9FA3-0D0772217C54", second.to_string());
}
TEST(GuidTests, OperatorLess_IsStrictWeakOrdering)
{
    // a and b differ in Data1/Data2 such that a non-lexicographic chained-|| operator
    // reported BOTH a < b and b < a (antisymmetry violation).
    GUID_t a{ "00000001-0005-0000-0000-000000000000" };
    GUID_t b{ "00000002-0003-0000-0000-000000000000" };
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);

    // c and d differ ONLY in Data3; a '==' in that position made them compare equivalent.
    GUID_t c{ "00000001-0001-0001-0000-000000000000" };
    GUID_t d{ "00000001-0001-0002-0000-000000000000" };
    EXPECT_TRUE(c < d);
    EXPECT_FALSE(d < c);

    // A std::set keyed on operator< must keep four distinct GUIDs distinct.
    std::set<GUID_t> s{ a, b, c, d };
    EXPECT_EQ(static_cast<size_t>(4), s.size());
}
