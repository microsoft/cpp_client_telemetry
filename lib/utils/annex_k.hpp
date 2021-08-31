//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef ANNEX_K_HPP
#define ANNEX_K_HPP

#ifndef _MSC_VER

#define __STDC_WANT_LIB_EXT1__ 1

/* WARNING: this routine works only for simple arguments */
#define sscanf_s    sscanf

#define memcpy_s(dest, destsz, src, count)  (memcpy(dest, src, (destsz<=count)?destsz:count)!=nullptr?0:EINVAL)

#define strncpy_s(dest, destsz, src, count) strncpy(dest, src, (destsz<=count)?destsz:count)

#endif

#endif

