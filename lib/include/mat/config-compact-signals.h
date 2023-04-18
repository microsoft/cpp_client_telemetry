//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once

#define EVTSDK_VERSION_PREFIX "EVT"
#if defined(_WIN32)
#  define HAVE_MAT_SIGNALS
#endif
#define HAVE_MAT_EXP
#define HAVE_MAT_FIFOSTORAGE
#define HAVE_MAT_JSONHPP
#define HAVE_MAT_ZLIB
#define HAVE_MAT_STORAGE
#define HAVE_MAT_DEFAULT_HTTP_CLIENT
#define HAVE_CS3
