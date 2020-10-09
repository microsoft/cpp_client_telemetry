//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "pal/PAL.hpp"
#include "EventProperty.hpp"
#include <assert.h>
#include <stdlib.h>
#include <algorithm>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include <system/ITelemetrySystem.hpp>

namespace testing {

    const char *getMATSDKLogComponent();

    extern MAT::ITelemetrySystem & getSystem();

    const int DELAY_FACTOR_FOR_SERVER = 567;

    class OutsideSequence {
    public:
        OutsideSequence() { swap(); }
        ~OutsideSequence() { swap(); }

    protected:
        void swap() { std::swap(sequence, *testing::internal::g_gmock_implicit_sequence.pointer()); }
        testing::Sequence* sequence = nullptr;
    };

    CsProtocol::Value toCsProtocolValue(const std::string& val);
    CsProtocol::Value toCsProtocolValue(bool val);
    CsProtocol::Value toCsProtocolValue(double val);
    CsProtocol::Value toCsProtocolValue(int64_t val);
    CsProtocol::Value toCsProtocolValue(uint64_t val);
    CsProtocol::Value toCsProtocolValue(MAT::EventLatency val);


    MATCHER_P(BinaryHasSubstr, str, "")
    {
        std::string haystack(reinterpret_cast<char const*>(arg.data()), arg.size());
        return Matches(HasSubstr(str))(haystack);
    }

    MATCHER_P2(Near, value, range, "")
    {
        UNREFERENCED_PARAMETER(result_listener);
        return (abs(arg - value) <= range);
    }

    MATCHER_P(StrAsIntGt, value, "")
    {
        UNREFERENCED_PARAMETER(result_listener);
        return std::stoi(arg) > value;
    }

    bool Compress(const char* source, size_t sourceLen, char** dest, size_t& destLen, bool prependSize);

    bool Expand(const char* source, size_t sourceLen, char** dest, size_t& destLen, bool sizeAtZeroIndex);

    EventProperties CreateSampleEvent(const char *name, EventPriority prio);

    std::string GetUniqueDBFileName();

#define CAPTURE_PERF_STATS(label) \
       LogMemUsage(label); \
       LogCpuUsage(label);

    void LogMemUsage(const char* label);

    void LogCpuUsage(const char* label);
    void InflateVector(std::vector<uint8_t> &in, std::vector<uint8_t> &out, bool isGzip = false);

} // namespace testing

