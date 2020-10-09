//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef TESTS_COMMON_DEBUGCONSOLE_HPP_
#define TESTS_COMMON_DEBUGCONSOLE_HPP_

#ifndef __FUNCSIG__
#define __FUNCSIG__ __FUNCTION__
#endif

#if defined(HAVE_CONSOLE_LOG) && !defined(LOG_DEBUG)
/* Log to console if there's no standard log facility defined */
#  include <cstdio>
#  ifndef LOG_DEBUG
#    define LOG_DEBUG(fmt_, ...)    fprintf(stdout, "%s: " fmt_ "\n", __FUNCSIG__ , ## __VA_ARGS__)
#    define LOG_TRACE(fmt_, ...)    fprintf(stdout, "%s: " fmt_ "\n", __FUNCSIG__ , ## __VA_ARGS__)
#    define LOG_INFO(fmt_, ...)     fprintf(stdout, "%s: " fmt_ "\n", __FUNCSIG__ , ## __VA_ARGS__)
#    define LOG_WARN(fmt_, ...)     fprintf(stderr, "%s: " fmt_ "\n", __FUNCSIG__ , ## __VA_ARGS__)
#    define LOG_ERROR(fmt_, ...)    fprintf(stderr, "%s: " fmt_ "\n", __FUNCSIG__ , ## __VA_ARGS__)
#  endif
#endif

#ifndef LOG_DEBUG
/* Don't log anything if there's no standard log facility defined */
#  define LOG_DEBUG(fmt_, ...)
#  define LOG_TRACE(fmt_, ...)
#  define LOG_INFO(fmt_, ...)
#  define LOG_WARN(fmt_, ...)
#  define LOG_ERROR(fmt_, ...)
#endif

#endif /* TESTS_COMMON_DEBUGCONSOLE_HPP_ */

