// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include <aria/Utils.hpp>

using namespace testing;


TEST(UtilsTests, ToString)
{
    using namespace Microsoft::Applications::Telemetry;

    EXPECT_THAT(toString(true),                                  Eq("true"));
    EXPECT_THAT(toString(false),                                Eq("false"));

    EXPECT_THAT(toString('A'),                                     Eq("65"));
    EXPECT_THAT(toString(static_cast<char>(-128)),               Eq("-128"));
    EXPECT_THAT(toString(static_cast<char>(127)),                 Eq("127"));

    EXPECT_THAT(toString(-12345),                              Eq("-12345"));
    EXPECT_THAT(toString(12345),                                Eq("12345"));

    EXPECT_THAT(toString(-1234567l),                         Eq("-1234567"));
    EXPECT_THAT(toString(1234567l),                           Eq("1234567"));

    EXPECT_THAT(toString(-12345678901ll),                Eq("-12345678901"));
    EXPECT_THAT(toString(12345678901ll),                  Eq("12345678901"));

    EXPECT_THAT(toString(static_cast<unsigned char>(255)),        Eq("255"));

    EXPECT_THAT(toString(12345u),                               Eq("12345"));

    EXPECT_THAT(toString(1234567ul),                          Eq("1234567"));

    EXPECT_THAT(toString(12345678901ull),                 Eq("12345678901"));

    EXPECT_THAT(toString(1234.5f),                        Eq("1234.500000"));
    EXPECT_THAT(toString(1234.567),                       Eq("1234.567000"));
    EXPECT_THAT(toString(1234.567891l),                   Eq("1234.567891"));
}
