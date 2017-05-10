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

namespace testing {


ARIASDK_LOG_DECL_COMPONENT_NS();


class OutsideSequence {
  public:
    OutsideSequence() { swap(); }
    ~OutsideSequence() { swap(); }

  protected:
    void swap() { std::swap(sequence, *testing::internal::g_gmock_implicit_sequence.pointer()); }
    testing::Sequence* sequence = nullptr;
};


MATCHER_P(BinaryHasSubstr, str, "")
{
    std::string haystack(reinterpret_cast<char const*>(arg.data()), arg.size());
    return Matches(HasSubstr(str))(haystack);
}

MATCHER_P2(Near, value, range, "")
{
    return (abs(arg - value) <= range);
}

MATCHER_P(StrAsIntGt, value, "")
{
    return std::stoi(arg) > value;
}


} // namespace testing
