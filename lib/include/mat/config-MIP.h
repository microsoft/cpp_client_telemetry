//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once

#define EVTSDK_VERSION_PREFIX "EVT"
#if defined(HAVE_PRIVATE_MODULES)
/* #define HAVE_MAT_EXP */
#define HAVE_MAT_FIFOSTORAGE
#endif
#define HAVE_MAT_JSONHPP
#define HAVE_MAT_ZLIB
#define HAVE_MAT_LOGGING
#define HAVE_MAT_WIN_LOG
#define HAVE_MAT_EVT_TRACEID
#define HAVE_MAT_STORAGE
#if defined(_WIN32)
#define WIN_DESKTOP
#endif
#define HAVE_MAT_DEFAULT_HTTP_CLIENT
#define HAVE_CS3

