//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef COMPILERWARNINGS_HPP
#define COMPILERWARNINGS_HPP

#if defined(_MSC_VER)

#define MAT_PUSH_WARNINGS                  __pragma(warning(push))
#define MAT_POP_WARNINGS                   __pragma(warning(pop))
#define MAT_DISABLE_WARNING(warningNumber) __pragma(warning(disable: warningNumber))

#elif defined(__clang__) || defined(__GCC__)

#define MAT_STRINGIFY_PRAGMA(args) _Pragma(#args)

#ifdef __clang__
#define MAT_STRINGIFY_PRAGMA_WITH_COMPILER(args) MAT_STRINGIFY_PRAGMA(clang args)
#else
#define MAT_STRINGIFY_PRAGMA_WITH_COMPILER(args) MAT_STRINGIFY_PRAGMA(GCC args)
#endif

#define MAT_PUSH_WARNINGS                MAT_STRINGIFY_PRAGMA_WITH_COMPILER(diagnostic push)
#define MAT_POP_WARNINGS                 MAT_STRINGIFY_PRAGMA_WITH_COMPILER(diagnostic pop)
#define MAT_DISABLE_WARNING(warningName) MAT_STRINGIFY_PRAGMA_WITH_COMPILER(diagnostic ignored #warningName)

#else

#define MAT_PUSH_WARNINGS
#define MAT_POP_WARNINGS
#define MAT_DISABLE_WARNING(warningName) 

#endif

/*

Define specific Disable warning macros here. Keep macros ordered alphabetically.

*/

#if defined(_MSC_VER)

#define MAT_DISABLE_WARNING_DEPRECATED_METHOD_CALL     MAT_DISABLE_WARNING(4996)
#define MAT_DISABLE_WARNING_EXCEPTION_EXECUTE_HANDLER  MAT_DISABLE_WARNING(6320)
#define MAT_DISABLE_WARNING_EXPRESSION_IS_ALWAYS_FALSE MAT_DISABLE_WARNING(4296)

#elif defined(__clang__) || defined(__GCC__)

#define MAT_DISABLE_WARNING_DEPRECATED_METHOD_CALL     MAT_DISABLE_WARNING(-Wdeprecated-declarations)
#define MAT_DISABLE_WARNING_EXPRESSION_IS_ALWAYS_FALSE MAT_DISABLE_WARNING(-Wtype-limits)

#endif

#endif  // COMPILERWARNINGS_HPP