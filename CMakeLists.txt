cmake_minimum_required(VERSION 4.2)
set(PRJ "varint")
project(${PRJ} VERSION 0.1.0 LANGUAGES C CXX)


# detect platform: windows/linux
set(PLATF "None")
if (WIN32)
    set(PLATF "Windows")
endif()
if (UNIX)
    set(PLATF "Unix")
endif()

# Check we run on known platform
if(PLATF STREQUAL "None")
    message(STATUS "Run on unkonwn platform. Expected: windows or unix")
endif()

set(SOURCES 
  src/main.cpp
  src/compressor.h
  src/compressor.cpp
  src/duration_meter.cpp
  src/duration_meter.h)

add_executable(${PRJ} ${SOURCES})

# print current compiler
message(STATUS "Current c++ compiler is: ${CMAKE_CXX_COMPILER_ID}")

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set_property(TARGET ${PRJ} PROPERTY CXX_STANDARD 17)
else()
    set_property(TARGET ${PRJ} PROPERTY CXX_STANDARD 23)
endif()

set(PLATFORM_COMMON_FLAGS
        # minumum export symbol table size
        -fvisibility-inlines-hidden
        # warning when float point values are compared with ==
        -Wfloat-equal
        # warning when floating point value converts to an int implicitly
        -Wfloat-conversion
        # use strict c++ standard
        -Wpedantic
        # enable all warnings
        -Wall
        # additional warnings like unused parameters
        -Wextra
        # warning if object is not initialize
        -Wuninitialized
        # non used variable, function, etc
        -Wunused
        # warn when assign signed int to unsigned
        -Wsign-conversion 
        # warn if float is implicitly promoted to double
        -Wdouble-promotion
        # enable reorder math operations for better performance
        -funsafe-math-optimizations
        # enable heuristic to predict branches
        -fguess-branch-probability)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(PLATFORM_COMMON_FLAGS "${PLATFORM_COMMON_FLAGS}" -g -O0)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    # -g to save debugging information
    set(PLATFORM_COMMON_FLAGS "${PLATFORM_COMMON_FLAGS}" -g -O2)
else()
    # do not apply -Ofast (== -O3 -ffast-math)
    set(PLATFORM_COMMON_FLAGS "${PLATFORM_COMMON_FLAGS}" -O3)
endif()

add_compile_options(${PLATFORM_COMMON_FLAGS})

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_features(${PRJ} PRIVATE cxx_std_17)
else()
    target_compile_features(${PRJ} PRIVATE cxx_std_23)
endif()
