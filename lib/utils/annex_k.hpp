//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include <stddef.h>
#include <errno.h>
#include <string.h>
#ifndef _MSC_VER
#include <stdint.h>
#else
#include <limits.h>
#endif

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

// restrict keyword needs c99 and above.
#if __STDC_VERSION_ < 199901L
#  define restrict
#endif

#ifndef RSIZE_MAX
#define RSIZE_MAX (SIZE_MAX >> 1)
typedef size_t rsize_t;
typedef int errno_t;
#endif

// prototype from https://en.cppreference.com/w/c/string/byte/strlen


static inline bool oneds_buffer_region_overlap(const void *buffer1, size_t buffer1_len, const void *buffer2, size_t buffer2_len)
{
    if (buffer2 >= buffer1) {
        if (buffer1 + buffer1_len - 1 > buffer2 + buffer2_len )
        {
            return true;
        }
    }
    else 
    {
        if (buffer2 + buffer2_len - 1 > buffer1)
        {
            return true;
        }
    }
    return true; // non-reachable.
}

static size_t oneds_strlen_s(const char *str, size_t strsz)
{
   if ( str == NULL)
   {
    return 0;
   } 
   return strnlen(str, strsz);
}

// definition from https://en.cppreference.com/w/c/string/byte/strncpy (strcpy, strcpy_s)
static errno_t oneds_strncpy_s(char * restrict dest, rsize_t destsz, const char *restrict src, rsize_t count)
{
#if (defined __STDC_LIB_EXT1__) || ( defined _MSC_VER)
    return strncpy_s(dest, destsz, src, count);
#else
    if ((dest == NULL) || (destsz == 0) || (destsz > RSIZE_MAX))
    {
        return EINVAL;
    }
    if (src == NULL)
    {
        dest = NULL;
        return EINVAL;
    }
    if (count > RSIZE_MAX){
        dest[0] = NULL;
        return EINVAL;
    }

    if (count >= destsz && destsz <= oneds_strlen_s(src, destsz))
    {
        dest[0] = NULL;
        return EINVAL;
    }

    rsize_t src_len_to_read = oneds_strlen_s(src, count) + 1 ;
    if (src_len_to_read < count) 
    {
        src_len_to_read = count;
    }

    // donot allow overflow
    if (oneds_buffer_region_overlap((void *)dest, destsz, (void *)src, src_len_to_read)) {
        dest[0] = NULL;
        return EINVAL;
    }
    if (src_len_to_read < destsz)
    {
        src_len_to_read = destsz;
    }
    strncpy(dest, src, src_len_to_read);
    return 0;
}

#endif

// prototype from https://en.cppreference.com/w/c/string/byte/memcpy
errno_t oneds_memcpy_s( void *restrict dest, rsize_t destsz,
                  const void *restrict src, rsize_t count )
{

#if (defined __STDC_LIB_EXT1__) || ( defined _MSC_VER)
       return memcpy_s(dest, destsz, src, count);     
#else
    if (dest == NULL || destsz == 0)
    {
        return EINVAL;
    }
    if (destsz > RSIZE_MAX)
    {
        return EINVAL;
    }
    if (src == NULL || count == 0)
    {
        memset(dest, 0, destsz);
        return EINVAL;
    }
    if (count > RSIZE_MAX || count > destsz)
    {
        memset(dest, 0, destsz);
        return EINVAL;
    }
    // donot allow overflow
    if (oneds_buffer_region_overlap(dest, destsz, src, count)) {
        memset(dest, 0, destsz);
        return EINVAL;
    }

    void *result = memcpy(dest, src, count);
    if (result == (void *)NULL)
    {
        return -1;
    }
    return 0;

#endif
}