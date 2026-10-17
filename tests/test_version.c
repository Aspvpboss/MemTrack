#include "memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_version ===\n");

    int major = -1;
    int minor = -1;
    int micro = -1;

    // Valid inputs
    int res = MemTrack_Get_Version(&major, &minor, &micro);
    TEST_ASSERT_EQ(res, 0);
    TEST_ASSERT_EQ(major, 1);
    TEST_ASSERT_EQ(minor, 4);
    TEST_ASSERT_EQ(micro, 0);

    // Invalid inputs: null major
    res = MemTrack_Get_Version(NULL, &minor, &micro);
    TEST_ASSERT_EQ(res, 1);

    // Invalid inputs: null minor
    res = MemTrack_Get_Version(&major, NULL, &micro);
    TEST_ASSERT_EQ(res, 1);

    // Invalid inputs: null micro
    res = MemTrack_Get_Version(&major, &minor, NULL);
    TEST_ASSERT_EQ(res, 1);

    // Invalid inputs: all null
    res = MemTrack_Get_Version(NULL, NULL, NULL);
    TEST_ASSERT_EQ(res, 1);

    TEST_REPORT();
}
