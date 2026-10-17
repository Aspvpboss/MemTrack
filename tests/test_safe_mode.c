#include "memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_safe_mode (Safe allocations without tracking) ===\n");

    // Init with auto_null_pointers = true
    int res = MemTrack_Init(NULL, NULL, true, false);
    TEST_ASSERT_EQ(res, 0);

    // In safe mode, MemTrack_Memory_Usage stays 0 because no tracking nodes are created
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);

    // Test t_malloc in safe mode
    char *buf = (char *)t_malloc(128);
    TEST_ASSERT_NOT_NULL(buf);
    strcpy(buf, "Safe mode buffer");
    TEST_ASSERT_STR_EQ(buf, "Safe mode buffer");
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);

    // Test t_calloc in safe mode
    int *arr = (int *)t_calloc(16, sizeof(int));
    TEST_ASSERT_NOT_NULL(arr);
    for (int i = 0; i < 16; i++) {
        TEST_ASSERT_EQ(arr[i], 0);
    }
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);

    // Test t_realloc in safe mode
    buf = (char *)t_realloc(buf, 256);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_STR_EQ(buf, "Safe mode buffer");

    // Test t_strdup in safe mode
    char *dup = t_strdup("Safe strdup");
    TEST_ASSERT_NOT_NULL(dup);
    TEST_ASSERT_STR_EQ(dup, "Safe strdup");

    // Test t_strndup in safe mode
    char *ndup = t_strndup("Safe strndup truncated", 12);
    TEST_ASSERT_NOT_NULL(ndup);
    TEST_ASSERT_STR_EQ(ndup, "Safe strndup");
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);

    // Test t_free in safe mode (with auto_null_pointers enabled)
    t_free(buf);
    TEST_ASSERT_NULL(buf);

    t_free(arr);
    TEST_ASSERT_NULL(arr);

    t_free(dup);
    TEST_ASSERT_NULL(dup);

    t_free(ndup);
    TEST_ASSERT_NULL(ndup);

    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);

    // Test MemTrack_Safe_* functions directly
    void *direct = MemTrack_Safe_malloc(64);
    TEST_ASSERT_NOT_NULL(direct);
    MemTrack_Safe_free(&direct);
    TEST_ASSERT_NULL(direct);

    int *direct_cal = (int *)MemTrack_Safe_calloc(4, sizeof(int));
    TEST_ASSERT_NOT_NULL(direct_cal);
    MemTrack_Safe_free((void **)&direct_cal);
    TEST_ASSERT_NULL(direct_cal);

    char *direct_dup = MemTrack_Safe_strdup("Direct dup");
    TEST_ASSERT_NOT_NULL(direct_dup);
    MemTrack_Safe_free((void **)&direct_dup);
    TEST_ASSERT_NULL(direct_dup);

    char *direct_ndup = MemTrack_Safe_strndup("Direct strndup", 6);
    TEST_ASSERT_NOT_NULL(direct_ndup);
    TEST_ASSERT_STR_EQ(direct_ndup, "Direct");
    MemTrack_Safe_free((void **)&direct_ndup);
    TEST_ASSERT_NULL(direct_ndup);

    MemTrack_Quit();
    TEST_REPORT();
}
