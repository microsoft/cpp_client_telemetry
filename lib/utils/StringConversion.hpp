//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef STRINGCONVERSION_HPP
#define STRINGCONVERSION_HPP

#include "ctmacros.hpp"
#include <string>

namespace MAT_NS_BEGIN {

std::wstring to_utf16_string(const std::string& in);

std::string to_utf8_string(const std::wstring& in);

} MAT_NS_END

#endif
