#ifndef ARIA_H
#define ARIA_H

#include <ctmacros.hpp>

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>

typedef enum
{
    // Basic types
    TYPE_STRING,
    TYPE_INT64,
    TYPE_DOUBLE,
    TYPE_TIME,
    TYPE_BOOLEAN,
    TYPE_GUID,
    // Arrays of basic types
    TYPE_STRING_ARRAY,
    TYPE_INT64_ARRAY,
    TYPE_DOUBLE_ARRAY,
    TYPE_TIME_ARRAY,
    TYPE_BOOL_ARRAY,
    TYPE_GUID_ARRAY,
    // NULL-type
    TYPE_NULL
} aria_prop_type;

typedef struct
{
    /// <summary>
    /// Specifies the first eight hexadecimal digits of the GUID.
    /// </summary>
    uint32_t Data1;

    /// <summary>
    /// Specifies the first group of four hexadecimal digits.
    ///</summary>
    uint16_t Data2;

    /// <summary>
    /// Specifies the second group of four hexadecimal digits.
    /// </summary>
    uint16_t Data3;

    /// <summary>
    /// An array of eight bytes.
    /// The first two bytes contain the third group of four hexadecimal digits.
    /// The remaining six bytes contain the final 12 hexadecimal digits.
    /// </summary>
    uint8_t  Data4[8];
} aria_guid_t;

typedef struct aria_event aria_event;

typedef struct
{
    char*                   name;
    aria_prop_type          type;
    union
    {
        // Basic types
        char*               as_string;
        int64_t             as_int64;
        double              as_double;
        bool                as_bool;
        aria_guid_t*        as_guid;
        uint64_t            as_time;
        // Array types are nullptr-terminated array of pointers
        char**              as_arr_string;
        int64_t**           as_arr_int64;
        bool**              as_arr_bool;
        double**            as_arr_double;
        aria_guid_t**       as_arr_guid;
        uint64_t**          as_arr_time;
    } value;
    uint32_t                piiKind;
} aria_prop;

#define ARIA_EVENT(...)    { __VA_ARGS__ , { .name = NULL, .type = TYPE_NULL } }

#define _STR(key, val)           { .name = key, .type = TYPE_STRING,  .value = { .as_string = val }, .piiKind = 0 }
#define _INT(key, val)           { .name = key, .type = TYPE_INT64,   .value = { .as_int64  = val }, .piiKind = 0 }
#define _DBL(key, val)           { .name = key, .type = TYPE_DOUBLE,  .value = { .as_double = val }, .piiKind = 0 }
#define _BOOL(key, val)          { .name = key, .type = TYPE_BOOLEAN, .value = { .as_bool   = val }, .piiKind = 0 }
#define _GUID(key, val)          { .name = key, .type = TYPE_GUID,    .value = { .as_guid   = val }, .piiKind = 0 }
#define _TIME(key, val)          { .name = key, .type = TYPE_TIME,    .value = { .as_time   = val }, .piiKind = 0 }

#define PII_STR(key,  val, kind) { .name = key, .type = TYPE_STRING,  .value = { .as_string = val }, .piiKind = kind }
#define PII_INT(key,  val, kind) { .name = key, .type = TYPE_INT64,   .value = { .as_int64  = val }, .piiKind = kind }
#define PII_DBL(key,  val, kind) { .name = key, .type = TYPE_DOUBLE,  .value = { .as_double = val }, .piiKind = kind }
#define PII_BOOL(key, val, kind) { .name = key, .type = TYPE_BOOLEAN, .value = { .as_bool   = val }, .piiKind = kind }
#define PII_GUID(key, val, kind) { .name = key, .type = TYPE_GUID,    .value = { .as_guid   = val }, .piiKind = kind }
#define PII_TIME(key, val, kind) { .name = key, .type = TYPE_TIME,    .value = { .as_time   = val }, .piiKind = kind }

ARIASDK_LIBABI bool ARIASDK_LIBABI_CDECL aria_initialize(const char* token);
ARIASDK_LIBABI void ARIASDK_LIBABI_CDECL aria_logevent(aria_prop* evt);
ARIASDK_LIBABI void ARIASDK_LIBABI_CDECL aria_teardown();
ARIASDK_LIBABI void ARIASDK_LIBABI_CDECL aria_pause();
ARIASDK_LIBABI void ARIASDK_LIBABI_CDECL aria_resume();
ARIASDK_LIBABI void ARIASDK_LIBABI_CDECL aria_upload();
ARIASDK_LIBABI void ARIASDK_LIBABI_CDECL aria_flush();

#ifdef __cplusplus
}
#endif

#endif