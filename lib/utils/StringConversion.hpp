// Copyright (c) Microsoft. All rights reserved.
#ifndef STRINGCONVERSION_HPP
#define STRINGCONVERSION_HPP

#include "Version.hpp"
#include "ctmacros.hpp"
#include <string>

namespace ARIASDK_NS_BEGIN {

std::wstring to_utf16_string(const std::string& in);

std::string to_utf8_string(const std::wstring& in);

} ARIASDK_NS_END

#endif