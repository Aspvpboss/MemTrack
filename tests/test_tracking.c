#include "memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_tracking ===\n");

    int res = MemTrack_Init(NULL, NULL, true, false);
    TEST_ASSERT_EQ(res, 0);

    TEST_ASSERT_EQ(check_memory_leak(), 0);
    TEST_ASSERT_EQ(check_memory_usage(), 0);

    // Test print with 0 allocations (no crash)
    print_tracking_info();

    // Allocation 1: 100 bytes
    char *p1 = (char *)t_malloc(100);
    TEST_ASSERT_NOT_NULL(p1);
    memset(p1, 'A', 100);
    TEST_ASSERT_EQ(check_memory_leak(), 1);
    TEST_ASSERT_EQ(check_memory_usage(), 100);

    // Allocation 2: 250 bytes
    char *p2 = (char *)t_malloc(250);
    TEST_ASSERT_NOT_NULL(p2);
    memset(p2, 'B', 250);
    TEST_ASSERT_EQ(check_memory_leak(), 1);
    TEST_ASSERT_EQ(check_memory_usage(), 350);

    // Allocation 3: 500 bytes
    char *p3 = (char *)t_malloc(500);
    TEST_ASSERT_NOT_NULL(p3);
    memset(p3, 'C', 500);
    TEST_ASSERT_EQ(check_memory_leak(), 1);
    TEST_ASSERT_EQ(check_memory_usage(), 850);

    // Verify memory integrity
    TEST_ASSERT_EQ(p1[0], 'A');
    TEST_ASSERT_EQ(p1[99], 'A');
    TEST_ASSERT_EQ(p2[0], 'B');
    TEST_ASSERT_EQ(p2[249], 'B');
    TEST_ASSERT_EQ(p3[0], 'C');
    TEST_ASSERT_EQ(p3[499], 'C');

    // Print with allocations present
    print_tracking_info();

    // Free p2 (middle allocation)
    t_free(p2);
    TEST_ASSERT_NULL(p2);
    TEST_ASSERT_EQ(check_memory_leak(), 1);
    TEST_ASSERT_EQ(check_memory_usage(), 600);

    // Free p1 (head allocation)
    t_free(p1);
    TEST_ASSERT_NULL(p1);
    TEST_ASSERT_EQ(check_memory_leak(), 1);
    TEST_ASSERT_EQ(check_memory_usage(), 500);

    // Free p3 (last remaining allocation)
    t_free(p3);
    TEST_ASSERT_NULL(p3);
    TEST_ASSERT_EQ(check_memory_leak(), 0);
    TEST_ASSERT_EQ(check_memory_usage(), 0);

    // Free NULL pointer safely (should be no-op)
    void *null_ptr = NULL;
    t_free(null_ptr);
    TEST_ASSERT_EQ(check_memory_leak(), 0);
    TEST_ASSERT_EQ(check_memory_usage(), 0);

    TEST_REPORT();
}
