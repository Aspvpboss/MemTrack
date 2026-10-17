#include "memtrack.h"
#include "test_helpers.h"

static int g_realloc_fail_count = 0;

static void realloc_fail_handler(void *arg) {
    if (arg) {
        (*(int *)arg)++;
    }
}

int main(void) {
    printf("=== Running test_realloc ===\n");

    int res = MemTrack_Init(realloc_fail_handler, &g_realloc_fail_count, true, false);
    TEST_ASSERT_EQ(res, 0);

    // 1. Realloc on NULL pointer acts like malloc
    char *buf = (char *)t_malloc(64);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)64);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 1);

    // Populate buffer
    strcpy(buf, "Hello MemTrack Realloc Test!");
    TEST_ASSERT_STR_EQ(buf, "Hello MemTrack Realloc Test!");

    // 2. Realloc larger (grow from 64 to 256)
    buf = (char *)t_realloc(buf, 256);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)256);
    TEST_ASSERT_STR_EQ(buf, "Hello MemTrack Realloc Test!");

    // Append more data
    strcat(buf, " More content added.");
    TEST_ASSERT_STR_EQ(buf, "Hello MemTrack Realloc Test! More content added.");

    // 3. Realloc smaller (shrink to 30)
    buf = (char *)t_realloc(buf, 30);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)30);
    buf[29] = '\0';
    TEST_ASSERT_EQ(strncmp(buf, "Hello MemTrack Realloc Test!", 28), 0);

    // 4. Realloc same size (30 to 30)
    buf = (char *)t_realloc(buf, 30);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)30);

    // Free
    t_free(buf);
    TEST_ASSERT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);

    // 5. Realloc on head, middle, and tail nodes in tracking list
    {
        char *head = (char *)t_malloc(10);
        char *mid  = (char *)t_malloc(20);
        char *tail = (char *)t_malloc(30);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)60);

        // Realloc head (unlinks head, appends as new tail)
        head = (char *)t_realloc(head, 15);
        TEST_ASSERT_NOT_NULL(head);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)65);

        // Realloc middle
        mid = (char *)t_realloc(mid, 25);
        TEST_ASSERT_NOT_NULL(mid);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)70);

        // Realloc current tail
        head = (char *)t_realloc(head, 50);
        TEST_ASSERT_NOT_NULL(head);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)105);

        t_free(tail);
        t_free(mid);
        t_free(head);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
    }


    // 7. Repeated grow/shrink stress loop
    {
        unsigned char *p = (unsigned char *)t_malloc(1);
        TEST_ASSERT_NOT_NULL(p);
        p[0] = 0xAB;

        for (size_t i = 2; i <= 128; i++) {
            p = (unsigned char *)t_realloc(p, i);
            TEST_ASSERT_NOT_NULL(p);
            TEST_ASSERT_EQ(p[0], 0xAB);
            p[i - 1] = (unsigned char)(i & 0xFF);
            TEST_ASSERT_EQ(MemTrack_Memory_Usage(), i);
        }
        for (size_t i = 127; i >= 1; i--) {
            p = (unsigned char *)t_realloc(p, i);
            TEST_ASSERT_NOT_NULL(p);
            TEST_ASSERT_EQ(p[0], 0xAB);
            TEST_ASSERT_EQ(MemTrack_Memory_Usage(), i);
        }
        t_free(p);
        TEST_ASSERT_NULL(p);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
    }

    // 8. MemTrack_Safe_realloc direct usage
    {
        char *sp = (char *)MemTrack_Safe_malloc(32);
        TEST_ASSERT_NOT_NULL(sp);
        strcpy(sp, "Safe realloc");
        sp = (char *)MemTrack_Safe_realloc(sp, 64);
        TEST_ASSERT_NOT_NULL(sp);
        TEST_ASSERT_STR_EQ(sp, "Safe realloc");
        MemTrack_Safe_free((void **)&sp);
        TEST_ASSERT_NULL(sp);
    }

    MemTrack_Quit();
    TEST_REPORT();
}
