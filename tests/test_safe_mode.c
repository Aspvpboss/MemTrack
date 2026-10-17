#include "memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_safe_mode (Safe allocations without tracking) ===\n");

    // Init with auto_null_pointers = true
    int res = MemTrack_Init(NULL, NULL, true, false);
    TEST_ASSERT_EQ(res, 0);

    // In safe mode, MemTrack_memory_usage stays 0 because no tracking nodes are created
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 0);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 0);

    // Test t_malloc in safe mode
    char *buf = (char *)t_malloc(128);
    TEST_ASSERT_NOT_NULL(buf);
    strcpy(buf, "Safe mode buffer");
    TEST_ASSERT_STR_EQ(buf, "Safe mode buffer");
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 0);

    // Test t_realloc in safe mode
    buf = (char *)t_realloc(buf, 256);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_STR_EQ(buf, "Safe mode buffer");

    // Test t_strdup in safe mode
    char *dup = t_strdup("Safe strdup");
    TEST_ASSERT_NOT_NULL(dup);
    TEST_ASSERT_STR_EQ(dup, "Safe strdup");

    // Test t_free in safe mode (with auto_null_pointers enabled)
    t_free(buf);
    TEST_ASSERT_NULL(buf);

    t_free(dup);
    TEST_ASSERT_NULL(dup);

    TEST_ASSERT_EQ(MemTrack_memory_usage(), 0);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 0);

    // Test MemTrack_safe_malloc and MemTrack_safe_free directly
    void *direct = MemTrack_safe_malloc(64);
    TEST_ASSERT_NOT_NULL(direct);
    MemTrack_safe_free(&direct);
    TEST_ASSERT_NULL(direct);

    // Test MemTrack_safe_strdup and MemTrack_safe_free directly
    char *direct_dup = MemTrack_safe_strdup("Direct dup");
    TEST_ASSERT_NOT_NULL(direct_dup);
    MemTrack_safe_free((void **)&direct_dup);
    TEST_ASSERT_NULL(direct_dup);

    TEST_REPORT();
}
