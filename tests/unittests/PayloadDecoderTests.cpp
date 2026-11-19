//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "common/Common.hpp"
#include "PayloadDecoder.hpp"
#include "utils/ZlibUtils.hpp"
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>

#if defined(HAVE_MAT_ZLIB) && defined(HAVE_MAT_JSONHPP)
#include "bond/All.hpp"
#include "bond/generated/CsProtocol_writers.hpp"
#include <nlohmann/json.hpp>
#include <zlib.h>

namespace clienttelemetry {
    namespace data {
        namespace v3 {
            bool Compress(const char* source, size_t sourceLen, char** dest, size_t& destLen, bool prependSize);
            bool Compress(const char* source, size_t sourceLen, char** dest, size_t& destLen, bool prependSize,
                          char* (*allocate)(size_t));
            bool Expand(const char* source, size_t sourceLen, char** dest, size_t& destLen, bool sizeAtZeroIndex);
            bool Expand(const char* source, size_t sourceLen, char** dest, size_t& destLen, bool sizeAtZeroIndex,
                        char* (*allocate)(size_t));
        }
    }
}
#endif

using namespace testing;
using namespace MAT;

TEST(PayloadDecoderTests, RejectsOversizedRequestsBeforeCopying)
{
    std::vector<uint8_t> input(ZlibUtils::MAX_INFLATED_SIZE + 1, 0);
    std::string output = "previous";
    EXPECT_FALSE(exporters::DecodeRequest(input, output, false));
    EXPECT_TRUE(output.empty());
}

TEST(PayloadDecoderTests, RejectsShortMalformedInputWithoutReadingPastTheEnd)
{
    const std::vector<uint8_t> input {0xff};
    std::string output;
    EXPECT_FALSE(exporters::DecodeRequest(input, output, false));
    EXPECT_TRUE(output.empty());
}

namespace
{
    // Builds a minimally-populated Common Schema record. to_json() in the
    // PayloadDecoder unconditionally dereferences element [0] of every ext
    // vector, so all seven must contain at least one element for serialization
    // to succeed.
    CsProtocol::Record MakeMinimalRecord()
    {
        CsProtocol::Record record;
        record.ver = "3.0";
        record.name = "Test.Event";
        record.time = 0;
        record.iKey = "o:0000";
        record.baseType = "custom";
        record.extProtocol.push_back(CsProtocol::Protocol{});
        record.extUser.push_back(CsProtocol::User{});
        record.extDevice.push_back(CsProtocol::Device{});
        record.extOs.push_back(CsProtocol::Os{});
        record.extApp.push_back(CsProtocol::App{});
        record.extNet.push_back(CsProtocol::Net{});
        record.extSdk.push_back(CsProtocol::Sdk{});
        return record;
    }
}

#if defined(HAVE_MAT_ZLIB) && defined(HAVE_MAT_JSONHPP)
TEST(PayloadDecoderTests, DecodeRequest_ManyRecordsPreservesOrderAndContents)
{
    constexpr size_t count = 4096;
    std::vector<uint8_t> input;
    for (size_t i = 0; i < count; ++i)
    {
        auto record = MakeMinimalRecord();
        record.name = "Event." + std::to_string(i) + std::string(512, 'x');
        std::vector<uint8_t> encoded;
        bond_lite::CompactBinaryProtocolWriter writer(encoded);
        bond_lite::Serialize(writer, record);
        input.insert(input.end(), encoded.begin(), encoded.end());
    }

    std::string output;
    ASSERT_TRUE(exporters::DecodeRequest(input, output, false));
    const auto records = nlohmann::json::parse(output);
    ASSERT_EQ(records.size(), count);
    for (size_t i = 0; i < count; ++i)
    {
        EXPECT_EQ(records[i]["name"].get<std::string>(),
                  "Event." + std::to_string(i) + std::string(512, 'x'));
    }
}
#endif

// A telemetry event field can legitimately contain bytes that are not valid
// UTF-8. nlohmann::json::dump() defaults to error_handler_t::strict, which
// throws type_error.316 on such input. Because DecodeRecord/DecodeRequest run
// on the decode path inside the hosting process, an unhandled throw terminates
// that process. These tests lock in the error_handler_t::replace behavior: no
// throw, and the malformed byte is emitted as the U+FFFD replacement character
// (EF BF BD).
TEST(PayloadDecoderTests, DecodeRecord_InvalidUtf8_DoesNotThrow)
{
    CsProtocol::Record record = MakeMinimalRecord();
    // Build the field with an explicit 0xFF byte (never valid UTF-8). A string
    // literal escape ("...\xFF...") would rely on implementation-defined char
    // conversion and can trip -Werror constant-conversion on some toolchains.
    std::string name = "Bad";
    name.push_back(static_cast<char>(0xFF));
    name += "Name";
    record.name = name;

    std::string out;
    bool decoded = false;
    EXPECT_NO_THROW({ decoded = exporters::DecodeRecord(record, out); });

    // When the SDK is built with JSON + Zlib support the real decoder runs and
    // must have replaced the bad byte. In a stubbed build DecodeRecord returns
    // false with an empty string, in which case the no-throw guarantee above is
    // what this test protects.
    if (decoded)
    {
        EXPECT_NE(out.find("\xEF\xBF\xBD"), std::string::npos)
            << "Malformed UTF-8 should be replaced with U+FFFD";
        EXPECT_EQ(out.find(static_cast<char>(0xFF)), std::string::npos)
            << "Raw invalid byte must not survive in the output";
    }
}

TEST(PayloadDecoderTests, DecodeRecord_ValidUtf8_IsPreserved)
{
    CsProtocol::Record record = MakeMinimalRecord();
    record.name = "Valid.Event";

    std::string out;
    bool decoded = false;
    EXPECT_NO_THROW({ decoded = exporters::DecodeRecord(record, out); });

    if (decoded)
    {
        EXPECT_NE(out.find("Valid.Event"), std::string::npos);
        EXPECT_EQ(out.find("\xEF\xBF\xBD"), std::string::npos)
            << "Valid UTF-8 must not be altered";
    }
}

#if defined(HAVE_MAT_ZLIB) && defined(HAVE_MAT_JSONHPP)
TEST(PayloadDecoderTests, Compress_InvalidInputLeavesOutputUnchanged)
{
    const char source[] = "sample";
    char previousOutput = 0;
    char* output = &previousOutput;
    size_t outputSize = 32;
    EXPECT_FALSE(clienttelemetry::data::v3::Compress(nullptr, sizeof(source), &output, outputSize, false));
    EXPECT_FALSE(clienttelemetry::data::v3::Compress(source, 0, &output, outputSize, false));
    EXPECT_EQ(output, &previousOutput);
    EXPECT_EQ(outputSize, 32u);
}

TEST(PayloadDecoderTests, Compress_AllocationFailureClearsExistingOutput)
{
    const char source[] = "sample";
    for (bool prependSize : {false, true})
    {
        char previousOutput = 0;
        char* output = &previousOutput;
        size_t outputSize = 32;
        static size_t requestedSize = 0;
        requestedSize = 0;
        EXPECT_FALSE(clienttelemetry::data::v3::Compress(source, sizeof(source), &output, outputSize, prependSize,
            [](size_t size) -> char* {
                requestedSize = size;
                return nullptr;
            }));
        EXPECT_GT(requestedSize, sizeof(source));
        EXPECT_EQ(output, nullptr);
        EXPECT_EQ(outputSize, 0u);
    }
}

TEST(PayloadDecoderTests, Compress_RejectsUnrepresentableSizeBeforeAllocation)
{
    const char source[] = "sample";
    for (bool prependSize : {false, true})
    {
        char previousOutput = 0;
        char* output = &previousOutput;
        size_t outputSize = 32;
        static bool allocated = false;
        allocated = false;
        EXPECT_FALSE(clienttelemetry::data::v3::Compress(source, std::numeric_limits<size_t>::max(),
            &output, outputSize, prependSize, [](size_t) -> char* {
                allocated = true;
                return nullptr;
            }));
        EXPECT_FALSE(allocated);
        EXPECT_EQ(output, nullptr);
        EXPECT_EQ(outputSize, 0u);
    }
}

TEST(PayloadDecoderTests, Compress_RejectsSaturatedZlibBound)
{
    const auto maxZlibSize = static_cast<std::uintmax_t>(std::numeric_limits<uLong>::max());
    if (maxZlibSize <= std::numeric_limits<size_t>::max())
    {
        const char source[] = "sample";
        char* output = nullptr;
        size_t outputSize = 0;
        static bool allocated = false;
        allocated = false;
        EXPECT_FALSE(clienttelemetry::data::v3::Compress(source, static_cast<size_t>(maxZlibSize),
            &output, outputSize, false, [](size_t) -> char* {
                allocated = true;
                return nullptr;
            }));
        EXPECT_FALSE(allocated);
        EXPECT_EQ(output, nullptr);
        EXPECT_EQ(outputSize, 0u);
    }
}

TEST(PayloadDecoderTests, Compress_RejectsUnrepresentableAllocationBound)
{
    const auto maxAllocation = static_cast<std::uintmax_t>(std::numeric_limits<ptrdiff_t>::max());
    if (maxAllocation <= static_cast<std::uintmax_t>(std::numeric_limits<uLong>::max()))
    {
        const char source[] = "sample";
        char* output = nullptr;
        size_t outputSize = 0;
        static bool allocated = false;
        allocated = false;
        EXPECT_FALSE(clienttelemetry::data::v3::Compress(source, static_cast<size_t>(maxAllocation),
            &output, outputSize, false, [](size_t) -> char* {
                allocated = true;
                return nullptr;
            }));
        EXPECT_FALSE(allocated);
        EXPECT_EQ(output, nullptr);
        EXPECT_EQ(outputSize, 0u);
    }
}

TEST(PayloadDecoderTests, Compress_RejectsSizeThatCannotFitPrefix)
{
    const auto oversized = static_cast<std::uintmax_t>(std::numeric_limits<uint32_t>::max()) + 1;
    if (oversized <= std::numeric_limits<size_t>::max())
    {
        const char source[] = "sample";
        char* output = nullptr;
        size_t outputSize = 0;
        static bool allocated = false;
        allocated = false;
        EXPECT_FALSE(clienttelemetry::data::v3::Compress(source, static_cast<size_t>(oversized),
            &output, outputSize, true, [](size_t) -> char* {
                allocated = true;
                return nullptr;
            }));
        EXPECT_FALSE(allocated);
        EXPECT_EQ(output, nullptr);
        EXPECT_EQ(outputSize, 0u);

        if (oversized <= static_cast<std::uintmax_t>(std::numeric_limits<uLong>::max()) &&
            oversized < static_cast<std::uintmax_t>(std::numeric_limits<ptrdiff_t>::max()))
        {
            EXPECT_FALSE(clienttelemetry::data::v3::Compress(source, static_cast<size_t>(oversized),
                &output, outputSize, false, [](size_t) -> char* {
                    allocated = true;
                    return nullptr;
                }));
            EXPECT_TRUE(allocated);
        }
    }
}

TEST(PayloadDecoderTests, Compress_PreservesZlibOutputWithAndWithoutSizePrefix)
{
    const char source[] = "sample compression payload";
    std::vector<uint8_t> expected(compressBound(sizeof(source)));
    uLongf expectedSize = static_cast<uLongf>(expected.size());
    ASSERT_EQ(compress2(expected.data(), &expectedSize, reinterpret_cast<const Bytef*>(source),
                        sizeof(source), Z_BEST_SPEED), Z_OK);

    for (bool prependSize : {false, true})
    {
        char* output = nullptr;
        size_t outputSize = 0;
        ASSERT_TRUE(clienttelemetry::data::v3::Compress(source, sizeof(source), &output, outputSize, prependSize));
        std::unique_ptr<char[]> owned(output);
        const size_t reserved = prependSize ? sizeof(uint32_t) : 0;
        ASSERT_EQ(outputSize, expectedSize + reserved);
        EXPECT_EQ(std::memcmp(output + reserved, expected.data(), expectedSize), 0);
        if (prependSize)
        {
            uint32_t originalSize = 0;
            std::memcpy(&originalSize, output, sizeof(originalSize));
            EXPECT_EQ(originalSize, sizeof(source));

            std::vector<char> unaligned(outputSize + 1);
            std::memcpy(unaligned.data() + 1, output, outputSize);
            char* expanded = nullptr;
            size_t expandedSize = 0;
            ASSERT_TRUE(clienttelemetry::data::v3::Expand(
                unaligned.data() + 1, outputSize, &expanded, expandedSize, true));
            std::unique_ptr<char[]> expandedOwner(expanded);
            ASSERT_EQ(expandedSize, sizeof(source));
            EXPECT_EQ(std::memcmp(expanded, source, sizeof(source)), 0);

            const uint16_t endianMarker = 1;
            if (*reinterpret_cast<const unsigned char*>(&endianMarker) == 1)
            {
                std::vector<char> legacy(outputSize + sizeof(uint32_t) + 1, 0);
                std::memcpy(legacy.data() + 1, output, sizeof(uint32_t));
                std::memcpy(legacy.data() + 1 + sizeof(uint64_t), output + sizeof(uint32_t),
                            outputSize - sizeof(uint32_t));
                expanded = nullptr;
                expandedSize = 0;
                ASSERT_TRUE(clienttelemetry::data::v3::Expand(legacy.data() + 1,
                    outputSize + sizeof(uint32_t), &expanded, expandedSize, true));
                std::unique_ptr<char[]> legacyOwner(expanded);
                ASSERT_EQ(expandedSize, sizeof(source));
                EXPECT_EQ(std::memcmp(expanded, source, sizeof(source)), 0);
            }
        }
        char restored[sizeof(source)] {};
        uLongf restoredSize = sizeof(restored);
        ASSERT_EQ(uncompress(reinterpret_cast<Bytef*>(restored), &restoredSize,
                             reinterpret_cast<const Bytef*>(output + reserved),
                             static_cast<uLong>(outputSize - reserved)), Z_OK);
        ASSERT_EQ(restoredSize, sizeof(source));
        EXPECT_EQ(std::memcmp(restored, source, sizeof(source)), 0);
    }
}

TEST(PayloadDecoderTests, Expand_RejectsShortOrIncompleteSizePrefixes)
{
    char previousOutput = 0;
    char* output = &previousOutput;
    size_t outputSize = 32;
    const char source = 0;
    EXPECT_FALSE(clienttelemetry::data::v3::Expand(&source, 0, &output, outputSize, true));
    EXPECT_EQ(output, &previousOutput);
    EXPECT_EQ(outputSize, 32u);

    for (size_t length = 1; length <= sizeof(uint64_t); ++length)
    {
        std::vector<char> prefix(length, 0);
        output = &previousOutput;
        outputSize = 32;
        EXPECT_FALSE(clienttelemetry::data::v3::Expand(prefix.data(), prefix.size(), &output, outputSize, true));
        EXPECT_EQ(output, nullptr);
        EXPECT_EQ(outputSize, 0u);

        if (length >= sizeof(uint32_t))
        {
            const uint32_t declaredSize = 1;
            std::memcpy(prefix.data(), &declaredSize, sizeof(declaredSize));
            output = &previousOutput;
            outputSize = 32;
            EXPECT_FALSE(clienttelemetry::data::v3::Expand(prefix.data(), prefix.size(), &output, outputSize, true));
            EXPECT_EQ(output, nullptr);
        }
    }
}

TEST(PayloadDecoderTests, Expand_UnrepresentableSizeClearsOutput)
{
    const char source[] = "invalid";
    char* output = nullptr;
    size_t outputSize = std::numeric_limits<size_t>::max();
    EXPECT_FALSE(clienttelemetry::data::v3::Expand(source, sizeof(source), &output, outputSize, false));
    EXPECT_EQ(output, nullptr);
    EXPECT_EQ(outputSize, 0u);
}

TEST(PayloadDecoderTests, Expand_AllocationFailureClearsExistingOutput)
{
    const char source[] = "invalid";
    char previousOutput = 0;
    char* output = &previousOutput;
    size_t outputSize = 32;
    static size_t requestedSize = 0;
    requestedSize = 0;
    EXPECT_FALSE(clienttelemetry::data::v3::Expand(source, sizeof(source), &output, outputSize, false,
        [](size_t size) -> char* {
            requestedSize = size;
            return nullptr;
        }));
    EXPECT_EQ(requestedSize, 32u);
    EXPECT_EQ(output, nullptr);
    EXPECT_EQ(outputSize, 0u);
}
#endif
