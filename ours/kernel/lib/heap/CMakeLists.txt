cmake_minimum_required(VERSION 3.20)

# Generic kernel tools library
project(heap VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(heap_headers INTERFACE)
add_library(ours::kernel::lib::heap::headers ALIAS heap_headers)
target_include_directories(heap_headers  
INTERFACE 
    "include"
)

add_library(heap)
add_library(ours::kernel::lib::heap ALIAS heap)
target_sources(heap
PRIVATE
    "scope.cpp"
)
target_link_libraries(heap
PRIVATE
    ours::kernel::common::headers
    ours::kernel::main::mem::headers
    ours::kernel::lib::ktl::headers
PUBLIC
    ours::kernel::lib::heap::headers
)