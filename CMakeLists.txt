cmake_minimum_required (VERSION 3.14)

project (chord)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED True)

set (CMAKE_EXE_LINKER_FLAGS "-rdynamic")
set (CMAKE_SHARED_LINKER_FLAGS "-rdynamic")

file (GLOB SRC_FILES "chord/*.cpp")

set (CMAKE_CXX_FLAGS "-O3 -Wall")

add_executable (chord ${SRC_FILES})

target_link_libraries (chord PRIVATE
    icarus
    pthread
)
