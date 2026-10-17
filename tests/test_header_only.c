#define MEMTRACK_IMPLEMENTATION
#include "memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_header_only (Pure single-header usage) ===\n");

    int res = MemTrack_Init(NULL, NULL, true, false);
    TEST_ASSERT_EQ(res, 0);

    // Verify version
    int major = 0, minor = 0, micro = 0;
    res = MemTrack_Get_Version(&major, &minor, &micro);
    TEST_ASSERT_EQ(res, 0);
    TEST_ASSERT_EQ(major, 1);
    TEST_ASSERT_EQ(minor, 4);
    TEST_ASSERT_EQ(micro, 0);

    // Verify allocations and tracking (malloc, calloc, realloc, strdup, strndup)
    int *nums = (int *)t_malloc(10 * sizeof(int));
    TEST_ASSERT_NOT_NULL(nums);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 10 * sizeof(int));
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 1);

    for (int i = 0; i < 10; i++) {
        nums[i] = i * i;
    }

    int *zeroed = (int *)t_calloc(5, sizeof(int));
    TEST_ASSERT_NOT_NULL(zeroed);
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_EQ(zeroed[i], 0);
    }
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 15 * sizeof(int));

    char *s = t_strndup("Header-only strndup test", 11);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_STR_EQ(s, "Header-only");
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 15 * sizeof(int) + 12);

    t_free(nums);
    t_free(zeroed);
    t_free(s);
    TEST_ASSERT_NULL(nums);
    TEST_ASSERT_NULL(zeroed);
    TEST_ASSERT_NULL(s);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);

    MemTrack_Quit();
    TEST_REPORT();
}
