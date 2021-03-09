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

Define specific Disable warning macros here.

Best effort should be made keep both MSVC and GCC/Clang blocks as equivalent as possible.

*/

#if defined(_MSC_VER)

#define MAT_DISABLE_WARNING_EXPRESSION_IS_ALWAYS_FALSE MAT_DISABLE_WARNING(4296)

#elif defined(__clang__) || defined(__GCC__)

#define MAT_DISABLE_WARNING_EXPRESSION_IS_ALWAYS_FALSE MAT_DISABLE_WARNING(-Wtype-limits)

#endif

#endif  // COMPILERWARNINGS_HPP