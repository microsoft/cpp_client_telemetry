/* 
 * Copyright 2019 (c) Microsoft. All rights reserved. 
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
#endif /* MAT_CONFIG_H */
