// Do NOT define MEMTRACK_IMPLEMENTATION - this tests linking against the precompiled shared library
#include "MemTrack/memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_compiled_lib (Linking against precompiled shared library) ===\n");

    int res = MemTrack_Init(NULL, NULL, true, false);
    TEST_ASSERT_EQ(res, 0);

    // Verify version
    int major = 0, minor = 0, micro = 0;
    res = MemTrack_Get_Version(&major, &minor, &micro);
    TEST_ASSERT_EQ(res, 0);
    TEST_ASSERT_EQ(major, 1);
    TEST_ASSERT_EQ(minor, 4);
    TEST_ASSERT_EQ(micro, 0);

    // Verify allocations and tracking
    char *msg = t_strdup("Precompiled library test message");
    TEST_ASSERT_NOT_NULL(msg);
    TEST_ASSERT_STR_EQ(msg, "Precompiled library test message");
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 1);

    t_free(msg);
    TEST_ASSERT_NULL(msg);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 0);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);

    TEST_REPORT();
}
