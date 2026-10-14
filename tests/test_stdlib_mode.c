#include "memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_stdlib_mode (STDLIB_ALLOCATIONS) ===\n");

    // In stdlib mode, macros map directly to standard library functions
    char *buf = (char *)t_malloc(128);
    TEST_ASSERT_NOT_NULL(buf);
    strcpy(buf, "Stdlib mode buffer");
    TEST_ASSERT_STR_EQ(buf, "Stdlib mode buffer");

    buf = (char *)t_realloc(buf, 256);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_STR_EQ(buf, "Stdlib mode buffer");

    char *dup = t_strdup("Stdlib strdup");
    TEST_ASSERT_NOT_NULL(dup);
    TEST_ASSERT_STR_EQ(dup, "Stdlib strdup");

    t_free(buf);
    t_free(dup);

    TEST_REPORT();
}
