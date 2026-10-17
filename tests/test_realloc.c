#include "memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_realloc ===\n");

    int res = MemTrack_Init(NULL, NULL, true, false);
    TEST_ASSERT_EQ(res, 0);

    // Realloc on NULL pointer acts like malloc
    char *buf = (char *)t_realloc(NULL, 64);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 64);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 1);

    // Populate buffer
    strcpy(buf, "Hello MemTrack Realloc Test!");
    TEST_ASSERT_STR_EQ(buf, "Hello MemTrack Realloc Test!");

    // Realloc larger (grow from 64 to 256)
    buf = (char *)t_realloc(buf, 256);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 256);
    // Data must be preserved
    TEST_ASSERT_STR_EQ(buf, "Hello MemTrack Realloc Test!");

    // Append more data
    strcat(buf, " More content added.");
    TEST_ASSERT_STR_EQ(buf, "Hello MemTrack Realloc Test! More content added.");

    // Realloc smaller (shrink to 30)
    buf = (char *)t_realloc(buf, 30);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 30);
    buf[29] = '\0';
    TEST_ASSERT_EQ(strncmp(buf, "Hello MemTrack Realloc Test!", 28), 0);

    // Realloc same size (30 to 30)
    buf = (char *)t_realloc(buf, 30);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 30);

    // Free
    t_free(buf);
    TEST_ASSERT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 0);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);

    TEST_REPORT();
}
