cmake_minimum_required(VERSION 3.20)
project(ours-os 
    VERSION 0.0.1 
    LANGUAGES C CXX
    DESCRIPTION "ours is a microkernel"
)

enable_language(ASM)

set(USTL_VERSION ${PROJECT_VERSION})
set(USTL_SOURCE_DIR ${PROJECT_SOURCE_DIR})

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/scripts/cmake)

include(CflagsTargets)
include(MakeImage)
include(PreDefines)

add_subdirectory("third_party")
add_subdirectory("ours")