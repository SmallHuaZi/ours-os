cmake_minimum_required(VERSION 3.20)

project(arch VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(arch_headers INTERFACE)
add_library(ours::kernel::lib::arch::headers ALIAS arch_headers)
target_include_directories(arch_headers
INTERFACE
    "include"
)

add_library(arch)
add_library(ours::kernel::lib::arch ALIAS arch)
target_link_libraries(arch
INTERFACE
    arch_headers
PRIVATE
    ours::kernel::common::headers
    ustl
)

add_subdirectory(x86)
add_subdirectory(generic)