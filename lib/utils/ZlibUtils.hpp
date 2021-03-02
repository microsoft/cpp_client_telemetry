//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef LIB_ZLIB_UTILS_HPP
#define LIB_ZLIB_UTILS_HPP

#include "ctmacros.hpp"
#include <cstdint>
#include <vector>

namespace MAT_NS_BEGIN 
{
    class ZlibUtils
    {
        public:
            static bool InflateVector(const std::vector<uint8_t>& in, std::vector<uint8_t>& out, bool isGzip);
    };

} MAT_NS_END

#endif
