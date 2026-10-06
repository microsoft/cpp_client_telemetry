//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "common/Common.hpp"
#include "LogConfiguration.hpp"

#include <limits>

using namespace MAT;

#ifdef HAVE_MAT_JSONHPP
TEST(LogConfigurationTests, NullAndNonObjectRootsReturnEmpty)
{
    const char* inputs[] = {nullptr, "null", "1", "true", "\"text\"", "[]", "[{}]"};
    for (const auto input : inputs)
    {
        auto config = FromJSON(input);
        EXPECT_TRUE((*config).empty());
    }
}

TEST(LogConfigurationTests, MalformedSyntaxRespectsExceptionMode)
{
    for (const char* input : {"", "[", "not-json"})
    {
#if HAVE_EXCEPTIONS
        EXPECT_ANY_THROW(FromJSON(input));
#else
        auto config = FromJSON(input);
        EXPECT_TRUE((*config).empty());
#endif
    }
}

TEST(LogConfigurationTests, PreservesNestedValuesAndNumericLimits)
{
    auto config = FromJSON(R"({"nested":{"key":"value"},"signed":-9223372036854775808,"unsigned":18446744073709551615,"enabled":true})");
    EXPECT_TRUE(config.HasConfig("nested"));
    EXPECT_EQ(static_cast<int64_t>(config["signed"]), std::numeric_limits<int64_t>::min());
    EXPECT_EQ(static_cast<uint64_t>(config["unsigned"]), std::numeric_limits<uint64_t>::max());
    EXPECT_TRUE(static_cast<bool>(config["enabled"]));
}
#endif
