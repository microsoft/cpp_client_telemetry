//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "CorrelationVector.hpp"

using namespace testing;
using namespace MAT;

using std::string;

void TestCorrelationVectorVersion(int version, size_t baseLength, int maxLength, const string & testMaxMinusOneValue)
{
    CorrelationVector cv;

    // test uninitialized value
    string emptyValue = cv.GetValue();
    ASSERT_EQ(emptyValue.length(), 0ul) << "Uninitialized base value length is incorrect";

    // test initialized value
    cv.Initialize(version);
    string baseValue = cv.GetValue();
    ASSERT_EQ(baseValue.length(), static_cast<size_t>(baseLength + 2)) << "Initialized base value length is incorrect";

    // test properties
    EXPECT_TRUE(cv.CanExtend());
    EXPECT_TRUE(cv.CanIncrement());

    // test extend
    EXPECT_TRUE(cv.Extend());
    string extValue = cv.GetValue();
    ASSERT_EQ(extValue, baseValue + ".0") << "Extended value is incorrect";

    // test increment
    EXPECT_TRUE(cv.Increment());
    string incValue = cv.GetValue();
    ASSERT_EQ(incValue, baseValue + ".1") << "Incremented value is incorrect";

    // test extending until the max length
    size_t curLength = cv.GetValue().length();
    size_t length = static_cast<size_t>(maxLength);
    for (size_t i = curLength; i + 2 <= length; i += 2)
    {
        EXPECT_TRUE(cv.CanExtend());
        EXPECT_TRUE(cv.Extend());
        curLength = cv.GetValue().length();
    }

    EXPECT_TRUE(!cv.CanExtend());

    // test incrementing 99 times until currentVector becomes 100
    // this works for both version 1 and version 2 since they have even base size and odd max length
    for (int i = 0; i < 99; i++)
    {
        EXPECT_TRUE(cv.CanIncrement());
        EXPECT_TRUE(cv.Increment());
    }

    EXPECT_FALSE(cv.Increment());

    // test init with a string value
    EXPECT_TRUE(cv.SetValue(testMaxMinusOneValue));
    EXPECT_TRUE(cv.IsInitialized());
    EXPECT_TRUE(cv.CanIncrement());
    EXPECT_TRUE(cv.CanExtend());

    // increment once and make sure that we can no longer increment this value (cause we reached max unsigned int)
    EXPECT_TRUE(cv.Increment());
    EXPECT_FALSE(cv.CanIncrement()) << "Allowed incrementing beyond max unsigned int.";

    // test init with an invalid value
    string savedValue = cv.GetValue();
    EXPECT_FALSE(cv.SetValue(string("01234567890123456789AB").substr(0, baseLength) + ".-1"));
    EXPECT_FALSE(cv.SetValue(string("01234567890123456789AB").substr(0, baseLength) + ".4294967296"));
    EXPECT_FALSE(cv.SetValue(string("01234567890123456789AB").substr(0, baseLength) + ".429496729699999999999999999999"));
    EXPECT_FALSE(cv.SetValue(string("01234567890123456789AB").substr(0, baseLength) + ".0."));
    EXPECT_FALSE(cv.SetValue(string("01234567890123456789AB").substr(0, baseLength) + "..0"));

    // test that even though we failed the old initialized value was not changed
    ASSERT_EQ(savedValue, cv.GetValue()) << "Value changed after failed attempts to set it.";
}

TEST(CorrelationVectorTests, IsInitialized_NotInitialized_ReturnsFalse)
{
   CorrelationVector correlationVector;
   EXPECT_FALSE(correlationVector.IsInitialized());
}

TEST(CorrelationVectorTests, Initialize_InvalidVersionNumber_ReturnsFalse)
{
   CorrelationVector correlationVector;
   EXPECT_FALSE(correlationVector.Initialize(0));
}

TEST(CorrelationVectorTests, Initialize_Version1OrTwo_ReturnsTrue)
{
   CorrelationVector versionOne;
   CorrelationVector versionTwo;
   EXPECT_TRUE(versionOne.Initialize(1));
   EXPECT_TRUE(versionTwo.Initialize(2));
}

TEST(CorrelationVectorTests, TestCorrelationVector_Version1)
{
    TestCorrelationVectorVersion(1, 16, 63, "0123456789abcdef.4294967295.4294967294");
}

TEST(CorrelationVectorTests, TestCorrelationVector_Version2)
{
   TestCorrelationVectorVersion(2, 22, 127, "01234567890123456789ab.4294967295.4294967294");
}
