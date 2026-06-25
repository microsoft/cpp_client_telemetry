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

#if !defined(MATSDK_PAL_WIN32) && !defined(MATSDK_PAL_CPP11)
#if defined(_WIN32)
#define MATSDK_PAL_WIN32
#else
#define MATSDK_PAL_CPP11
#endif
#endif

#endif /* MAT_CONFIG_H */
