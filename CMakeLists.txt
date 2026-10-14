cmake_minimum_required(VERSION 3.16)

project(MemTrack
    VERSION 1.3.0
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
option(MEMTRACK_BUILD_SHARED "Build MemTrack shared library (.so / .dll)" ON)

# Standard installation paths
include(GNUInstallDirs)

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

# 1. Header-only INTERFACE target (for examples and header-only usage)
add_library(memtrack_header INTERFACE)
add_library(MemTrack::memtrack_header ALIAS memtrack_header)

target_include_directories(memtrack_header INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/MemTrack>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(memtrack_header INTERFACE Threads::Threads)

# 2. Precompiled SHARED library target (Primary library for shared-lib users)
if(MEMTRACK_BUILD_SHARED)
    add_library(memtrack SHARED src/memtrack.c)
    add_library(MemTrack::memtrack ALIAS memtrack)
    add_library(MemTrack::memtrack_shared ALIAS memtrack)
    add_library(MemTrack::memtrack_lib ALIAS memtrack)

    target_include_directories(memtrack PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/MemTrack>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    target_link_libraries(memtrack PUBLIC Threads::Threads)
    target_compile_definitions(memtrack
        PUBLIC
            MEMTRACK_DLL
        PRIVATE
            MEMTRACK_BUILD_DLL
    )
    set_target_properties(memtrack PROPERTIES
        OUTPUT_NAME memtrack
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
    )
else()
    # When shared library is disabled, alias MemTrack::memtrack to the header-only target
    add_library(MemTrack::memtrack ALIAS memtrack_header)
endif()

# -----------------------------------------------------------------------------
# Example Executables (corresponds to Makefile's build and build_cpp)
# -----------------------------------------------------------------------------
if(MEMTRACK_BUILD_EXAMPLES)
    # C Example (main.c)
    add_executable(memtrack_example_c main.c)
    target_link_libraries(memtrack_example_c PRIVATE memtrack_header)
    target_compile_definitions(memtrack_example_c PRIVATE TRACK_ALLOCATIONS)

    # C++ Example (main.cpp)
    if(MEMTRACK_HAS_CXX)
        add_executable(memtrack_example_cpp main.cpp)
        target_link_libraries(memtrack_example_cpp PRIVATE memtrack_header)
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
install(DIRECTORY MemTrack/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/MemTrack
    FILES_MATCHING PATTERN "*.h"
)

install(FILES MemTrack/memtrack.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set(MEMTRACK_TARGETS_TO_INSTALL memtrack_header)
if(MEMTRACK_BUILD_SHARED)
    list(APPEND MEMTRACK_TARGETS_TO_INSTALL memtrack)
endif()

install(TARGETS ${MEMTRACK_TARGETS_TO_INSTALL}
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

# Package Configuration for find_package(MemTrack)
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/MemTrackConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/MemTrackConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/MemTrackConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/MemTrack"
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MemTrackConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MemTrackConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MemTrack
)
