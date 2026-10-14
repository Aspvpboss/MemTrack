cmake_minimum_required(VERSION 3.16)

project(MemTrack
    VERSION 1.2.0
    DESCRIPTION "Lightweight C/C++ memory allocation tracking and leak detection library"
    LANGUAGES C
)

# Optional C++ support
include(CheckLanguage)
check_language(CXX)
if(CMAKE_CXX_COMPILER)
    enable_language(CXX)
    set(MEMTRACK_HAS_CXX TRUE)
else()
    set(MEMTRACK_HAS_CXX FALSE)
    message(STATUS "No C++ compiler found; C++ targets will be skipped.")
endif()

# Configuration Options
option(MEMTRACK_BUILD_TESTS "Build MemTrack test suite" ON)
option(MEMTRACK_BUILD_EXAMPLES "Build MemTrack example executables" ON)
option(MEMTRACK_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(BUILD_SHARED_LIBS "Build shared libraries instead of static" OFF)

# Compiler Warning Flags (from Makefile reference: -Wall -Werror -Wpedantic)
if(MSVC)
    add_compile_options(/W4)
    if(MEMTRACK_WARNINGS_AS_ERRORS)
        add_compile_options(/WX)
    endif()
else()
    add_compile_options(-Wall -Wpedantic)
    if(MEMTRACK_WARNINGS_AS_ERRORS)
        add_compile_options(-Werror)
    endif()
endif()

# Threading dependency (POSIX threads)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# -----------------------------------------------------------------------------
# Library Targets
# -----------------------------------------------------------------------------

# 1. Header-only INTERFACE target
add_library(memtrack INTERFACE)
add_library(MemTrack::memtrack ALIAS memtrack)

target_include_directories(memtrack INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/MemTrack>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(memtrack INTERFACE Threads::Threads)

# 2. Precompiled STATIC/SHARED library target
add_library(memtrack_lib src/memtrack.c)
add_library(MemTrack::memtrack_lib ALIAS memtrack_lib)

target_include_directories(memtrack_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/MemTrack>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(memtrack_lib PUBLIC Threads::Threads)

# -----------------------------------------------------------------------------
# Example Executables (corresponds to Makefile's build and build_cpp)
# -----------------------------------------------------------------------------
if(MEMTRACK_BUILD_EXAMPLES)
    # C Example (main.c)
    add_executable(memtrack_example_c main.c)
    target_link_libraries(memtrack_example_c PRIVATE memtrack)
    target_compile_definitions(memtrack_example_c PRIVATE TRACK_ALLOCATIONS)

    # C++ Example (main.cpp)
    if(MEMTRACK_HAS_CXX)
        add_executable(memtrack_example_cpp main.cpp)
        target_link_libraries(memtrack_example_cpp PRIVATE memtrack)
        target_compile_definitions(memtrack_example_cpp PRIVATE TRACK_ALLOCATIONS)
    endif()
endif()

# -----------------------------------------------------------------------------
# Tests & CTest Integration
# -----------------------------------------------------------------------------
if(MEMTRACK_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# -----------------------------------------------------------------------------
# Installation and Export
# -----------------------------------------------------------------------------
include(GNUInstallDirs)

install(DIRECTORY MemTrack/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/MemTrack
    FILES_MATCHING PATTERN "*.h"
)

install(FILES MemTrack/memtrack.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS memtrack memtrack_lib
    EXPORT MemTrackTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT MemTrackTargets
    FILE MemTrackTargets.cmake
    NAMESPACE MemTrack::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MemTrack
)
