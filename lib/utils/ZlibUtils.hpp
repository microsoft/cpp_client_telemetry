//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef LIB_ZLIB_UTILS_HPP
#define LIB_ZLIB_UTILS_HPP

#include "ctmacros.hpp"
#include <cstdint>
#include <cstddef>
#include <vector>

namespace MAT_NS_BEGIN 
{
    class ZlibUtils
    {
        public:
            static constexpr std::size_t MAX_INFLATED_SIZE = 64u * 1024u * 1024u;
            static bool InflateVector(const std::vector<uint8_t>& in, std::vector<uint8_t>& out, bool isGzip);
    };

} MAT_NS_END

#endif
