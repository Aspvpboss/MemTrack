#include "memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_lifecycle ===\n");

    // Pre-init state
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 0);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 0);

    // First init should succeed
    int res = MemTrack_Init(NULL, NULL, false, false);
    TEST_ASSERT_EQ(res, 0);

    // Second init without quit should fail
    res = MemTrack_Init(NULL, NULL, false, false);
    TEST_ASSERT_EQ(res, 1);

    // Allocate memory so Quit has tracked allocations to tear down
    void *p = t_malloc(128);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 1);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 128);

    // MemTrack_Quit should free the allocation and reset init to false
    MemTrack_Quit();

    // After Quit with allocations cleaned up, Init should succeed again
    res = MemTrack_Init(NULL, NULL, true, false);
    TEST_ASSERT_EQ(res, 0);

    // Allocate and free normally
    void *p2 = t_malloc(64);
    TEST_ASSERT_NOT_NULL(p2);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 64);
    t_free(p2);
    TEST_ASSERT_NULL(p2);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 0);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 0);

    // Call MemTrack_tracking_info safely
    MemTrack_tracking_info();

    // Allocate one node so Quit can cleanly destroy mutex and set init = false
    void *p_cleanup = t_malloc(16);
    TEST_ASSERT_NOT_NULL(p_cleanup);
    MemTrack_Quit();

    TEST_REPORT();
}
