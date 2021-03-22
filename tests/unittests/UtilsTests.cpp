//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include <utils/Utils.hpp>
#include "CorrelationVector.hpp"

using namespace testing;
using namespace MAT;

using std::string;

TEST(UtilsTests, TestValidatePropertyName)
{
	// Valid property name could be described by the following Regex
	// ^[a-zA-Z0-9](([a-zA-Z0-9|_|.]){0,98}[a-zA-Z0-9])?$

	string abc = "abcdefghijklmnopqrstuvxyz";
	string ABC = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	string d20 = "01234567890123456789";

	// positive tests
	EXPECT_TRUE(validatePropertyName(abc + ABC));
	EXPECT_TRUE(validatePropertyName(abc + "." + d20 + "_" + ABC));
	EXPECT_TRUE(validatePropertyName(abc + "._._" + d20 + "__.." + ABC));
	EXPECT_TRUE(validatePropertyName(d20 + d20 + d20 + d20 + d20));

	// value too long
	EXPECT_FALSE(validatePropertyName(d20 + d20 + d20 + d20 + d20 + "a"));

	// invalid characters
	for (int i = 1; i < 255; i++)
	{
		char curChar = (char)i;
		string curString(1, curChar);

		if (!isalnum(i)
#if MATSDK_PAL_LEGACY
			// if MATSDK_PAL_LEGACY is defined ':' and '-' are also allowed
			// for backward compatibility.
			&& curChar != ':' && curChar != '-'
#endif
		)
		{
           // or as part of the bigger string
			if (curChar != '.' && curChar != '_')
			{
				EXPECT_FALSE(validatePropertyName(abc + ABC + curString + d20));
			}
		}
		else
		{
			// alphanumeric characters are allowed
			EXPECT_TRUE(validatePropertyName(curString));
			EXPECT_TRUE(validatePropertyName(curString + "._" + curString));

			// dots and underscores are not allowed in the begginging or in the end
			EXPECT_FALSE(validatePropertyName("." + curString));
			EXPECT_FALSE(validatePropertyName(curString + "."));
			EXPECT_TRUE(validatePropertyName("_" + curString));
			EXPECT_TRUE(validatePropertyName(curString + "_"));
		}
	}

	// Special case:
	// CorrelationVector::PropertyName is allowed
	EXPECT_TRUE(validatePropertyName(CorrelationVector::PropertyName));
}

