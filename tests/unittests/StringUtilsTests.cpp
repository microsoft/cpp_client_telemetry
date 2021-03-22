//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "utils/StringConversion.hpp"
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

TEST(StringUtilsTests, ToString)
{
    EXPECT_THAT(toString(true), Eq("true"));
    EXPECT_THAT(toString(false), Eq("false"));

    EXPECT_THAT(toString('A'), Eq("65"));
    EXPECT_THAT(toString(static_cast<char>(-128)), Eq("-128"));
    EXPECT_THAT(toString(static_cast<char>(127)), Eq("127"));

    EXPECT_THAT(toString(-12345), Eq("-12345"));
    EXPECT_THAT(toString(12345), Eq("12345"));

    EXPECT_THAT(toString(-1234567l), Eq("-1234567"));
    EXPECT_THAT(toString(1234567l), Eq("1234567"));

    EXPECT_THAT(toString(-12345678901ll), Eq("-12345678901"));
    EXPECT_THAT(toString(12345678901ll), Eq("12345678901"));

    EXPECT_THAT(toString(static_cast<unsigned char>(255)), Eq("255"));

    EXPECT_THAT(toString(12345u), Eq("12345"));

    EXPECT_THAT(toString(1234567ul), Eq("1234567"));

    EXPECT_THAT(toString(12345678901ull), Eq("12345678901"));

    EXPECT_THAT(toString(1234.5f), Eq("1234.500000"));
    EXPECT_THAT(toString(1234.567), Eq("1234.567000"));
    EXPECT_THAT(toString(1234.567891l), Eq("1234.567891"));
}

#ifdef _WIN32
TEST(StringUtilsTests, Utf8Utf16Conversion)
{
    std::vector<std::string> test_strings = {
        "Falsches Üben von Xylophonmusik quält jeden größeren Zwerg",
        "The quick brown fox jumps over the lazy dog",
        "El pingüino Wenceslao hizo kilómetros bajo exhaustiva lluvia y frío,"
        "añoraba a su querido cachorro.",
        "Le cœur déçu mais l'âme plutôt naïve, Louÿs rêva de crapaüter en canoë"
        "au delà des îles, près du mälström où brûlent les novæ.",
        "Árvíztűrő tükörfúrógép",
        "いろはにほへとちりぬるを",
        "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム",
        "В чащах юга жил бы цитрус? Да, но фальшивый экземпляр!",
        "我能吞下玻璃而不伤身体。",
        "!@#$%^&*()-=_+[]\\{}|;':\",./<>?",
    };

    for (std::string str : test_strings) {
      EXPECT_EQ(str, to_utf8_string(to_utf16_string(str)));
    }
}
#endif  // _WIN32
