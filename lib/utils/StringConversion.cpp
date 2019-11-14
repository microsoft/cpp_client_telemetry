// Copyright (c) Microsoft. All rights reserved.
#include "StringConversion.hpp"

#include <codecvt>
#include <locale>

namespace ARIASDK_NS_BEGIN
{
    /** \brief Convert UTF-8 to UTF-16
    */
    std::wstring to_utf16_string(const std::string& in)
    {
        // libc++ doesn't properly handle wstring conversions, but handles u16 correctly
        static_assert(sizeof(std::wstring::value_type) == sizeof(std::u16string::value_type),
                      "wchar_t is expected to be the same size as char16_t on Windows");
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
        std::u16string result = converter.from_bytes(in);
        return std::wstring(result.begin(), result.end());
    }

    /** \brief Convert UTF-16 to UTF-8
    */
    std::string to_utf8_string(const std::wstring& in)
    {
        // libc++ doesn't properly handle wstring conversions, but handles u16 correctly
        static_assert(sizeof(std::wstring::value_type) == sizeof(std::u16string::value_type),
                      "wchar_t is expected to be the same size as char16_t on Windows");
        std::u16string in_u16(in.begin(), in.end());
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
        return converter.to_bytes(in_u16);
    }
}
ARIASDK_NS_END
