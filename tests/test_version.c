#include "memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_version ===\n");

    int major = -1;
    int minor = -1;
    int micro = -1;

    // Valid inputs
    int res = MemTrack_GetVersion(&major, &minor, &micro);
    TEST_ASSERT_EQ(res, 0);
    TEST_ASSERT_EQ(major, 1);
    TEST_ASSERT_EQ(minor, 3);
    TEST_ASSERT_EQ(micro, 1);

    // Invalid inputs: null major
    res = MemTrack_GetVersion(NULL, &minor, &micro);
    TEST_ASSERT_EQ(res, 1);

    // Invalid inputs: null minor
    res = MemTrack_GetVersion(&major, NULL, &micro);
    TEST_ASSERT_EQ(res, 1);

    // Invalid inputs: null micro
    res = MemTrack_GetVersion(&major, &minor, NULL);
    TEST_ASSERT_EQ(res, 1);

    // Invalid inputs: all null
    res = MemTrack_GetVersion(NULL, NULL, NULL);
    TEST_ASSERT_EQ(res, 1);

    TEST_REPORT();
}
