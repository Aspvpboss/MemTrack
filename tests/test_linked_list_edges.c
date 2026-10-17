#include "memtrack.h"
#include "test_helpers.h"

int main(void) {
    printf("=== Running test_linked_list_edges ===\n");

    int res = MemTrack_Init(NULL, NULL, true, false);
    TEST_ASSERT_EQ(res, 0);

    // 1. Single node in list
    {
        void *p = t_malloc(50);
        TEST_ASSERT_NOT_NULL(p);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 50);
        t_free(p);
        TEST_ASSERT_NULL(p);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 0);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
    }

    // 2. Head node deletion with remaining nodes
    {
        void *a = t_malloc(10);
        void *b = t_malloc(20);
        void *c = t_malloc(30);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 60);

        t_free(a); // delete head
        TEST_ASSERT_NULL(a);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 50);

        t_free(b);
        t_free(c);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 0);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
    }

    // 3. Tail node deletion with remaining nodes
    {
        void *a = t_malloc(10);
        void *b = t_malloc(20);
        void *c = t_malloc(30);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 60);

        t_free(c); // delete tail
        TEST_ASSERT_NULL(c);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 30);

        t_free(a);
        t_free(b);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 0);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
    }

    // 4. Middle node deletion
    {
        void *a = t_malloc(10);
        void *b = t_malloc(20);
        void *c = t_malloc(30);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 60);

        t_free(b); // delete middle
        TEST_ASSERT_NULL(b);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 40);

        t_free(a);
        t_free(c);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 0);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
    }

    // 5. Reverse order free (LIFO)
    {
        const int COUNT = 50;
        void *ptrs[50];
        for (int i = 0; i < COUNT; i++) {
            ptrs[i] = t_malloc(10 + i);
            TEST_ASSERT_NOT_NULL(ptrs[i]);
        }
        for (int i = COUNT - 1; i >= 0; i--) {
            t_free(ptrs[i]);
            TEST_ASSERT_NULL(ptrs[i]);
        }
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 0);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
    }

    // 6. Forward order free (FIFO)
    {
        const int COUNT = 50;
        void *ptrs[50];
        for (int i = 0; i < COUNT; i++) {
            ptrs[i] = t_malloc(10 + i);
            TEST_ASSERT_NOT_NULL(ptrs[i]);
        }
        for (int i = 0; i < COUNT; i++) {
            t_free(ptrs[i]);
            TEST_ASSERT_NULL(ptrs[i]);
        }
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 0);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
    }

    // 7. Interleaved odd-even free
    {
        const int COUNT = 60;
        void *ptrs[60];
        for (int i = 0; i < COUNT; i++) {
            ptrs[i] = t_malloc(16);
            TEST_ASSERT_NOT_NULL(ptrs[i]);
        }
        // Free odd indices first
        for (int i = 1; i < COUNT; i += 2) {
            t_free(ptrs[i]);
            TEST_ASSERT_NULL(ptrs[i]);
        }
        // Free even indices
        for (int i = 0; i < COUNT; i += 2) {
            t_free(ptrs[i]);
            TEST_ASSERT_NULL(ptrs[i]);
        }
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 0);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
    }

    // 8. Attempting to free untracked pointer
    {
        void *tracked = t_malloc(100);
        TEST_ASSERT_NOT_NULL(tracked);

        int dummy = 42;
        void *untracked = &dummy;
        // MemTrack_Debug_free should fail gracefully, print error, and not crash
        MemTrack_Debug_free(&untracked, __FILE__, __LINE__);

        // The tracked allocation should still be intact
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 100);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 1);

        t_free(tracked);
        TEST_ASSERT_NULL(tracked);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 0);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
    }

    TEST_REPORT();
}
