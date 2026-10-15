#include "memtrack.h"
#include "test_helpers.h"
#include <pthread.h>

#define NUM_THREADS 8
#define ITERATIONS_PER_THREAD 100

typedef struct {
    int thread_id;
    int success;
} ThreadData;

static void *worker_func(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->success = 1;

    for (int i = 0; i < ITERATIONS_PER_THREAD; i++) {
        size_t size = 32 + (i % 64);
        int *buf = (int *)t_malloc(size * sizeof(int));
        if (!buf) {
            data->success = 0;
            break;
        }

        // Fill with pattern specific to this thread and iteration
        int pattern = (data->thread_id << 16) | i;
        for (size_t j = 0; j < size; j++) {
            buf[j] = pattern;
        }

        // Verify pattern
        for (size_t j = 0; j < size; j++) {
            if (buf[j] != pattern) {
                data->success = 0;
                break;
            }
        }

        // Realloc to larger size
        size_t new_size = size * 2;
        buf = (int *)t_realloc(buf, new_size * sizeof(int));
        if (!buf) {
            data->success = 0;
            break;
        }

        // Verify original content preserved
        for (size_t j = 0; j < size; j++) {
            if (buf[j] != pattern) {
                data->success = 0;
                break;
            }
        }

        // Test strdup
        char msg[64];
        snprintf(msg, sizeof(msg), "thread-%d-iter-%d", data->thread_id, i);
        char *str = t_strdup(msg);
        if (!str || strcmp(str, msg) != 0) {
            data->success = 0;
        }

        t_free(str);
        t_free(buf);
    }

    return NULL;
}

int main(void) {
    printf("=== Running test_threads (Multi-threading stress test) ===\n");

    int res = MemTrack_Init(NULL, NULL, true, false);
    TEST_ASSERT_EQ(res, 0);

    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].success = 0;
        int rc = pthread_create(&threads[i], NULL, worker_func, &thread_data[i]);
        TEST_ASSERT_EQ(rc, 0);
    }

    // While threads are executing, call MemTrack_memory_usage and MemTrack_is_memory_leak
    for (int i = 0; i < 50; i++) {
        (void)MemTrack_memory_usage();
        (void)MemTrack_is_memory_leak();
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        TEST_ASSERT_EQ(thread_data[i].success, 1);
    }

    // All threads joined, all allocations should be freed
    TEST_ASSERT_EQ(MemTrack_memory_usage(), 0);
    TEST_ASSERT_EQ(MemTrack_is_memory_leak(), 0);

    // Leave a small allocation so MemTrack_Quit tears down and resets info.init
    void *cleanup = t_malloc(16);
    TEST_ASSERT_NOT_NULL(cleanup);
    MemTrack_Quit();

    TEST_REPORT();
}
