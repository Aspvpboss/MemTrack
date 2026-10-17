# MemTrack v1.4.0

A thread-safe, configurable C memory allocation tracker and leak detector. MemTrack can be used as an **stb-style single-header library** (`memtrack.h`) or built and linked as a **shared library** via CMake.

## Changes


## Documentation

### Compiler Macros
Pass one of these flags to your compiler (e.g., `-DTRACK_ALLOCATIONS`) to configure MemTrack:

- `#define STDLIB_ALLOCATIONS` - uses standard library `malloc` and `free` directly.
- `#define TRACK_ALLOCATIONS` - uses MemTrack's tracking `malloc` and `free` functions.
- `#define NDEBUG` - disables print logging.
- **NO MACRO defined** - doesn't track allocations, but still uses the safe `malloc` and `free` wrappers (allowing for auto-nulling pointers and exiting on `malloc` failure).

### Function Macros
These are the macros you can configure to change what underlying functions MemTrack uses *(only works if you recompile the shared library or use the single-header library)*:

```c
#define TRACK_PRINTF(fmt, ...)     fprintf(stderr, fmt, ##__VA_ARGS__)
#define TRACK_MALLOC(size)         malloc(size)
#define TRACK_REALLOC(mem, newsize) realloc(mem, newsize)
#define TRACK_FREE(ptr)            free(ptr)
#define TRACK_STRLEN(str)          strlen(str)
#define TRACK_STRCPY(dst, src)     strcpy(dst, src)
#define TRACK_STRDUP(str)          strdup(str)
#define TRACK_EXIT                 exit(EXIT_FAILURE)
#define TRACK_MUTEX_TYPE           pthread_mutex_t
#define TRACK_MUTEX_CREATE(mutex)  pthread_mutex_init(&mutex, NULL)
#define TRACK_MUTEX_DESTROY(mutex) pthread_mutex_destroy(&mutex)
#define TRACK_MUTEX_LOCK(mutex)    pthread_mutex_lock(mutex)
#define TRACK_MUTEX_UNLOCK(mutex)  pthread_mutex_unlock(mutex)
```

### Using MemTrack

If using `memtrack.h` as a single-header library, define `#define MEMTRACK_IMPLEMENTATION` in **one** C file before including `"memtrack.h"`.

`MemTrack_Init` and `MemTrack_Quit` are the only functions that must be called exclusively on the main thread:

```c
// Initializes MemTrack. Returns 1 on failure or if called more than once.
// Note: If compiled with STDLIB_ALLOCATIONS, auto_null_pointers and memory_failure_abort have no effect.
int MemTrack_Init(void (*malloc_fail_handler)(void*), void *handler_arg, bool auto_null_pointers, bool memory_failure_abort);

// Cleans up internal tracking resources. Call after all other threads have terminated.
void MemTrack_Quit(void);

// Returns current tracked heap usage in bytes.
size_t MemTrack_memory_usage(void);

// Returns 1 if unfreed tracked allocations exist, or 0 if no leaks are present.
int MemTrack_is_memory_leak(void);

// Outputs detailed allocation and leak tracking info.
void MemTrack_tracking_info(void);

// Populates major, minor, and micro version numbers. Returns 1 on failure.
int MemTrack_GetVersion(int *major, int *minor, int *micro);
```

### Allocation Macros

These macro definitions change depending on which compiler macro you define, but they take these generic arguments. All of these macros support multi-threading:

```c
void* t_malloc(size_t size);
void* t_realloc(void *mem, size_t size);
void  t_free(void *mem);
char* t_strdup(const char *string);
```

## Example Program

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
    } else {
        printf("isn't null after free\n");
    }

    void *result = NULL;
    pthread_join(thread, &result);

    if(MemTrack_is_memory_leak())
        MemTrack_tracking_info();

    MemTrack_Quit();
    return 0;
}
```

## Building and Testing with CMake

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

### Install System-Wide (or User-Wide)
To install system-wide on Linux (e.g. Fedora):
```bash
sudo cmake --install build
sudo ldconfig  # update dynamic linker cache for libmemtrack.so
```



Or install to user environment without root (`~/.local`):
```bash
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build build
cmake --install build
```
for other operating systems, just put the shared lib, header files, and cmake files where they need to go


### Integration in CMake Projects (Shared Library Users)
Installing into your environment is designed for **shared library users**. Once installed, other CMake projects on Fedora automatically detect MemTrack:

```cmake
find_package(MemTrack REQUIRED)

# Dynamically links against libmemtrack.so (no MEMTRACK_IMPLEMENTATION needed in consumer code):
target_link_libraries(your_target PRIVATE MemTrack::memtrack)
```

*(Non-shared users do not need to install anything or use `find_package` — they just copy `memtrack.h` into their project and define `MEMTRACK_IMPLEMENTATION` in one C file).*

### CMake Options
- `MEMTRACK_BUILD_SHARED` (Default: `ON`) - Build MemTrack shared library (`.so` / `.dll`). When `OFF`, no compiled library is built.
- `MEMTRACK_BUILD_TESTS` (Default: `ON`) - Build comprehensive unit tests
- `MEMTRACK_BUILD_EXAMPLES` (Default: `ON`) - Build example binaries (`main.c` / `main.cpp`)
- `MEMTRACK_WARNINGS_AS_ERRORS` (Default: `ON`) - Treat warnings as errors (`-Werror`)