// Copyright (c) Microsoft. All rights reserved.
#include "StringConversion.hpp"

#include <codecvt>
#include <locale>

namespace MAT_NS_BEGIN
{
    /** \brief Convert UTF-8 to UTF-16
    */
    std::wstring to_utf16_string(const std::string& in)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
        return converter.from_bytes(in);
    }

    /** \brief Convert UTF-16 to UTF-8
    */
    std::string to_utf8_string(const std::wstring& in)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
        return converter.to_bytes(in);
    }
}
MAT_NS_END
