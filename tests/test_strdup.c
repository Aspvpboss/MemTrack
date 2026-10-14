#include "memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_strdup ===\n");

    int res = MemTrack_Init(NULL, NULL, true, false);
    TEST_ASSERT_EQ(res, 0);

    const char *orig = "MemTrack string duplication test";
    size_t expected_len = strlen(orig) + 1;

    // Duplicate standard string
    char *dup = t_strdup(orig);
    TEST_ASSERT_NOT_NULL(dup);
    TEST_ASSERT_NE((uintptr_t)dup, (uintptr_t)orig);
    TEST_ASSERT_STR_EQ(dup, orig);
    TEST_ASSERT_EQ(check_memory_usage(), expected_len);
    TEST_ASSERT_EQ(check_memory_leak(), 1);

    // Duplicate empty string
    char *dup_empty = t_strdup("");
    TEST_ASSERT_NOT_NULL(dup_empty);
    TEST_ASSERT_STR_EQ(dup_empty, "");
    TEST_ASSERT_EQ(check_memory_usage(), expected_len + 1);

    // Duplicate NULL string safely
    char *dup_null = t_strdup(NULL);
    TEST_ASSERT_NULL(dup_null);
    TEST_ASSERT_EQ(check_memory_usage(), expected_len + 1);

    // Free empty duplicate
    t_free(dup_empty);
    TEST_ASSERT_NULL(dup_empty);
    TEST_ASSERT_EQ(check_memory_usage(), expected_len);

    // Free standard duplicate
    t_free(dup);
    TEST_ASSERT_NULL(dup);
    TEST_ASSERT_EQ(check_memory_usage(), 0);
    TEST_ASSERT_EQ(check_memory_leak(), 0);

    TEST_REPORT();
}
