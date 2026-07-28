//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once

#define EVTSDK_VERSION_PREFIX "EVT"
#if defined(_WIN32)
#if defined __has_include
#  if __has_include ("modules/azmon/AITelemetrySystem.hpp")
#    define HAVE_MAT_AI
#  endif
#  if __has_include ("modules/utc/UtcTelemetrySystem.hpp")
#    define HAVE_MAT_UTC
#  endif
#endif
#endif
#define HAVE_MAT_EXP
#define HAVE_MAT_FIFOSTORAGE
//#define HAVE_MAT_DEFAULTDATAVIEWER
#define HAVE_MAT_JSONHPP
#define HAVE_MAT_ZLIB
#define HAVE_MAT_LOGGING
/* #define HAVE_MAT_WIN_LOG     */
/* #define HAVE_MAT_EVT_TRACEID     */
#define HAVE_MAT_STORAGE
#define HAVE_MAT_DEFAULT_HTTP_CLIENT
// The two macros below are also added on the command line by
// tests/{functests,unittests}/CMakeLists.txt when BUILD_LIVEEVENTINSPECTOR
// / BUILD_PRIVACYGUARD are ON. Guard against -Wmacro-redefined under
// -Werror on Linux/macOS.
#ifndef HAVE_MAT_LIVEEVENTINSPECTOR
#define HAVE_MAT_LIVEEVENTINSPECTOR
#endif
#ifndef HAVE_MAT_PRIVACYGUARD
#define HAVE_MAT_PRIVACYGUARD
#endif
//#define HAVE_MAT_DEFAULT_FILTER
#if defined(_WIN32) && !defined(_WINRT_DLL)
#define HAVE_MAT_NETDETECT
#endif
#define HAVE_CS3
//#define HAVE_CS4
//#define HAVE_CS4_FULL
//#define HAVE_ONEDS_BOUNDCHECK_METHODS

