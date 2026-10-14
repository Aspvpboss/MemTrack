#include "memtrack.h"
#include "test_helpers.h"

// Internal functions defined in memtrack.h
void check_malloc_error(void *mem);
void debug_check_malloc_error(void *mem, const char *file, int line);

static int g_handler_called = 0;
static void *g_handler_arg_received = NULL;

static void custom_fail_handler(void *arg) {
    g_handler_called++;
    g_handler_arg_received = arg;
}

int main(void) {
    printf("=== Running test_fail_handler ===\n");

    int dummy_data = 12345;

    // Initialize with custom fail handler, user arg, and abort = false
    int res = MemTrack_Init(custom_fail_handler, &dummy_data, false, false);
    TEST_ASSERT_EQ(res, 0);

    // Call check_malloc_error with non-null pointer (should NOT trigger handler)
    void *valid_ptr = (void *)0xDEADBEEF;
    check_malloc_error(valid_ptr);
    TEST_ASSERT_EQ(g_handler_called, 0);
    TEST_ASSERT_NULL(g_handler_arg_received);

    // Call check_malloc_error with NULL pointer (SHOULD trigger handler)
    check_malloc_error(NULL);
    TEST_ASSERT_EQ(g_handler_called, 1);
    TEST_ASSERT_EQ(g_handler_arg_received, (void *)&dummy_data);

    // Call debug_check_malloc_error with NULL pointer (SHOULD trigger handler again)
    debug_check_malloc_error(NULL, __FILE__, __LINE__);
    TEST_ASSERT_EQ(g_handler_called, 2);
    TEST_ASSERT_EQ(g_handler_arg_received, (void *)&dummy_data);

    // Call debug_check_malloc_error with non-null (should NOT trigger)
    debug_check_malloc_error(valid_ptr, __FILE__, __LINE__);
    TEST_ASSERT_EQ(g_handler_called, 2);

    // Also test through actual impossible allocation size ((size_t)-1)
    void *p_fail = safe_malloc((size_t)-1);
    TEST_ASSERT_NULL(p_fail);
    TEST_ASSERT_EQ(g_handler_called, 3);

    // Cleanup: leave one node for Quit to reset init
    void *p = t_malloc(16);
    TEST_ASSERT_NOT_NULL(p);
    MemTrack_Quit();

    TEST_REPORT();
}
