//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "ctmacros.hpp"

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

// restrict keyword needs c99 and above.
#if __STDC_VERSION_ < 199901L
#  define restrict
#endif

#ifndef RSIZE_MAX
#define RSIZE_MAX (SIZE_MAX >> 1)
typedef size_t rsize_t;
typedef int errno_t;
#endif

namespace MAT_NS_BEGIN
{
class BoundCheckFunctions
{
private:
static bool oneds_buffer_region_overlap(const char *buffer1, size_t buffer1_len, const char *buffer2, size_t buffer2_len)
{
    if (buffer2 >= buffer1) 
    {
        if (buffer1 + buffer1_len - 1 > buffer2 )
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
    return false; 
}

public:
// prototype - https://en.cppreference.com/w/c/string/byte/strlen
//  Returns the length of the given null-terminated byte string
//  - returns zero if str is a null pointer
//  -  returns strsz if the null character was not found in the first strsz bytes of str.

static size_t oneds_strnlen_s(const char *str, size_t strsz)
{
   if ( str == NULL)
   {
    return 0;
   } 
   return strnlen(str, strsz);
}

// prototype - https://en.cppreference.com/w/c/string/byte/strncpy (strcpy, strcpy_s)
// Copies at most count characters of the character array pointed to by src 
//(including the terminating null character, but not any of the characters that follow 
// the null character) to character array pointed to by dest.
// Handles error conditions at runtime - 
//   - src or dest is a null pointer
//   - destsz is zero or greater than RSIZE_MAX
//   - count is greater than RSIZE_MAX
//   - count is greater or equal destsz, but destsz is less or equal strnlen_s(src, count), in other words, truncation would occur
//   - overlap would occur between the source and the destination strings
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
        dest = NULL;
        return EINVAL;
    }
    if (count >= destsz && destsz <= oneds_strnlen_s(src, count))
    {
        dest = NULL;
        return EINVAL;
    }

    rsize_t src_len_to_read = oneds_strnlen_s(src, count) + 1 ;
    if (src_len_to_read < count) 
    {
        src_len_to_read = count;
    }

    // donot allow overflow
    if (oneds_buffer_region_overlap(dest, destsz, src, src_len_to_read)) {
        dest = NULL;
        return EINVAL;
    }
    if (src_len_to_read < destsz)
    {
        src_len_to_read = destsz;
    }
    strncpy(dest, src, src_len_to_read);
    return 0;
#endif
}

// prototype -  https://en.cppreference.com/w/c/string/byte/memcpy
// Copies count characters from the object pointed to by src to the 
// object pointed to by dest. Both objects are interpreted as arrays 
// of unsigned char.
// 
// Handles error conditions at runtime - 
//  - dest or src is a null pointer
// - destsz or count is greater than RSIZE_MAX
// - count is greater than destsz (buffer overflow would occur)
// - the source and the destination objects overlap
// 
// In case of error, the entire destination range [dest, dest+destsz) is zeroed out 
// (if both dest and destsz are valid))

static errno_t oneds_memcpy_s( void *restrict dest, rsize_t destsz,
                  const void *restrict src, rsize_t count )
{
#if (defined __STDC_LIB_EXT1__) || ( defined _MSC_VER)
       return memcpy_s(dest, destsz, src, count);     
#else
    if (dest == NULL)
    {
        return EINVAL;
    }
    if (destsz > RSIZE_MAX)
    {
        return EINVAL;
    }
    if (src == NULL)
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
    if (oneds_buffer_region_overlap((char *)dest, destsz, (char *)src, count)) {
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
};
}
MAT_NS_END
#endif
