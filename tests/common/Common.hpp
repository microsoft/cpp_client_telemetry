// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "pal/PAL.hpp"
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

    extern const char *getAriaSdkLogComponent();

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

    AriaProtocol::Value toAriaProtocolValue(std::string val);
    AriaProtocol::Value toAriaProtocolValue(bool val);
    AriaProtocol::Value toAriaProtocolValue(double val);
    AriaProtocol::Value toAriaProtocolValue(int64_t val);
    AriaProtocol::Value toAriaProtocolValue(uint64_t val);
    AriaProtocol::Value toAriaProtocolValue(Microsoft::Applications::Events::EventLatency val);


    MATCHER_P(BinaryHasSubstr, str, "")
    {
        std::string haystack(reinterpret_cast<char const*>(arg.data()), arg.size());
        return Matches(HasSubstr(str))(haystack);
    }

#pragma warning( push )
#pragma warning(disable: 4100)
    MATCHER_P2(Near, value, range, "")
    {
        UNREFERENCED_PARAMETER(result_listener);
        return (abs(arg - value) <= range);
    }
#pragma warning( pop ) 

    MATCHER_P(StrAsIntGt, value, "")
    {
        UNREFERENCED_PARAMETER(result_listener);
        return std::stoi(arg) > value;
    }

    bool Compress(const char* source, size_t sourceLen, char** dest, size_t& destLen, bool prependSize);

    bool Expand(const char* source, size_t sourceLen, char** dest, size_t& destLen, bool sizeAtZeroIndex);

} // namespace testing
