#ifndef TYPENAME_HPP
#define TYPENAME_HPP
//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include <string>
#include <typeinfo>

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>

__inline static std::string demangle(const char* name) {
    int status = -4;
    std::unique_ptr<char, void(*)(void*)> res{ abi::__cxa_demangle(name, NULL, NULL, &status), std::free };
    return (status == 0) ? res.get() : name;
}

#define TYPENAME(t) demangle(typeid(t).name()).c_str()
#define HAS_RTTI 1
#else
#define demangle(name)  (name)

#if defined(_CPPRTTI) && defined(_WIN32)
#define TYPENAME(t)   typeid(t).name()
#define HAS_RTTI 1
#else
/* No RTTI support */
#define TYPENAME(t)   ""
#define HAS_RTTI 0
#endif

#endif

#endif

