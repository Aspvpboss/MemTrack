#define MEMTRACK_IMPLEMENTATION

#include "memtrack.h"
#include "test_helpers.h"

static int g_fail_count = 0;

static void fail_handler(void *arg) {
    (*(int *)arg)++;
}

static bool all_zero(const void *p, size_t n) {
    const unsigned char *b = (const unsigned char *)p;
    for (size_t i = 0; i < n; i++) {
        if (b[i] != 0) return false;
    }
    return true;
}

/* ---------- MemTrack_safe_calloc ---------- */

static void test_MemTrack_safe_calloc_zeroed(void) {
    int *arr = (int *)MemTrack_safe_calloc(16, sizeof(int));
    TEST_ASSERT_NOT_NULL(arr);
    if (arr) {
        TEST_ASSERT_TRUE(all_zero(arr, 16 * sizeof(int)));
    }
    MemTrack_safe_free((void **)&arr);
}

static void test_MemTrack_safe_calloc_usable(void) {
    int *arr = (int *)MemTrack_safe_calloc(8, sizeof(int));
    TEST_ASSERT_NOT_NULL(arr);
    if (!arr) return;
    for (int i = 0; i < 8; i++) arr[i] = i * 3;
    for (int i = 0; i < 8; i++) TEST_ASSERT_EQ(arr[i], i * 3);
    MemTrack_safe_free((void **)&arr);
}

static void test_MemTrack_safe_calloc_single_byte(void) {
    char *c = (char *)MemTrack_safe_calloc(1, 1);
    TEST_ASSERT_NOT_NULL(c);
    if (c) TEST_ASSERT_EQ(*c, 0);
    MemTrack_safe_free((void **)&c);
}

static void test_MemTrack_safe_calloc_zero_args(void) {
    /* Result is implementation-defined (NULL or unique pointer); must not crash */
    void *a = MemTrack_safe_calloc(0, 16);
    void *b = MemTrack_safe_calloc(16, 0);
    MemTrack_safe_free(&a);
    MemTrack_safe_free(&b);
    TEST_ASSERT_TRUE(true);
}

static void test_MemTrack_safe_calloc_overflow_returns_null(void) {
    int before = g_fail_count;
    void *p = MemTrack_safe_calloc(SIZE_MAX, 2);
    TEST_ASSERT_NULL(p);
    TEST_ASSERT_EQ(g_fail_count, before + 1);
}

static void test_MemTrack_safe_calloc_huge_returns_null(void) {
    int before = g_fail_count;
    void *p = MemTrack_safe_calloc(SIZE_MAX / 2, 1);
    TEST_ASSERT_NULL(p);
    TEST_ASSERT_EQ(g_fail_count, before + 1);
}

static void test_MemTrack_safe_calloc_success_no_handler_call(void) {
    int before = g_fail_count;
    void *p = MemTrack_safe_calloc(4, 4);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQ(g_fail_count, before);
    MemTrack_safe_free(&p);
}

static void test_MemTrack_safe_free_nulls_calloc_pointer(void) {
    void *p = MemTrack_safe_calloc(4, 4);
    TEST_ASSERT_NOT_NULL(p);
    MemTrack_safe_free(&p);
    TEST_ASSERT_NULL(p);
}

/* ---------- MemTrack_debug_calloc ---------- */

static void test_MemTrack_debug_calloc_zeroed(void) {
    int *arr = (int *)MemTrack_debug_calloc(32, sizeof(int), __FILE__, __LINE__);
    TEST_ASSERT_NOT_NULL(arr);
    if (arr) {
        TEST_ASSERT_TRUE(all_zero(arr, 32 * sizeof(int)));
    }
    MemTrack_debug_free((void **)&arr, __FILE__, __LINE__);
}

static void test_MemTrack_debug_calloc_tracked_and_freed(void) {
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 0);

    void *p = MemTrack_debug_calloc(10, 4, __FILE__, __LINE__);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 1);

    MemTrack_debug_free(&p, __FILE__, __LINE__);
    TEST_ASSERT_NULL(p);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 0);
}

/* Total tracked size should be num * size. NOTE: MemTrack_debug_calloc currently
 * passes only `size` to append_allocation, so this fails until fixed. */
static void test_MemTrack_debug_calloc_tracks_total_size(void) {
    void *p = MemTrack_debug_calloc(10, 4, __FILE__, __LINE__);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), (size_t)40);
    MemTrack_debug_free(&p, __FILE__, __LINE__);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), (size_t)0);
}

static void test_MemTrack_debug_calloc_multiple_allocations(void) {
    void *a = MemTrack_debug_calloc(1, 8, __FILE__, __LINE__);
    void *b = MemTrack_debug_calloc(1, 16, __FILE__, __LINE__);
    void *c = MemTrack_debug_calloc(1, 32, __FILE__, __LINE__);
    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_NOT_NULL(c);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), (size_t)56);

    /* free middle, head, then tail to exercise list unlinking */
    MemTrack_debug_free(&b, __FILE__, __LINE__);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 1);
    MemTrack_debug_free(&a, __FILE__, __LINE__);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 1);
    MemTrack_debug_free(&c, __FILE__, __LINE__);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 0);
}

static void test_MemTrack_debug_calloc_overflow_returns_null(void) {
    int before = g_fail_count;
    void *p = MemTrack_debug_calloc(SIZE_MAX, 2, __FILE__, __LINE__);
    TEST_ASSERT_NULL(p);
    TEST_ASSERT_EQ(g_fail_count, before + 1);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 0); /* nothing tracked on failure */
}

static void test_MemTrack_debug_calloc_then_realloc(void) {
    int *arr = (int *)MemTrack_debug_calloc(4, sizeof(int), __FILE__, __LINE__);
    TEST_ASSERT_NOT_NULL(arr);
    if (!arr) return;

    arr = (int *)MemTrack_debug_realloc(arr, 8 * sizeof(int), __FILE__, __LINE__);
    TEST_ASSERT_NOT_NULL(arr);
    if (!arr) return;

    /* original calloc'd region must still be zero after growth */
    TEST_ASSERT_TRUE(all_zero(arr, 4 * sizeof(int)));
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 1);

    MemTrack_debug_free((void **)&arr, __FILE__, __LINE__);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 0);
}

static void test_MemTrack_debug_free_untracked_calloc_pointer(void) {
    /* Pointer allocated with MemTrack_safe_calloc isn't tracked; MemTrack_debug_free should
     * report an error and leave the pointer alone rather than crash. */
    void *p = MemTrack_safe_calloc(4, 4);
    TEST_ASSERT_NOT_NULL(p);
    void *copy = p;
    MemTrack_debug_free(&p, __FILE__, __LINE__);
    TEST_ASSERT_EQ(p == copy, 1); /* not nulled, not freed */
    TRACK_FREE(copy);
}

/* ---------- t_calloc / t_free macros (TRACK_ALLOCATIONS mode) ---------- */

static void test_t_calloc_macro(void) {
    int *arr = (int *)t_calloc(12, sizeof(int));
    TEST_ASSERT_NOT_NULL(arr);
    if (arr) {
        TEST_ASSERT_TRUE(all_zero(arr, 12 * sizeof(int)));
        TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 1);
    }
    t_free(arr);
    TEST_ASSERT_NULL(arr);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 0);
}

static void test_t_calloc_struct(void) {
    typedef struct { int a; double b; char *c; } S;
    S *s = (S *)t_calloc(3, sizeof(S));
    TEST_ASSERT_NOT_NULL(s);
    if (s) {
        for (int i = 0; i < 3; i++) {
            TEST_ASSERT_EQ(s[i].a, 0);
            TEST_ASSERT_TRUE(s[i].b == 0.0);
            TEST_ASSERT_NULL(s[i].c);
        }
    }
    t_free(s);
}

int main(void) {
    /* Init once: MemTrack_Quit() returns early on an empty list without
     * resetting `init`, so repeated Init/Quit cycles aren't reliable. */
    if (MemTrack_Init(fail_handler, &g_fail_count, true, false)) {
        fprintf(stderr, "MemTrack_Init failed\n");
        return 1;
    }

    test_MemTrack_safe_calloc_zeroed();
    test_MemTrack_safe_calloc_usable();
    test_MemTrack_safe_calloc_single_byte();
    test_MemTrack_safe_calloc_zero_args();
    test_MemTrack_safe_calloc_overflow_returns_null();
    test_MemTrack_safe_calloc_huge_returns_null();
    test_MemTrack_safe_calloc_success_no_handler_call();
    test_MemTrack_safe_free_nulls_calloc_pointer();

    test_MemTrack_debug_calloc_zeroed();
    test_MemTrack_debug_calloc_tracked_and_freed();
    test_MemTrack_debug_calloc_tracks_total_size();
    test_MemTrack_debug_calloc_multiple_allocations();
    test_MemTrack_debug_calloc_overflow_returns_null();
    test_MemTrack_debug_calloc_then_realloc();
    test_MemTrack_debug_free_untracked_calloc_pointer();

    test_t_calloc_macro();
    test_t_calloc_struct();

    MemTrack_Quit();
    TEST_REPORT();
}