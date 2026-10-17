#include "memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_tracking ===\n");

    int res = MemTrack_Init(NULL, NULL, true, false);
    TEST_ASSERT_EQ(res, 0);

    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 0);

    // Test print with 0 allocations (no crash)
    MemTrack_Tracking_Info();

    // Allocation 1: 100 bytes
    char *p1 = (char *)t_malloc(100);
    TEST_ASSERT_NOT_NULL(p1);
    memset(p1, 'A', 100);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 1);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 100);

    // Allocation 2: 250 bytes
    char *p2 = (char *)t_malloc(250);
    TEST_ASSERT_NOT_NULL(p2);
    memset(p2, 'B', 250);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 1);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 350);

    // Allocation 3: 500 bytes
    char *p3 = (char *)t_malloc(500);
    TEST_ASSERT_NOT_NULL(p3);
    memset(p3, 'C', 500);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 1);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 850);

    // Verify memory integrity
    TEST_ASSERT_EQ(p1[0], 'A');
    TEST_ASSERT_EQ(p1[99], 'A');
    TEST_ASSERT_EQ(p2[0], 'B');
    TEST_ASSERT_EQ(p2[249], 'B');
    TEST_ASSERT_EQ(p3[0], 'C');
    TEST_ASSERT_EQ(p3[499], 'C');

    // Print with allocations present
    MemTrack_Tracking_Info();

    // Free p2 (middle allocation)
    t_free(p2);
    TEST_ASSERT_NULL(p2);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 1);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 600);

    // Free p1 (head allocation)
    t_free(p1);
    TEST_ASSERT_NULL(p1);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 1);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 500);

    // Free p3 (last remaining allocation)
    t_free(p3);
    TEST_ASSERT_NULL(p3);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 0);

    // Free NULL pointer safely (should be no-op)
    void *null_ptr = NULL;
    t_free(null_ptr);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 0);

    TEST_REPORT();
}
