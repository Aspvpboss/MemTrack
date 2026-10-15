#include "memtrack.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

int main() {
    std::printf("=== Running test_cpp (C++ compatibility test) ===\n");

    int res = MemTrack_Init(NULL, NULL, true, false);
    if (res != 0) {
        std::fprintf(stderr, "MemTrack_Init failed\n");
        return 1;
    }

    int major = 0, minor = 0, micro = 0;
    if (MemTrack_GetVersion(&major, &minor, &micro) != 0 || major != 1 || minor != 3 || micro != 1) {
        std::fprintf(stderr, "MemTrack_GetVersion failed\n");
        return 1;
    }

    // Allocate array in C++
    int *array = static_cast<int *>(t_malloc(100 * sizeof(int)));
    if (!array) {
        std::fprintf(stderr, "t_malloc failed\n");
        return 1;
    }

    for (int i = 0; i < 100; i++) {
        array[i] = i * 2;
    }

    if (MemTrack_is_memory_leak() != 1) {
        std::fprintf(stderr, "MemTrack_is_memory_leak expected 1\n");
        return 1;
    }

    // Realloc in C++
    array = static_cast<int *>(t_realloc(array, 200 * sizeof(int)));
    if (!array) {
        std::fprintf(stderr, "t_realloc failed\n");
        return 1;
    }

    if (array[50] != 100) {
        std::fprintf(stderr, "realloc data mismatch\n");
        return 1;
    }

    t_free(array);
    if (array != NULL) {
        std::fprintf(stderr, "t_free auto-null failed\n");
        return 1;
    }

    if (MemTrack_is_memory_leak() != 0 || MemTrack_memory_usage() != 0) {
        std::fprintf(stderr, "leaks remain\n");
        return 1;
    }

    // String duplication in C++
    char *dup = t_strdup("C++ strdup test");
    if (!dup || std::strcmp(dup, "C++ strdup test") != 0) {
        std::fprintf(stderr, "t_strdup failed\n");
        return 1;
    }

    t_free(dup);
    if (dup != NULL) {
        std::fprintf(stderr, "t_free dup auto-null failed\n");
        return 1;
    }

    std::printf("Result: PASSED\n");
    return 0;
}
