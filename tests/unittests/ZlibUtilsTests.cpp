// Copyright (c) Microsoft Corporation. All rights reserved.

#include "common/Common.hpp"
#include <utils/ZlibUtils.hpp>
#include "zlib.h"

using namespace testing;
using namespace MAT;

TEST(ZlibUtilsTests, RejectsDecompressionBeyondTheOutputLimit)
{
    // A small compressed input must not be able to grow an already-full output.
    std::vector<uint8_t> input = {0x03, 0x00};
    std::vector<uint8_t> output(ZlibUtils::MAX_INFLATED_SIZE + 1, 0);
    EXPECT_FALSE(ZlibUtils::InflateVector(input, output, false));
}

TEST(ZlibUtilsTests, BoundsHighlyCompressiblePayloads)
{
    std::vector<uint8_t> original(ZlibUtils::MAX_INFLATED_SIZE + 1, 'x');
    std::vector<uint8_t> compressed(compressBound(static_cast<uLong>(original.size())));
    uLongf size = static_cast<uLongf>(compressed.size());
    ASSERT_EQ(compress2(compressed.data(), &size, original.data(),
        static_cast<uLong>(original.size()), Z_BEST_SPEED), Z_OK);
    // compress2 writes a zlib header and checksum, not raw deflate.
    compressed.erase(compressed.begin(), compressed.begin() + 2);
    compressed.resize(static_cast<size_t>(size) - 6);
    std::vector<uint8_t> output;
    EXPECT_FALSE(ZlibUtils::InflateVector(compressed, output, false));
    EXPECT_LE(output.size(), ZlibUtils::MAX_INFLATED_SIZE);
}

TEST(ZlibUtilsTests, AcceptsAnExactlyFullInflatedPayload)
{
    std::vector<uint8_t> original(ZlibUtils::MAX_INFLATED_SIZE, 'x');
    std::vector<uint8_t> compressed(compressBound(static_cast<uLong>(original.size())));
    uLongf size = static_cast<uLongf>(compressed.size());
    ASSERT_EQ(compress2(compressed.data(), &size, original.data(),
        static_cast<uLong>(original.size()), Z_BEST_SPEED), Z_OK);
    compressed.erase(compressed.begin(), compressed.begin() + 2);
    compressed.resize(static_cast<size_t>(size) - 6);
    std::vector<uint8_t> output;
    EXPECT_TRUE(ZlibUtils::InflateVector(compressed, output, false));
    EXPECT_EQ(output, original);
}

TEST(ZlibUtilsTests, InflateVector)
{
    std::vector<uint8_t> uncompressed(682440, 99);
    std::vector<uint8_t> compressed = {31, 139, 8, 0, 0, 0, 0, 0, 0, 10, 237, 193, 129, 0, 0, 0, 0, 195, 32, 216, 249, 59, 220, 224, 4, 85, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 240, 12, 134, 107, 209, 56, 200, 105, 10, 0};
    std::vector<uint8_t> inflated;
    ZlibUtils::InflateVector(compressed, inflated, true);
    ASSERT_EQ(uncompressed, inflated);
}
