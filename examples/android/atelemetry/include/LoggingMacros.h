/*
 * LoggingMacros.h
 *
 *  Created on: Oct 8, 2020
 *      Author: Max Golovanov
 */

#ifndef INCLUDE_LOGGINGMACROS_H_
#define INCLUDE_LOGGINGMACROS_H_

#define INFO(...)            \
    do                       \
    {                        \
        printf(__VA_ARGS__); \
        printf("\n");        \
        ALOGD(__VA_ARGS__);  \
    } while (0)

void assert_fail(const char* file, int line, const char* func, const char* expr)
{
    INFO("assertion failed at file %s, line %d, function %s:", file, line,
         func);
    INFO("%s", expr);
    abort();
}

#define ASSERT(e)                                          \
    do                                                     \
    {                                                      \
        if (!(e))                                          \
            assert_fail(__FILE__, __LINE__, __func__, #e); \
    } while (0)

// Where to print the parcel contents: aout, alog, aerr. alog doesn't seem to work.
#define PLOG ::android::aout

#endif /* INCLUDE_LOGGINGMACROS_H_ */
