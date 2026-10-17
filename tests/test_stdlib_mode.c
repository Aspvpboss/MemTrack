#include "memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_stdlib_mode (STDLIB_ALLOCATIONS) ===\n");

    // In stdlib mode, macros map directly to standard library functions (and MemTrack_strndup)
    char *buf = (char *)t_malloc(128);
    TEST_ASSERT_NOT_NULL(buf);
    strcpy(buf, "Stdlib mode buffer");
    TEST_ASSERT_STR_EQ(buf, "Stdlib mode buffer");

    int *arr = (int *)t_calloc(10, sizeof(int));
    TEST_ASSERT_NOT_NULL(arr);
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQ(arr[i], 0);
    }

    buf = (char *)t_realloc(buf, 256);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_STR_EQ(buf, "Stdlib mode buffer");

    char *dup = t_strdup("Stdlib strdup");
    TEST_ASSERT_NOT_NULL(dup);
    TEST_ASSERT_STR_EQ(dup, "Stdlib strdup");

    char *ndup = t_strndup("Stdlib strndup extra", 14);
    TEST_ASSERT_NOT_NULL(ndup);
    TEST_ASSERT_STR_EQ(ndup, "Stdlib strndup");

    char *ndup_zero = t_strndup("Stdlib", 0);
    TEST_ASSERT_NOT_NULL(ndup_zero);
    TEST_ASSERT_STR_EQ(ndup_zero, "");

    char *ndup_null = t_strndup(NULL, 5);
    TEST_ASSERT_NULL(ndup_null);

    t_free(buf);
    t_free(arr);
    t_free(dup);
    t_free(ndup);
    t_free(ndup_zero);

    TEST_REPORT();
}
