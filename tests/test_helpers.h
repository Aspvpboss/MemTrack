#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

static int g_test_count = 0;
static int g_test_failures = 0;

#define TEST_ASSERT(cond) do { \
    g_test_count++; \
    if (!(cond)) { \
        g_test_failures++; \
        fprintf(stderr, "  [FAIL] %s:%d: Assertion failed: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while(0)

#define TEST_ASSERT_TRUE(cond) TEST_ASSERT(cond)
#define TEST_ASSERT_FALSE(cond) TEST_ASSERT(!(cond))

#define TEST_ASSERT_EQ(actual, expected) do { \
    g_test_count++; \
    if ((actual) != (expected)) { \
        g_test_failures++; \
        fprintf(stderr, "  [FAIL] %s:%d: %s == %s failed (actual: %lld, expected: %lld)\n", \
                __FILE__, __LINE__, #actual, #expected, (long long)(actual), (long long)(expected)); \
    } \
} while(0)

#define TEST_ASSERT_NE(actual, not_expected) do { \
    g_test_count++; \
    if ((actual) == (not_expected)) { \
        g_test_failures++; \
        fprintf(stderr, "  [FAIL] %s:%d: %s != %s failed (both are %lld)\n", \
                __FILE__, __LINE__, #actual, #not_expected, (long long)(actual)); \
    } \
} while(0)

#define TEST_ASSERT_NULL(ptr) do { \
    g_test_count++; \
    if ((ptr) != NULL) { \
        g_test_failures++; \
        fprintf(stderr, "  [FAIL] %s:%d: Expected NULL for %s, got %p\n", \
                __FILE__, __LINE__, #ptr, (void*)(ptr)); \
    } \
} while(0)

#define TEST_ASSERT_NOT_NULL(ptr) do { \
    g_test_count++; \
    if ((ptr) == NULL) { \
        g_test_failures++; \
        fprintf(stderr, "  [FAIL] %s:%d: Expected non-NULL for %s\n", \
                __FILE__, __LINE__, #ptr); \
    } \
} while(0)

#define TEST_ASSERT_STR_EQ(actual, expected) do { \
    g_test_count++; \
    const char *act = (const char *)(actual); \
    const char *exp = (const char *)(expected); \
    if (!act || !exp || strcmp(act, exp) != 0) { \
        g_test_failures++; \
        fprintf(stderr, "  [FAIL] %s:%d: String mismatch: actual \"%s\", expected \"%s\"\n", \
                __FILE__, __LINE__, act ? act : "(null)", exp ? exp : "(null)"); \
    } \
} while(0)

#define TEST_REPORT() do { \
    printf("Passed: %d/%d assertions\n", g_test_count - g_test_failures, g_test_count); \
    if (g_test_failures > 0) { \
        fprintf(stderr, "Result: FAILED (%d failures)\n", g_test_failures); \
        return 1; \
    } \
    printf("Result: PASSED\n"); \
    return 0; \
} while(0)

#endif /* TEST_HELPERS_H */
