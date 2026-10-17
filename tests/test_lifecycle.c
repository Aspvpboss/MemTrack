#include "memtrack.h"
#include "test_helpers.h"

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#include <sys/wait.h>
#endif

typedef struct {
    int count;
    int last_code;
} ErrorTracker;

static void custom_err_handler_1(void *arg) {
    ErrorTracker *et = (ErrorTracker *)arg;
    if (et) {
        et->count++;
        et->last_code = 101;
    }
}

static void custom_err_handler_2(void *arg) {
    ErrorTracker *et = (ErrorTracker *)arg;
    if (et) {
        et->count += 10;
        et->last_code = 202;
    }
}

static void test_pre_init_and_uninitialized_behavior(void) {
    // Pre-init state checks
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
    MemTrack_Tracking_Info();

    // Quit before Init should not crash
    MemTrack_Quit();
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);

    // Allocations while uninitialized: fail_handler must not be called even on failure
    void *fail_ptr = MemTrack_Safe_calloc(SIZE_MAX, 2);
    TEST_ASSERT_NULL(fail_ptr);
}

static void test_spam_init_without_quit(void) {
    int res = MemTrack_Init(NULL, NULL, true, false);
    TEST_ASSERT_EQ(res, 0);

    void *p = t_malloc(64);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)64);

    // Spamming MemTrack_Init 100 times while already initialized should always return 1
    // and must NOT wipe out existing tracked allocations or change settings
    for (int i = 0; i < 100; i++) {
        res = MemTrack_Init(NULL, NULL, false, true);
        TEST_ASSERT_EQ(res, 1);
    }

    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)64);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 1);

    // auto_null_pointers should still be true from the initial Init call
    t_free(p);
    TEST_ASSERT_NULL(p);
    TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
    TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);

    MemTrack_Quit();
}

static void test_spam_init_quit_empty_cycles(void) {
    // Spam Init and Quit 1000 times with an empty allocation list
    for (int i = 0; i < 1000; i++) {
        int res = MemTrack_Init(NULL, NULL, (i % 2) == 0, false);
        TEST_ASSERT_EQ(res, 0);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
        MemTrack_Quit();
    }

    // Multiple consecutive Quits when already uninitialized
    for (int i = 0; i < 10; i++) {
        MemTrack_Quit();
    }

    // Re-init after multiple Quits must still succeed
    int res = MemTrack_Init(NULL, NULL, true, false);
    TEST_ASSERT_EQ(res, 0);
    MemTrack_Quit();
}

static void test_spam_init_quit_with_freed_allocations(void) {
    // Spam 250 full lifecycle cycles with all allocation types freed before Quit
    for (int i = 0; i < 250; i++) {
        bool auto_null = (i % 2) == 0;
        int res = MemTrack_Init(NULL, NULL, auto_null, false);
        TEST_ASSERT_EQ(res, 0);

        char *m = (char *)t_malloc(32);
        int *c = (int *)t_calloc(8, sizeof(int));
        char *s1 = t_strdup("cycle_strdup");
        char *s2 = t_strndup("cycle_strndup_extra", 13);

        TEST_ASSERT_NOT_NULL(m);
        TEST_ASSERT_NOT_NULL(c);
        TEST_ASSERT_NOT_NULL(s1);
        TEST_ASSERT_NOT_NULL(s2);

        m = (char *)t_realloc(m, 64);
        TEST_ASSERT_NOT_NULL(m);

        size_t expected = 64 + (8 * sizeof(int)) + (strlen("cycle_strdup") + 1) + 14;
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), expected);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 1);

        t_free(m);
        t_free(c);
        t_free(s1);
        t_free(s2);

        if (auto_null) {
            TEST_ASSERT_NULL(m);
            TEST_ASSERT_NULL(c);
            TEST_ASSERT_NULL(s1);
            TEST_ASSERT_NULL(s2);
        } else {
            TEST_ASSERT_NOT_NULL(m);
            TEST_ASSERT_NOT_NULL(c);
            TEST_ASSERT_NOT_NULL(s1);
            TEST_ASSERT_NOT_NULL(s2);
        }

        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);

        MemTrack_Quit();
    }
}

static void test_spam_init_quit_with_active_leaks_teardown(void) {
    // Spam 200 cycles where allocations are still tracked when MemTrack_Quit() is called.
    // MemTrack_Quit() frees all tracking nodes and resets head/tail/init.
    for (int i = 0; i < 200; i++) {
        int res = MemTrack_Init(NULL, NULL, true, false);
        TEST_ASSERT_EQ(res, 0);

        // Must start clean despite previous cycle quitting with active tracked nodes
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);

        void *p1 = t_malloc(16 + (i % 32));
        void *p2 = t_calloc(4, 8);
        char *p3 = t_strndup("unfreed_before_quit", 7);

        TEST_ASSERT_NOT_NULL(p1);
        TEST_ASSERT_NOT_NULL(p2);
        TEST_ASSERT_NOT_NULL(p3);
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 1);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)(16 + (i % 32) + 32 + 8));

        // Tear down tracking list via MemTrack_Quit
        MemTrack_Quit();

        // After Quit, tracking list must be empty
        TEST_ASSERT_EQ(MemTrack_Is_Memory_Leak(), 0);
        TEST_ASSERT_EQ(MemTrack_Memory_Usage(), (size_t)0);

        // Free raw allocations so test process itself doesn't accumulate heap memory
        TRACK_FREE(p1);
        TRACK_FREE(p2);
        TRACK_FREE(p3);
    }
}

static void test_config_and_handler_switching_across_cycles(void) {
    ErrorTracker tracker1 = {0, 0};
    ErrorTracker tracker2 = {0, 0};

    // Cycle 1: handler 1
    TEST_ASSERT_EQ(MemTrack_Init(custom_err_handler_1, &tracker1, true, false), 0);
    void *fail1 = MemTrack_Debug_calloc(SIZE_MAX, 2, __FILE__, __LINE__);
    TEST_ASSERT_NULL(fail1);
    TEST_ASSERT_EQ(tracker1.count, 1);
    TEST_ASSERT_EQ(tracker1.last_code, 101);
    MemTrack_Quit();

    // While uninitialized after Quit, handler 1 must NOT fire
    void *fail_uninit = MemTrack_Safe_calloc(SIZE_MAX, 2);
    TEST_ASSERT_NULL(fail_uninit);
    TEST_ASSERT_EQ(tracker1.count, 1);

    // Cycle 2: handler 2
    TEST_ASSERT_EQ(MemTrack_Init(custom_err_handler_2, &tracker2, false, false), 0);
    void *fail2 = MemTrack_Safe_calloc(SIZE_MAX, 2);
    TEST_ASSERT_NULL(fail2);
    TEST_ASSERT_EQ(tracker1.count, 1); // unchanged
    TEST_ASSERT_EQ(tracker2.count, 10);
    TEST_ASSERT_EQ(tracker2.last_code, 202);
    MemTrack_Quit();

    // Cycle 3: NULL handler
    TEST_ASSERT_EQ(MemTrack_Init(NULL, NULL, true, false), 0);
    void *fail3 = MemTrack_Debug_calloc(SIZE_MAX, 2, __FILE__, __LINE__);
    TEST_ASSERT_NULL(fail3);
    TEST_ASSERT_EQ(tracker1.count, 1);
    TEST_ASSERT_EQ(tracker2.count, 10);
    MemTrack_Quit();
}

#if defined(__unix__) || defined(__APPLE__)
static void test_abort_if_mem_error_subprocess(void) {
    // Test 1: Safe allocation failure with abort_if_mem_error = true exits with EXIT_FAILURE
    pid_t pid1 = fork();
    TEST_ASSERT_TRUE(pid1 >= 0);
    if (pid1 == 0) {
        // Child process: redirect stderr to /dev/null to keep test output clean
        freopen("/dev/null", "w", stderr);
        MemTrack_Init(NULL, NULL, true, true);
        void *p = MemTrack_Safe_calloc(SIZE_MAX, 2);
        (void)p;
        _exit(0); // Should not reach here
    } else if (pid1 > 0) {
        int status = 0;
        waitpid(pid1, &status, 0);
        TEST_ASSERT_TRUE(WIFEXITED(status));
        TEST_ASSERT_EQ(WEXITSTATUS(status), EXIT_FAILURE);
    }

    // Test 2: Debug allocation failure with abort_if_mem_error = true exits with EXIT_FAILURE
    pid_t pid2 = fork();
    TEST_ASSERT_TRUE(pid2 >= 0);
    if (pid2 == 0) {
        freopen("/dev/null", "w", stderr);
        MemTrack_Init(NULL, NULL, true, true);
        void *p = MemTrack_Debug_calloc(SIZE_MAX, 2, __FILE__, __LINE__);
        (void)p;
        _exit(0); // Should not reach here
    } else if (pid2 > 0) {
        int status = 0;
        waitpid(pid2, &status, 0);
        TEST_ASSERT_TRUE(WIFEXITED(status));
        TEST_ASSERT_EQ(WEXITSTATUS(status), EXIT_FAILURE);
    }
}
#endif

int main(void) {
    printf("=== Running test_lifecycle ===\n");

    test_pre_init_and_uninitialized_behavior();
    test_spam_init_without_quit();
    test_spam_init_quit_empty_cycles();
    test_spam_init_quit_with_freed_allocations();
    test_spam_init_quit_with_active_leaks_teardown();
    test_config_and_handler_switching_across_cycles();

#if defined(__unix__) || defined(__APPLE__)
    test_abort_if_mem_error_subprocess();
#endif

    TEST_REPORT();
}
