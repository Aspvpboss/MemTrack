#include "memtrack.h"
#include "test_helpers.h"

static int g_strndup_fail_count = 0;

static void strndup_fail_handler(void *arg) {
    if (arg) {
        (*(int *)arg)++;
    }
}

/* ---------- MemTrack_strndup (raw helper) ---------- */

static void test_MemTrack_strndup_basic(void) {
    const char *orig = "Hello, MemTrack v1.4.0!";
    size_t len = strlen(orig);

    // Exact length
    char *s1 = MemTrack_strndup(orig, len);
    TEST_ASSERT_NOT_NULL(s1);
    TEST_ASSERT_NE((uintptr_t)s1, (uintptr_t)orig);
    TEST_ASSERT_STR_EQ(s1, orig);
    TEST_ASSERT_EQ(strlen(s1), len);
    TRACK_FREE(s1);

    // Size larger than string length (including huge size)
    char *s2 = MemTrack_strndup(orig, len + 1000);
    TEST_ASSERT_NOT_NULL(s2);
    TEST_ASSERT_STR_EQ(s2, orig);
    TEST_ASSERT_EQ(strlen(s2), len);
    TRACK_FREE(s2);

    // Huge size (SIZE_MAX) on a null-terminated string should stop at '\0' and not overflow/fail
    char *s3 = MemTrack_strndup(orig, SIZE_MAX);
    TEST_ASSERT_NOT_NULL(s3);
    TEST_ASSERT_STR_EQ(s3, orig);
    TEST_ASSERT_EQ(strlen(s3), len);
    TRACK_FREE(s3);
}

static void test_MemTrack_strndup_truncation(void) {
    const char *orig = "abcdefghi";

    // Truncate to 5 chars
    char *s1 = MemTrack_strndup(orig, 5);
    TEST_ASSERT_NOT_NULL(s1);
    TEST_ASSERT_STR_EQ(s1, "abcde");
    TEST_ASSERT_EQ(strlen(s1), (size_t)5);
    TEST_ASSERT_EQ(s1[5], '\0');
    TRACK_FREE(s1);

    // Truncate to 1 char
    char *s2 = MemTrack_strndup(orig, 1);
    TEST_ASSERT_NOT_NULL(s2);
    TEST_ASSERT_STR_EQ(s2, "a");
    TEST_ASSERT_EQ(strlen(s2), (size_t)1);
    TEST_ASSERT_EQ(s2[1], '\0');
    TRACK_FREE(s2);

    // Truncate to 0 chars
    char *s3 = MemTrack_strndup(orig, 0);
    TEST_ASSERT_NOT_NULL(s3);
    TEST_ASSERT_STR_EQ(s3, "");
    TEST_ASSERT_EQ(strlen(s3), (size_t)0);
    TEST_ASSERT_EQ(s3[0], '\0');
    TRACK_FREE(s3);
}

static void test_MemTrack_strndup_edge_inputs(void) {
    // NULL source
    TEST_ASSERT_NULL(MemTrack_strndup(NULL, 0));
    TEST_ASSERT_NULL(MemTrack_strndup(NULL, 10));
    TEST_ASSERT_NULL(MemTrack_strndup(NULL, SIZE_MAX));

    // Empty string with various sizes
    char *e0 = MemTrack_strndup("", 0);
    TEST_ASSERT_NOT_NULL(e0);
    TEST_ASSERT_STR_EQ(e0, "");
    TRACK_FREE(e0);

    char *e1 = MemTrack_strndup("", 1);
    TEST_ASSERT_NOT_NULL(e1);
    TEST_ASSERT_STR_EQ(e1, "");
    TRACK_FREE(e1);

    char *e100 = MemTrack_strndup("", 100);
    TEST_ASSERT_NOT_NULL(e100);
    TEST_ASSERT_STR_EQ(e100, "");
    TRACK_FREE(e100);

    // Non-null-terminated character buffer
    char raw_buf[4] = {'M', 'E', 'M', 'T'};
    char *from_raw = MemTrack_strndup(raw_buf, 4);
    TEST_ASSERT_NOT_NULL(from_raw);
    TEST_ASSERT_STR_EQ(from_raw, "MEMT");
    TEST_ASSERT_EQ(strlen(from_raw), (size_t)4);
    TEST_ASSERT_EQ(from_raw[4], '\0');
    TRACK_FREE(from_raw);

    // Embedded null byte within size
    const char embedded[8] = {'f', 'o', 'o', '\0', 'b', 'a', 'r', '\0'};
    char *from_emb = MemTrack_strndup(embedded, 8);
    TEST_ASSERT_NOT_NULL(from_emb);
    TEST_ASSERT_STR_EQ(from_emb, "foo");
    TEST_ASSERT_EQ(strlen(from_emb), (size_t)3);
    TRACK_FREE(from_emb);
}

/* ---------- MemTrack_Safe_strndup ---------- */

static void test_MemTrack_Safe_strndup(void) {
    int before = g_strndup_fail_count;

    char *s1 = MemTrack_Safe_strndup("Safe strndup test", 4);
    TEST_ASSERT_NOT_NULL(s1);
    TEST_ASSERT_STR_EQ(s1, "Safe");
    TEST_ASSERT_EQ(g_strndup_fail_count, before);
    // Safe allocations do not add tracking nodes
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
    MemTrack_Safe_free((void **)&s1);
    TEST_ASSERT_NULL(s1);

    // Zero size
    char *s2 = MemTrack_Safe_strndup("Non-empty", 0);
    TEST_ASSERT_NOT_NULL(s2);
    TEST_ASSERT_STR_EQ(s2, "");
    MemTrack_Safe_free((void **)&s2);
    TEST_ASSERT_NULL(s2);

    // Size larger than string
    char *s3 = MemTrack_Safe_strndup("Short", 500);
    TEST_ASSERT_NOT_NULL(s3);
    TEST_ASSERT_STR_EQ(s3, "Short");
    MemTrack_Safe_free((void **)&s3);
    TEST_ASSERT_NULL(s3);

    // NULL input to MemTrack_Safe_strndup returns NULL and invokes check_malloc_error when initialized
    before = g_strndup_fail_count;
    char *s_null = MemTrack_Safe_strndup(NULL, 5);
    TEST_ASSERT_NULL(s_null);
}

/* ---------- MemTrack_Debug_strndup & t_strndup ---------- */

static void test_MemTrack_Debug_strndup_and_macro(void) {
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);

    // 1. Truncated string: "Hello, World!" with size 5 -> allocates 6 bytes ("Hello\0")
    char *s1 = t_strndup("Hello, World!", 5);
    TEST_ASSERT_NOT_NULL(s1);
    TEST_ASSERT_STR_EQ(s1, "Hello");
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)6);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 1);

    // 2. Oversized length: "Hi" with size 100 -> must only allocate 3 bytes ("Hi\0"), NOT 101
    char *s2 = t_strndup("Hi", 100);
    TEST_ASSERT_NOT_NULL(s2);
    TEST_ASSERT_STR_EQ(s2, "Hi");
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)(6 + 3));

    // 3. Size 0: allocates 1 byte ("\0")
    char *s3 = t_strndup("Anything", 0);
    TEST_ASSERT_NOT_NULL(s3);
    TEST_ASSERT_STR_EQ(s3, "");
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)(6 + 3 + 1));

    // 4. Empty string with large size: allocates 1 byte ("\0")
    char *s4 = t_strndup("", 999);
    TEST_ASSERT_NOT_NULL(s4);
    TEST_ASSERT_STR_EQ(s4, "");
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)(6 + 3 + 1 + 1));

    // 5. Embedded null byte: "abc\0xyz" with size 7 -> allocates 4 bytes ("abc\0")
    const char emb[7] = {'a', 'b', 'c', '\0', 'x', 'y', 'z'};
    char *s5 = t_strndup(emb, 7);
    TEST_ASSERT_NOT_NULL(s5);
    TEST_ASSERT_STR_EQ(s5, "abc");
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)(6 + 3 + 1 + 1 + 4));

    // 6. Non-null-terminated array: 4 chars -> allocates 5 bytes
    char raw[4] = {'1', '2', '3', '4'};
    char *s6 = MemTrack_Debug_strndup(raw, 4, __FILE__, __LINE__);
    TEST_ASSERT_NOT_NULL(s6);
    TEST_ASSERT_STR_EQ(s6, "1234");
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)(6 + 3 + 1 + 1 + 4 + 5));

    // 7. NULL source: returns NULL, does not track, does not call fail_handler
    int before = g_strndup_fail_count;
    char *s_null = t_strndup(NULL, 10);
    TEST_ASSERT_NULL(s_null);
    TEST_ASSERT_EQ(g_strndup_fail_count, before);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)20);

    // 8. Reallocating a t_strndup result
    s1 = (char *)t_realloc(s1, 32);
    TEST_ASSERT_NOT_NULL(s1);
    TEST_ASSERT_STR_EQ(s1, "Hello");
    strcat(s1, " Extended!");
    TEST_ASSERT_STR_EQ(s1, "Hello Extended!");
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)(32 + 3 + 1 + 1 + 4 + 5));

    // Free all in mixed order
    t_free(s3);
    TEST_ASSERT_NULL(s3);
    t_free(s1);
    TEST_ASSERT_NULL(s1);
    t_free(s6);
    TEST_ASSERT_NULL(s6);
    t_free(s2);
    TEST_ASSERT_NULL(s2);
    t_free(s4);
    TEST_ASSERT_NULL(s4);
    t_free(s5);
    TEST_ASSERT_NULL(s5);

    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
}

int main(void) {
    printf("=== Running test_strndup ===\n");

    int res = MemTrack_Init(strndup_fail_handler, &g_strndup_fail_count, true, false);
    TEST_ASSERT_EQ(res, 0);

    test_MemTrack_strndup_basic();
    test_MemTrack_strndup_truncation();
    test_MemTrack_strndup_edge_inputs();
    test_MemTrack_Safe_strndup();
    test_MemTrack_Debug_strndup_and_macro();

    MemTrack_Quit();
    TEST_REPORT();
}
