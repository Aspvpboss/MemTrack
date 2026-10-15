#define MEMTRACK_IMPLEMENTATION
#include "memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_header_only (Pure single-header usage) ===\n");

    int res = MemTrack_Init(NULL, NULL, true, false);
    TEST_ASSERT_EQ(res, 0);

    // Verify version
    int major = 0, minor = 0, micro = 0;
    res = MemTrack_GetVersion(&major, &minor, &micro);
    TEST_ASSERT_EQ(res, 0);
    TEST_ASSERT_EQ(major, 1);
    TEST_ASSERT_EQ(minor, 3);
    TEST_ASSERT_EQ(micro, 1);

    // Verify allocations and tracking
    int *nums = (int *)t_malloc(10 * sizeof(int));
    TEST_ASSERT_NOT_NULL(nums);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 10 * sizeof(int));
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 1);

    for (int i = 0; i < 10; i++) {
        nums[i] = i * i;
    }

    t_free(nums);
    TEST_ASSERT_NULL(nums);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 0);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 0);

    TEST_REPORT();
}
