/* 
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * 
 * Custom build configuration / custom build recipe.
 */
#ifndef MAT_CONFIG_H
#define MAT_CONFIG_H
#include "config-ikeys.h"

#ifdef   CONFIG_CUSTOM_H
/* Use custom config.h build settings */
#include CONFIG_CUSTOM_H
#else
#include "config-default.h"
#endif

/* Allow the build to omit the built-in default HTTP client. When the host
 * supplies its own IHttpClient (via CFG_MODULE_HTTP_CLIENT) it can build with
 * -DMATSDK_NO_DEFAULT_HTTP_CLIENT (set by CMake when BUILD_CURL_HTTP_CLIENT=OFF)
 * to avoid compiling/linking any built-in client -- e.g. to drop libcurl and its
 * TLS backend on Linux. This undefines the macro regardless of which config
 * preset above defined it. */
#if defined(MATSDK_NO_DEFAULT_HTTP_CLIENT) && defined(HAVE_MAT_DEFAULT_HTTP_CLIENT)
#undef HAVE_MAT_DEFAULT_HTTP_CLIENT
#endif

#if !defined(MATSDK_PAL_WIN32) && !defined(MATSDK_PAL_CPP11)
#if defined(_WIN32)
#define MATSDK_PAL_WIN32
#else
#define MATSDK_PAL_CPP11
#endif
#endif

#endif /* MAT_CONFIG_H */
