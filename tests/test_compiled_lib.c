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

    // Verify allocations and tracking across shared library boundary
    char *msg = t_strdup("Precompiled library test message");
    TEST_ASSERT_NOT_NULL(msg);
    TEST_ASSERT_STR_EQ(msg, "Precompiled library test message");
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 1);

    char *short_msg = t_strndup("Precompiled library test message", 11);
    TEST_ASSERT_NOT_NULL(short_msg);
    TEST_ASSERT_STR_EQ(short_msg, "Precompiled");

    int *cal = (int *)t_calloc(8, sizeof(int));
    TEST_ASSERT_NOT_NULL(cal);
    for (int i = 0; i < 8; i++) {
        TEST_ASSERT_EQ(cal[i], 0);
    }

    t_free(msg);
    t_free(short_msg);
    t_free(cal);
    TEST_ASSERT_NULL(msg);
    TEST_ASSERT_NULL(short_msg);
    TEST_ASSERT_NULL(cal);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);

    MemTrack_Quit();
    TEST_REPORT();
}
