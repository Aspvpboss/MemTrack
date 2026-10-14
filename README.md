### MemTrack v1.2

# Documentation

This library tracks allocations and can report memory leaks; furthermore, it can also
get heap usage and automatically null pointers after freeing them and exiting the program if malloc fails.

This library does support multi-threading, read the function docs for more info

I wrote this library in a way to configure it as you want. If you just want to track allocations when debugging, then only use the normal malloc and free during runtime you can do that. Or if you want it to null pointers when freed during runtime and check for malloc failure during runtime, in exchange for a very small performance hit.

#### when compiling use the these macros to configure the library. I use gcc so I use -Dmacro_name to define a global macro when compiling

### Compiler Macros 
- #define STDLIB_ALLOCATIONS - this uses stdlib malloc and free 

- #define TRACK_ALLOCATIONS - this uses my tracking malloc and free functions 

- #define NDEBUG - this will disable printf logging

- NO MACRO defined - this doesn't track allocations, however it still uses the safe malloc and free functions which allows for auto nulling pointers and exiting if malloc failure



####  There are macros that you can configure memtrack to use your own functions instead of the default ones it uses

### Function Macros
- #define TRACK_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
- #define TRACK_MALLOC(size) malloc(size)
- #define TRACK_REALLOC(mem, newsize) realloc(mem, newsize)
- #define TRACK_FREE(ptr) free(ptr)
- #define TRACK_STRLEN(str) strlen(str)
- #define TRACK_STRCPY(dst, src) strcpy(dst, src)
- #define TRACK_STRDUP(str) strdup(str)
- #define TRACK_EXIT exit(EXIT_FAILURE)
- #define TRACK_MUTEX_TYPE pthread_mutex_t 
- #define TRACK_MUTEX_CREATE(mutex) pthread_mutex_init(&mutex, NULL)
- #define TRACK_MUTEX_DESTROY(mutex) pthread_mutex_destroy(&mutex)
- #define TRACK_MUTEX_LOCK(mutex) pthread_mutex_lock(mutex)
- #define TRACK_MUTEX_UNLOCK(mutex) pthread_mutex_unlock(mutex)

# Initializing

First, you must define the MEMTRACK_IMPLEMENTATION macro to paste the functions in, then include the memtrack header. You only need to define the MEMTRACK_IMPLEMENTATION once in one C file

    #define MEMTRACK_IMPLEMENTATION 
    #include "memtrack.h"

Secondly, in the main function, you need to call MemTrack_Init(); and use your desired configs

    MemTrack_Init(NULL, NULL, false, false);

If you are using the memory_failure_abort config, the malloc failure function you pass into the init function will run with the void* you pass into init if malloc fails

Importantly, if you're using the macro STDLIB_ALLOCATIONS, the auto_null_pointers and memory_failure_abort bools will do nothing

# Using MemTrack

Here are the main functions and macros this library provides, most of these functions support multi-threading 

## Functions


MemTrack_Init and MemTrack_Quit are the only functions that should only be called on the main thread

    int MemTrack_Init(void(*malloc_fail_handler)(void*), void *handler_arg, bool auto_null_pointers, bool memory_failure_abort);     
    // returns 1 if failure or if called more than once

    void MemTrack_Quit(); // call this after all other threads are cleaned up


    size_t check_memory_usage(); // returns size_t of the amount of bytes used in heap
    int check_memory_leak(); // returns 1 if there are tracked allocations, returns 0 if there aren't tracked allocations
    void print_tracking_info(); // prints all tracking information
    int MemTrack_GetVersion(int *major, int *minor); // gets the version of memtrack, returns 1 if failure


## Macros

These macro definitions change depending on what compiler macro you define, but these are the generic args you give them. All of these macros support multi-threading

    void* t_malloc(size_t size);

    void* t_realloc(void *mem, size_t size);
    
    void t_free(void *mem); 

    char* t_strdup(const char *string);


# Example Program

```c
#define MEMTRACK_IMPLEMENTATION
#include "memtrack.h"
#include <pthread.h>

void malloc_failure(void *data){
    printf("\ndub\n");
}


void* thread_func(void *arg){
    return t_malloc(100);
}



int main(void){

    MemTrack_Init(malloc_failure, NULL, true, true);

    pthread_t thread;

    if(pthread_create(&thread, NULL, thread_func, NULL)){
        return 1;
    } 

    int *array = t_malloc(999999);

    if(!array)
        printf("array is null\n");


    t_free(array);

    if(!array){

        printf("is null after free\n");

    } else{

        printf("isn't null after free\n");

    }
   
    

    void *result = NULL;
    pthread_join(thread, &result);

    if(check_memory_leak())
        print_tracking_info();
    
    MemTrack_Quit();

    return 0;
}
```

# Building and Testing with CMake

MemTrack provides modern CMake targets and CTest integration.

### Build
```bash
cmake -B build -S .
cmake --build build
```

### Run Tests
```bash
ctest --test-dir build --output-on-failure
```

Or run the unified test suite executable directly:
```bash
./build/tests/test_all
```

### Integration in CMake Projects
MemTrack provides both header-only and precompiled library targets:

```cmake
# Header-only usage:
target_link_libraries(your_target PRIVATE MemTrack::memtrack)

# Or precompiled library usage:
target_link_libraries(your_target PRIVATE MemTrack::memtrack_lib)
```

### CMake Options
- `MEMTRACK_BUILD_TESTS` (Default: `ON`) - Build comprehensive unit tests
- `MEMTRACK_BUILD_EXAMPLES` (Default: `ON`) - Build example binaries (`main.c` / `main.cpp`)
- `MEMTRACK_WARNINGS_AS_ERRORS` (Default: `ON`) - Treat warnings as errors (`-Werror`)
- `BUILD_SHARED_LIBS` (Default: `OFF`) - Build shared library instead of static