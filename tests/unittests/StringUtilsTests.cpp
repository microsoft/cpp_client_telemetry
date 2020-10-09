//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "utils/StringUtils.hpp"

using namespace testing;
using namespace MAT;

using std::string;
using std::vector;

TEST(StringUtilsTests, SplitString)
{
	// testing method
	// void SplitString(const std::string& s, const char separator, std::vector<std::string>& parts);

	vector<string> parts;

	// empty input gives empty output
	parts.clear();
	StringUtils::SplitString("", ' ', parts);
	ASSERT_EQ(parts.size(), 0ul);

	// string with no separator gives single entry
	parts.clear();
	StringUtils::SplitString("abcdef0123456\t\r\n", ' ', parts);
	ASSERT_EQ(parts.size(), 1ul);
	ASSERT_EQ(parts.size() > 0 ? parts[0] : "null", "abcdef0123456\t\r\n");

	// special case '\0' character gives single entry
	parts.clear();
	StringUtils::SplitString("abcdef0123456\t\r\n", '\0', parts);
	ASSERT_EQ(parts.size(), 1ul);
	ASSERT_EQ(parts.size() > 0 ? parts[0] : "null", "abcdef0123456\t\r\n");

	// string with separator gives the correct number of entries
	parts.clear();
	StringUtils::SplitString("ab:cd:ef", ':', parts);
	ASSERT_EQ(parts.size(), 3ul);
	ASSERT_EQ(parts.size() > 0 ? parts[0] : "null", "ab");
	ASSERT_EQ(parts.size() > 1 ? parts[1] : "null", "cd");
	ASSERT_EQ(parts.size() > 2 ? parts[2] : "null", "ef");

	// consequitive separators produce empty string entries
	parts.clear();
	StringUtils::SplitString("..test..", '.', parts);
	ASSERT_EQ(parts.size(), 5ul);
	ASSERT_EQ(parts.size() > 0 ? parts[0] : "null", "");
	ASSERT_EQ(parts.size() > 1 ? parts[1] : "null", "");
	ASSERT_EQ(parts.size() > 2 ? parts[2] : "null", "test");
	ASSERT_EQ(parts.size() > 3 ? parts[3] : "null", "");
	ASSERT_EQ(parts.size() > 4 ? parts[4] : "null", "");

	// if parts was not empty these values will remain in the list while the new values are being added
	parts.clear();
	parts.push_back("ab");
	parts.push_back("cd");
	StringUtils::SplitString(" 01 23 ", ' ', parts);
	ASSERT_EQ(parts.size(), 6ul);
	ASSERT_EQ(parts.size() > 0 ? parts[0] : "null", "ab");
	ASSERT_EQ(parts.size() > 1 ? parts[1] : "null", "cd");
	ASSERT_EQ(parts.size() > 2 ? parts[2] : "null", "");
	ASSERT_EQ(parts.size() > 3 ? parts[3] : "null", "01");
	ASSERT_EQ(parts.size() > 4 ? parts[4] : "null", "23");
	ASSERT_EQ(parts.size() > 5 ? parts[5] : "null", "");
}

TEST(StringUtilsTests, AreAllCharactersWhitelisted)
{
	// testing method
	// bool AreAllCharactersWhitelisted(const std::string& stringToTest, const std::string& whitelist);

	string allAsciiChars;

	for (int i = 1; i <= 255; i++)
	{
		allAsciiChars += ((char)i);
	}

	// any string is whitelisted against the full list of characters
	EXPECT_TRUE(StringUtils::AreAllCharactersWhitelisted(allAsciiChars, allAsciiChars));
	EXPECT_TRUE(StringUtils::AreAllCharactersWhitelisted("", allAsciiChars));
	EXPECT_TRUE(StringUtils::AreAllCharactersWhitelisted("a", allAsciiChars));
	EXPECT_TRUE(StringUtils::AreAllCharactersWhitelisted("aaaaaaaaaaa", allAsciiChars));
	EXPECT_TRUE(StringUtils::AreAllCharactersWhitelisted(string(10000, ' '), allAsciiChars));

	// any non-empty string is NOT whitelisted against an empty list of characters
	EXPECT_FALSE(StringUtils::AreAllCharactersWhitelisted(allAsciiChars, ""));
	EXPECT_FALSE(StringUtils::AreAllCharactersWhitelisted("a", ""));
	EXPECT_FALSE(StringUtils::AreAllCharactersWhitelisted("aaaaaaaaaaa", ""));
	EXPECT_FALSE(StringUtils::AreAllCharactersWhitelisted(string(10000, ' '), ""));

	// empty string is whitelisted against any whitelist of characters
	EXPECT_TRUE(StringUtils::AreAllCharactersWhitelisted("", allAsciiChars));
	EXPECT_TRUE(StringUtils::AreAllCharactersWhitelisted("", ""));
	EXPECT_TRUE(StringUtils::AreAllCharactersWhitelisted("", "a"));
	EXPECT_TRUE(StringUtils::AreAllCharactersWhitelisted("", "aaaaaaaaaaa"));
	EXPECT_TRUE(StringUtils::AreAllCharactersWhitelisted("", string(10000, ' ')));

	// a few more positive and negative tests
	EXPECT_TRUE(StringUtils::AreAllCharactersWhitelisted("abc123", "abcdef123456"));
	EXPECT_FALSE(StringUtils::AreAllCharactersWhitelisted("abc123", "abcdef23456"));
}
