cmake_minimum_required(VERSION 3.20)

project(multiboot VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(multiboot INTERFACE)
add_library(ours::kernel::lib::multiboot  ALIAS multiboot)

target_include_directories(multiboot  
    INTERFACE include
)

target_link_libraries(multiboot
    INTERFACE ours::kernel::common::headers
)