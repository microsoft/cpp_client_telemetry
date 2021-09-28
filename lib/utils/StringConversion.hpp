//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef STRINGCONVERSION_HPP
#define STRINGCONVERSION_HPP

#include "ctmacros.hpp"
#include <string>

namespace MAT_NS_BEGIN {

#ifdef _WIN32
std::wstring to_utf16_string(const std::string& in);

std::string to_utf8_string(const std::wstring& in);
#endif  // _WIN32

} MAT_NS_END

#endif
