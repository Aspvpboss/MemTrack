#include "memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_auto_null ===\n");

    // Case 1: auto_null_pointers = true
    {
        int res = MemTrack_Init(NULL, NULL, true, false);
        TEST_ASSERT_EQ(res, 0);

        // Debug / macro allocations
        void *p_malloc = t_malloc(64);
        int *p_calloc = (int *)t_calloc(8, sizeof(int));
        char *p_strdup = t_strdup("auto null strdup");
        char *p_strndup = t_strndup("auto null strndup", 9);

        TEST_ASSERT_NOT_NULL(p_malloc);
        TEST_ASSERT_NOT_NULL(p_calloc);
        TEST_ASSERT_NOT_NULL(p_strdup);
        TEST_ASSERT_NOT_NULL(p_strndup);

        t_free(p_malloc);
        t_free(p_calloc);
        t_free(p_strdup);
        t_free(p_strndup);

        TEST_ASSERT_NULL(p_malloc);
        TEST_ASSERT_NULL(p_calloc);
        TEST_ASSERT_NULL(p_strdup);
        TEST_ASSERT_NULL(p_strndup);

        // Double free when auto_null_pointers = true should be a safe no-op
        t_free(p_malloc);
        TEST_ASSERT_NULL(p_malloc);

        // Direct Safe and Debug free
        void *p_safe = MemTrack_Safe_malloc(64);
        TEST_ASSERT_NOT_NULL(p_safe);
        MemTrack_Safe_free(&p_safe);
        TEST_ASSERT_NULL(p_safe);
        MemTrack_Safe_free(&p_safe); // already NULL
        MemTrack_Safe_free(NULL);    // NULL double pointer

        void *p_dbg = MemTrack_Debug_malloc(64, __FILE__, __LINE__);
        TEST_ASSERT_NOT_NULL(p_dbg);
        MemTrack_Debug_free(&p_dbg, __FILE__, __LINE__);
        TEST_ASSERT_NULL(p_dbg);
        MemTrack_Debug_free(&p_dbg, __FILE__, __LINE__); // already NULL
        MemTrack_Debug_free(NULL, __FILE__, __LINE__);   // NULL double pointer

        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);

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

        // Second t_free on the non-nulled pointer in Debug mode must detect it is no longer tracked
        // and refuse to double-free it!
        t_free(p1);
        TEST_ASSERT_EQ(p1, saved1);

        int *p_cal = (int *)t_calloc(4, sizeof(int));
        void *saved_cal = p_cal;
        t_free(p_cal);
        TEST_ASSERT_EQ((void *)p_cal, saved_cal);

        char *p_ndup = t_strndup("non-null free", 8);
        void *saved_ndup = p_ndup;
        t_free(p_ndup);
        TEST_ASSERT_EQ((void *)p_ndup, saved_ndup);

        void *p2 = MemTrack_Safe_malloc(64);
        TEST_ASSERT_NOT_NULL(p2);
        void *saved2 = p2;
        MemTrack_Safe_free(&p2);
        TEST_ASSERT_EQ(p2, saved2);
        TEST_ASSERT_NOT_NULL(p2);

        void *p3 = MemTrack_Debug_malloc(64, __FILE__, __LINE__);
        TEST_ASSERT_NOT_NULL(p3);
        void *saved3 = p3;
        MemTrack_Debug_free(&p3, __FILE__, __LINE__);
        TEST_ASSERT_EQ(p3, saved3);
        TEST_ASSERT_NOT_NULL(p3);

        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);

        MemTrack_Quit();
    }

    TEST_REPORT();
}
