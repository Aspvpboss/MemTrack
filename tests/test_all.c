#include "memtrack.h"
#include "test_helpers.h"
#include <pthread.h>

static void test_version(void) {
    int major = -1, minor = -1, micro = -1;
    TEST_ASSERT_EQ(MemTrack_GetVersion(&major, &minor, &micro), 0);
    TEST_ASSERT_EQ(major, 1);
    TEST_ASSERT_EQ(minor, 3);
    TEST_ASSERT_EQ(micro, 1);
    TEST_ASSERT_EQ(MemTrack_GetVersion(NULL, &minor, &micro), 1);
    TEST_ASSERT_EQ(MemTrack_GetVersion(&major, NULL, &micro), 1);
    TEST_ASSERT_EQ(MemTrack_GetVersion(&major, &minor, NULL), 1);
    TEST_ASSERT_EQ(MemTrack_GetVersion(NULL, NULL, NULL), 1);
}

static void test_tracking_and_usage(void) {
    char *p1 = (char *)t_malloc(100);
    TEST_ASSERT_NOT_NULL(p1);
    memset(p1, 'X', 100);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 100);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 1);

    char *p2 = (char *)t_malloc(200);
    TEST_ASSERT_NOT_NULL(p2);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 300);

    t_free(p1);
    TEST_ASSERT_NULL(p1);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 200);

    t_free(p2);
    TEST_ASSERT_NULL(p2);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 0);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 0);

    void *null_ptr = NULL;
    t_free(null_ptr);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 0);
}

static void test_realloc(void) {
    char *buf = (char *)t_realloc(NULL, 32);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 32);

    strcpy(buf, "test realloc string");
    buf = (char *)t_realloc(buf, 128);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 128);
    TEST_ASSERT_STR_EQ(buf, "test realloc string");

    buf = (char *)t_realloc(buf, 16);
    TEST_ASSERT_NOT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 16);

    t_free(buf);
    TEST_ASSERT_NULL(buf);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 0);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 0);
}

static void test_strdup(void) {
    const char *orig = "test strdup";
    char *dup = t_strdup(orig);
    TEST_ASSERT_NOT_NULL(dup);
    TEST_ASSERT_STR_EQ(dup, orig);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), strlen(orig) + 1);

    char *dup_null = t_strdup(NULL);
    TEST_ASSERT_NULL(dup_null);

    t_free(dup);
    TEST_ASSERT_NULL(dup);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 0);
}

static void test_linked_list_edges(void) {
    // 3 elements: free middle, free head, free tail
    void *a = t_malloc(10);
    void *b = t_malloc(20);
    void *c = t_malloc(30);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 60);

    t_free(b); // middle
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 40);

    t_free(a); // head
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 30);

    t_free(c); // only remaining
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 0);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 0);

    // Freeing untracked pointer
    int dummy = 99;
    void *untracked = &dummy;
    debug_free(&untracked, __FILE__, __LINE__);
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 0);
}

static void *thread_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < 50; i++) {
        void *p = t_malloc(64);
        if (p) {
            memset(p, 0x55, 64);
            t_free(p);
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
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 0);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 0);
}

int main(void) {
    printf("========================================\n");
    printf("  MemTrack Unified Test Suite Runner   \n");
    printf("========================================\n");

    int res = MemTrack_Init(NULL, NULL, true, false);
    TEST_ASSERT_EQ(res, 0);

    printf("[1/6] Running test_version...\n");
    test_version();

    printf("[2/6] Running test_tracking_and_usage...\n");
    test_tracking_and_usage();

    printf("[3/6] Running test_realloc...\n");
    test_realloc();

    printf("[4/6] Running test_strdup...\n");
    test_strdup();

    printf("[5/6] Running test_linked_list_edges...\n");
    test_linked_list_edges();

    printf("[6/6] Running test_threads...\n");
    test_threads();

    // Print tracking info safely (should be empty)
    MemTrack_tracking_info();

    printf("========================================\n");
    TEST_REPORT();
}
