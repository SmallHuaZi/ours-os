cmake_minimum_required(VERSION 3.20)

project(arch VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(arch)
add_library(ours::kernel::lib::arch ALIAS arch)

target_link_libraries(arch
PRIVATE
    ours::kernel::common::headers
    ustl
)

add_subdirectory(x86)
add_subdirectory(generic)