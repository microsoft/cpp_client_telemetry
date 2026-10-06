//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef LIB_WINDOWS_UTILS_HPP
#define LIB_WINDOWS_UTILS_HPP

#include "ctmacros.hpp"

#ifdef _WIN32
#include <Windows.h>
#include <cstring>
#include <type_traits>

namespace MAT_NS_BEGIN {

    template <typename FunctionPointer>
    FunctionPointer GetWindowsProcAddress(HMODULE module, LPCSTR name) noexcept
    {
        static_assert(std::is_function<typename std::remove_pointer<FunctionPointer>::type>::value,
                      "Windows procedure lookup requires a function pointer");
        const auto address = ::GetProcAddress(module, name);
        FunctionPointer function = nullptr;
        static_assert(sizeof(function) == sizeof(address), "Windows function pointers must have the same size");
        std::memcpy(&function, &address, sizeof(function));
        return function;
    }

} MAT_NS_END
#endif

#endif
