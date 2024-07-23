#include "msvc.h"
#include "memset_s.h"
#ifdef NDEBUG
/* No debug */
#include "sqlite3_retail.c"
#else
#include "sqlite3_debug.c"
#endif
