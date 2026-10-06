//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "common/Common.hpp"
#include "PayloadDecoder.hpp"
#include "utils/ZlibUtils.hpp"

#if defined(HAVE_MAT_ZLIB) && defined(HAVE_MAT_JSONHPP)
#include "bond/All.hpp"
#include "bond/generated/CsProtocol_writers.hpp"
#include <nlohmann/json.hpp>
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
