#include "memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_auto_null ===\n");

    // Case 1: auto_null_pointers = true
    {
        int res = MemTrack_Init(NULL, NULL, true, false);
        TEST_ASSERT_EQ(res, 0);

        void *p1 = t_malloc(64);
        TEST_ASSERT_NOT_NULL(p1);
        t_free(p1);
        TEST_ASSERT_NULL(p1);

        void *p2 = MemTrack_safe_malloc(64);
        TEST_ASSERT_NOT_NULL(p2);
        MemTrack_safe_free(&p2);
        TEST_ASSERT_NULL(p2);

        void *p3 = MemTrack_debug_malloc(64, __FILE__, __LINE__);
        TEST_ASSERT_NOT_NULL(p3);
        MemTrack_debug_free(&p3, __FILE__, __LINE__);
        TEST_ASSERT_NULL(p3);

        // Leave an allocation so MemTrack_Quit tears down and resets info.init
        void *p_dummy = t_malloc(16);
        TEST_ASSERT_NOT_NULL(p_dummy);
        MemTrack_Quit();
    }

    // Case 2: auto_null_pointers = false
    {
        int res = MemTrack_Init(NULL, NULL, false, false);
        TEST_ASSERT_EQ(res, 0);

        void *p1 = t_malloc(64);
        TEST_ASSERT_NOT_NULL(p1);
        void *saved1 = p1;
        t_free(p1);
        TEST_ASSERT_EQ(p1, saved1); // Pointer should not be nulled
        TEST_ASSERT_NOT_NULL(p1);

        void *p2 = MemTrack_safe_malloc(64);
        TEST_ASSERT_NOT_NULL(p2);
        void *saved2 = p2;
        MemTrack_safe_free(&p2);
        TEST_ASSERT_EQ(p2, saved2);
        TEST_ASSERT_NOT_NULL(p2);

        void *p3 = MemTrack_debug_malloc(64, __FILE__, __LINE__);
        TEST_ASSERT_NOT_NULL(p3);
        void *saved3 = p3;
        MemTrack_debug_free(&p3, __FILE__, __LINE__);
        TEST_ASSERT_EQ(p3, saved3);
        TEST_ASSERT_NOT_NULL(p3);

        void *p_dummy = t_malloc(16);
        TEST_ASSERT_NOT_NULL(p_dummy);
        MemTrack_Quit();
    }

    TEST_REPORT();
}
