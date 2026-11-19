#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static size_t allocated_size;
static unsigned allocation_calls;
static int allocation_succeeds = 1;
static char allocation;

static void *test_malloc(size_t size) {
    allocated_size = size;
    allocation_calls++;
    return allocation_succeeds ? &allocation : NULL;
}

static void *test_calloc(size_t items, size_t size) {
    allocation_calls++;
    if (size != 0 && items > (size_t)-1 / size)
        return NULL;
    allocated_size = items * size;
    return allocation_succeeds ? &allocation : NULL;
}

static void test_free(void *ptr) {
    (void)ptr;
}

#define malloc test_malloc
#define calloc test_calloc
#define free test_free
#include "../zlib/zutil.c"
#undef malloc
#undef calloc
#undef free

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "Allocation check failed at line %d\n", __LINE__); \
        return 1; \
    } \
} while (0)

int main(void) {
    voidpf result;
    size_t size_t_width = sizeof(size_t);
    size_t uint_width = sizeof(uInt);
    size_t max_size = (size_t)-1;

#ifdef ZLIB_TEST_REQUIRE_32_BIT
    CHECK(size_t_width == 4 && uint_width == 4);
#endif

    allocation_calls = 0;
    result = zcalloc(NULL, 3, 7);
    CHECK(result == &allocation && allocation_calls == 1 && allocated_size == 21);

    result = zcalloc(NULL, 0, 7);
    CHECK(result == &allocation && allocated_size == 0);
    result = zcalloc(NULL, 7, 0);
    CHECK(result == &allocation && allocated_size == 0);

    allocation_succeeds = 0;
    result = zcalloc(NULL, 3, 7);
    CHECK(result == NULL);
    allocation_succeeds = 1;

    if (size_t_width > sizeof(unsigned)) {
        result = zcalloc(NULL, UINT_MAX, 2);
        CHECK(result == &allocation && allocated_size == (size_t)UINT_MAX * 2);
        if (UINT_MAX <= max_size / UINT_MAX) {
            result = zcalloc(NULL, UINT_MAX, UINT_MAX);
            CHECK(result == &allocation &&
                  allocated_size == (size_t)UINT_MAX * UINT_MAX);
        }
    } else {
        result = zcalloc(NULL, UINT_MAX, 1);
        CHECK(result == &allocation && allocated_size == (size_t)UINT_MAX);
        allocation_calls = 0;
        result = zcalloc(NULL, UINT_MAX, 2);
        CHECK(result == NULL);
        if (uint_width > 2)
            CHECK(allocation_calls == 0);
    }

    return 0;
}
