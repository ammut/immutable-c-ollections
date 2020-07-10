cmake_minimum_required(VERSION 3.10)
project(ico C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(GCC_COMPILE_FLAGS "-Wall -Wextra -pedantic -Wcast-align -Wswitch-enum -Wswitch-default -Winit-self")
if(CMAKE_BUILD_TYPE MATCHES Release)
    # nothing yet
else()
    # Debug build
    set(CHAMP_VERBOSITY 4)
    set(GCC_COMPILE_FLAGS "${GCC_COMPILE_FLAGS} -DCHAMP_VERBOSITY=${CHAMP_VERBOSITY} --coverage")
    set(GXX_COMPILE_FLAGS "${GXX_COMPILE_FLAGS} -DCHAMP_VERBOSITY=${CHAMP_VERBOSITY} --coverage")
    set(GCC_LINK_FLAGS "${GCC_LINK_FLAGS} --coverage")
endif()


if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using GCC
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COMPILE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GXX_COMPILE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_LINK_FLAGS}")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    # using Intel C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # using Visual Studio C++
endif()

include_directories(.)

add_library(champ STATIC champ.c champ_fns.c)

add_library(stm_rc STATIC stm_rc.c)

add_subdirectory(Catch_tests)
