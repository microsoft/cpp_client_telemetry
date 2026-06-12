#ifndef MEMSET_S_H
#define MEMSET_S_H

#include <errno.h>
#include <stddef.h>
#include <string.h>

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

#ifndef RSIZE_MAX
#define RSIZE_MAX (SIZE_MAX >> 1)
#endif

typedef size_t rsize_t;
typedef int errno_t;

/*
** The memset_s implementation is added as a secure version of the traditional
** memset function. It includes safety checks to prevent buffer overflows.
*/
static errno_t memset_s_impl(void* s, rsize_t smax, int c, rsize_t n)
{
    if (!s || smax > RSIZE_MAX)
    {
        return EINVAL;
    }
    if (n > smax)
    {
        // Set memory up to the buffer size and return an error
        memset(s, c, smax);
        return EINVAL;
    }
    // Perform the memory set operation for the requested size
    memset(s, c, n);
    return 0;
}

// Define the macro for conditional use of memset_s or memset
#ifdef USE_ONEDS_SECURE_MEM_FUNCTIONS
#define MEMSET_S(s, smax, c, n) memset_s_impl(s, smax, c, n)
#else
#define MEMSET_S(s, smax, c, n) memset(s, c, n)
#endif

#endif  // MEMSET_S_H
