//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "StringConversion.hpp"

#include <codecvt>
#include <locale>

#ifdef _WIN32
#include "windows.h"
#endif  // _WIN32

namespace MAT_NS_BEGIN
{

// codecvt_utf8_utf16 has been deprecated in c++17, and it has behaved
// inconsistently on different standard libraries, like libc++.
// Since this is only used on Windows, we rely on windows.h.
#ifdef _WIN32
    /** \brief Convert UTF-8 to UTF-16
    */
    std::wstring to_utf16_string(const std::string& in)
    {
        int in_length = static_cast<int>(in.size());
        int out_length = MultiByteToWideChar(CP_UTF8, 0, &in[0], in_length, NULL, 0);
        std::wstring result(out_length, '\0');
        MultiByteToWideChar(CP_UTF8, 0, &in[0], in_length, &result[0], out_length);
        return result;
    }

    /** \brief Convert UTF-16 to UTF-8
    */
    std::string to_utf8_string(const std::wstring& in)
    {
        int in_length = static_cast<int>(in.size());
        int out_length = WideCharToMultiByte(CP_UTF8, 0, &in[0], in_length, NULL, 0, NULL, NULL);
        std::string result(out_length, '\0');
        WideCharToMultiByte(CP_UTF8, 0, &in[0], in_length, &result[0], out_length, NULL, NULL);
        return result;
    }
#endif  // _WIN32
}
MAT_NS_END

