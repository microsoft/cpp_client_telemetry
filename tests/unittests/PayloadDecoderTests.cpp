//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "common/Common.hpp"
#include "PayloadDecoder.hpp"

using namespace testing;
using namespace MAT;

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

// A telemetry event field can legitimately contain bytes that are not valid
// UTF-8. nlohmann::json::dump() defaults to error_handler_t::strict, which
// throws type_error.316 on such input. Because DecodeRecord/DecodeRequest run
// on the Diagnostic Data Viewer path inside the hosting process, an unhandled
// throw terminates that process (Watson crash). These tests lock in the
// error_handler_t::replace behavior: no throw, and the malformed byte is
// emitted as the U+FFFD replacement character (EF BF BD).
TEST(PayloadDecoderTests, DecodeRecord_InvalidUtf8_DoesNotThrow)
{
    CsProtocol::Record record = MakeMinimalRecord();
    // 0xFF is never a valid UTF-8 byte.
    record.name = std::string("Bad\xFFName");

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
        EXPECT_EQ(out.find('\xFF'), std::string::npos)
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
