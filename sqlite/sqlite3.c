#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* expose mremap()/MREMAP_MAYMOVE on glibc-Linux; harmless on Windows/Android */
#endif
#include "msvc.h"
#include "memset_s.h"
#ifdef NDEBUG
/* No debug */
#include "sqlite3_retail.c"
#else
#include "sqlite3_debug.c"
#endif
