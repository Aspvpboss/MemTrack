# DEPRECIATED


GCC = gcc

INCLUDE_LIB_TEST = -Ilib/include
INCLUDE_LIB = -IMemTrack
SRC_LIB = lib/src/*.c


FLAGS = -Wall -Werror -Wpedantic
OUTPUT = ./build/out_c.exe



default: build_test run

c_all: build run



.PHONY: build_test build run c_all

build_test:  
	@${GCC} main.c lib/src/memtrack.c -o ${OUTPUT} ${INCLUDE_LIB_TEST} -DTRACK_ALLOCATIONS  ${FLAGS}  

build:  
	@${GCC} main.c -o ${OUTPUT} ${INCLUDE_LIB} -DTRACK_ALLOCATIONS  ${FLAGS}  

run:
	@${OUTPUT}




G++ = g++
OUTPUT_CPP = build/out_cpp.exe


cpp_all: build_cpp run_cpp
cpp_test: build_test_cpp run_cpp

build_test_cpp:
	@${G++} main.cpp lib/src/memtrack.c -o ${OUTPUT_CPP} ${INCLUDE_LIB_TEST} -DTRACK_ALLOCATIONS ${FLAGS}

build_cpp:
	@${G++} main.cpp -o ${OUTPUT_CPP} ${INCLUDE_LIB} -DTRACK_ALLOCATIONS ${FLAGS}

run_cpp:
	@${OUTPUT_CPP}

