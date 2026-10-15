# Include helpers directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Determine test target to link against:
# If shared library is built, link against memtrack (shared library).
# If shared library is disabled, use an internal OBJECT library (does NOT produce a static lib .a).
if(TARGET memtrack)
    set(MEMTRACK_TEST_LIB memtrack)
else()
    add_library(memtrack_test_obj OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/../src/memtrack.c)
    target_include_directories(memtrack_test_obj PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../MemTrack
    )
    target_link_libraries(memtrack_test_obj PUBLIC Threads::Threads)
    set(MEMTRACK_TEST_LIB memtrack_test_obj)
endif()

# Helper function to register a C test target
function(add_memtrack_test target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} PRIVATE ${MEMTRACK_TEST_LIB})
    # Most tests use allocation tracking by default unless specified
    set(extra_defs ${ARGN})
    if(extra_defs)
        target_compile_definitions(${target_name} PRIVATE ${extra_defs})
    else()
        target_compile_definitions(${target_name} PRIVATE TRACK_ALLOCATIONS)
    endif()
    add_test(NAME ${target_name} COMMAND ${target_name})
endfunction()

# Core unit tests
add_memtrack_test(test_version test_version.c)
add_memtrack_test(test_lifecycle test_lifecycle.c)
add_memtrack_test(test_tracking test_tracking.c)
add_memtrack_test(test_realloc test_realloc.c)
add_memtrack_test(test_strdup test_strdup.c)
add_memtrack_test(test_auto_null test_auto_null.c)
add_memtrack_test(test_linked_list_edges test_linked_list_edges.c)
add_memtrack_test(test_fail_handler test_fail_handler.c)
add_memtrack_test(test_threads test_threads.c)
add_memtrack_test(test_calloc test_calloc.c)

# Mode tests
# test_safe_mode: no allocation macro defined (safe malloc/free mode)
add_executable(test_safe_mode test_safe_mode.c)
target_link_libraries(test_safe_mode PRIVATE ${MEMTRACK_TEST_LIB})
add_test(NAME test_safe_mode COMMAND test_safe_mode)

# test_stdlib_mode: STDLIB_ALLOCATIONS defined (standard stdlib mode)
add_executable(test_stdlib_mode test_stdlib_mode.c)
target_link_libraries(test_stdlib_mode PRIVATE ${MEMTRACK_TEST_LIB})
target_compile_definitions(test_stdlib_mode PRIVATE STDLIB_ALLOCATIONS)
add_test(NAME test_stdlib_mode COMMAND test_stdlib_mode)

# test_header_only: tests pure single-header usage (links interface target)
add_executable(test_header_only test_header_only.c)
target_link_libraries(test_header_only PRIVATE memtrack_header)
target_compile_definitions(test_header_only PRIVATE TRACK_ALLOCATIONS)
add_test(NAME test_header_only COMMAND test_header_only)

# test_compiled_lib: tests linking against precompiled shared library target without MEMTRACK_IMPLEMENTATION
if(TARGET memtrack)
    add_executable(test_compiled_lib test_compiled_lib.c)
    target_link_libraries(test_compiled_lib PRIVATE memtrack)
    target_compile_definitions(test_compiled_lib PRIVATE TRACK_ALLOCATIONS)
    add_test(NAME test_compiled_lib COMMAND test_compiled_lib)
endif()

# Unified test runner running all test modules
add_executable(test_all test_all.c)
target_link_libraries(test_all PRIVATE ${MEMTRACK_TEST_LIB})
target_compile_definitions(test_all PRIVATE TRACK_ALLOCATIONS)
add_test(NAME test_all COMMAND test_all)

# C++ test (if CXX is available)
if(MEMTRACK_HAS_CXX)
    add_executable(test_cpp test_cpp.cpp)
    target_link_libraries(test_cpp PRIVATE ${MEMTRACK_TEST_LIB})
    target_compile_definitions(test_cpp PRIVATE TRACK_ALLOCATIONS)
    add_test(NAME test_cpp COMMAND test_cpp)
endif()
