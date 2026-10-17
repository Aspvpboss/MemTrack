#include "memtrack.h"
#include "test_helpers.h"
#include <pthread.h>

static void test_version(void) {
    int major = -1, minor = -1, micro = -1;
    TEST_ASSERT_EQ(MemTrack_Get_Version(&major, &minor, &micro), 0);
    TEST_ASSERT_EQ(major, 1);
    TEST_ASSERT_EQ(minor, 4);
    TEST_ASSERT_EQ(micro, 0);
    TEST_ASSERT_EQ(MemTrack_Get_Version(NULL, &minor, &micro), 1);
    TEST_ASSERT_EQ(MemTrack_Get_Version(&major, NULL, &micro), 1);
    TEST_ASSERT_EQ(MemTrack_Get_Version(&major, &minor, NULL), 1);
    TEST_ASSERT_EQ(MemTrack_Get_Version(NULL, NULL, NULL), 1);
}

static void test_tracking_and_usage(void) {
    char *p1 = (char *)t_malloc(100);
    TEST_ASSERT_NOT_NULL(p1);
    memset(p1, 'X', 100);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)100);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 1);

    char *p2 = (char *)t_malloc(200);
    TEST_ASSERT_NOT_NULL(p2);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)300);

    t_free(p1);
    TEST_ASSERT_NULL(p1);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)200);

    t_free(p2);
    TEST_ASSERT_NULL(p2);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);

    void *null_ptr = NULL;
    t_free(null_ptr);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
}

static void test_calloc_and_realloc(void) {
    int *cal = (int *)t_calloc(10, sizeof(int));
    TEST_ASSERT_NOT_NULL(cal);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), 10 * sizeof(int));
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQ(cal[i], 0);
    }
    t_free(cal);
    TEST_ASSERT_NULL(cal);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);

    char *buf = (char *)t_realloc(NULL, 32);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)32);

    strcpy(buf, "test realloc string");
    buf = (char *)t_realloc(buf, 128);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)128);
    TEST_ASSERT_STR_EQ(buf, "test realloc string");

    buf = (char *)t_realloc(buf, 16);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)16);

    t_free(buf);
    TEST_ASSERT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
}

static void test_strdup_and_strndup(void) {
    const char *orig = "test strdup";
    char *dup = t_strdup(orig);
    TEST_ASSERT_NOT_NULL(dup);
    TEST_ASSERT_STR_EQ(dup, orig);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), strlen(orig) + 1);

    char *dup_null = t_strdup(NULL);
    TEST_ASSERT_NULL(dup_null);

    t_free(dup);
    TEST_ASSERT_NULL(dup);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);

    char *ndup = t_strndup("test strndup extra", 12);
    TEST_ASSERT_NOT_NULL(ndup);
    TEST_ASSERT_STR_EQ(ndup, "test strndup");
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)13);

    char *ndup_oversized = t_strndup("abc", 500);
    TEST_ASSERT_NOT_NULL(ndup_oversized);
    TEST_ASSERT_STR_EQ(ndup_oversized, "abc");
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)(13 + 4));

    char *ndup_null = t_strndup(NULL, 10);
    TEST_ASSERT_NULL(ndup_null);

    t_free(ndup);
    t_free(ndup_oversized);
    TEST_ASSERT_NULL(ndup);
    TEST_ASSERT_NULL(ndup_oversized);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
}

static void test_linked_list_edges(void) {
    // 3 elements: free middle, free head, free tail
    void *a = t_malloc(10);
    void *b = t_malloc(20);
    void *c = t_malloc(30);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)60);

    t_free(b); // middle
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)40);

    t_free(a); // head
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)30);

    t_free(c); // only remaining
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);

    // Freeing untracked pointer
    int dummy = 99;
    void *untracked = &dummy;
    MemTrack_Debug_free(&untracked, __FILE__, __LINE__);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
}

static void *thread_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < 50; i++) {
        void *p = t_malloc(64);
        char *s = t_strndup("worker_thread_string", 6);
        int *c = (int *)t_calloc(4, sizeof(int));
        if (p) {
            memset(p, 0x55, 64);
            t_free(p);
        }
        if (s) {
            t_free(s);
        }
        if (c) {
            t_free(c);
        }
    }
    return NULL;
}

static void test_threads(void) {
    pthread_t th[4];
    for (int i = 0; i < 4; i++) {
        pthread_create(&th[i], NULL, thread_worker, NULL);
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(th[i], NULL);
    }
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
}

static void test_init_quit_spam(void) {
    MemTrack_Quit();
    for (int i = 0; i < 200; i++) {
        TEST_ASSERT_EQ(MemTrack_Init(NULL, NULL, true, false), 0);
        TEST_ASSERT_EQ(MemTrack_Init(NULL, NULL, true, false), 1);
        if (i % 2 == 0) {
            char *s = t_strndup("spam", 4);
            TEST_ASSERT_NOT_NULL(s);
            t_free(s);
        }
        MemTrack_Quit();
    }
    TEST_ASSERT_EQ(MemTrack_Init(NULL, NULL, true, false), 0);
}

int main(void) {
    printf("========================================\n");
    printf("  MemTrack Unified Test Suite Runner   \n");
    printf("========================================\n");

    int res = MemTrack_Init(NULL, NULL, true, false);
    TEST_ASSERT_EQ(res, 0);

    printf("[1/7] Running test_version...\n");
    test_version();

    printf("[2/7] Running test_tracking_and_usage...\n");
    test_tracking_and_usage();

    printf("[3/7] Running test_calloc_and_realloc...\n");
    test_calloc_and_realloc();

    printf("[4/7] Running test_strdup_and_strndup...\n");
    test_strdup_and_strndup();

    printf("[5/7] Running test_linked_list_edges...\n");
    test_linked_list_edges();

    printf("[6/7] Running test_threads...\n");
    test_threads();

    printf("[7/7] Running test_init_quit_spam...\n");
    test_init_quit_spam();

    // Print tracking info safely (should be empty)
    MemTrack_Tracking_Info();
    MemTrack_Quit();

    printf("========================================\n");
    TEST_REPORT();
}
