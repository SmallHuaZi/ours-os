cmake_minimum_required(VERSION 3.20)

# Generic kernel tools library
project(heap VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(heap_headers INTERFACE)
add_library(kernel::lib::heap::headers ALIAS heap_headers)
target_include_directories(heap_headers  
INTERFACE 
    "include"
)

add_library(heap INTERFACE)
add_library(kernel::lib::heap ALIAS heap)
target_sources(heap
INTERFACE
    "scope.cpp"
)
target_link_libraries(heap
INTERFACE
    kernel::main::headers
    kernel::lib::ustl::headers
    kernel::lib::ktl::headers
    kernel::lib::ktl::headers
    kernel::lib::heap::headers
)