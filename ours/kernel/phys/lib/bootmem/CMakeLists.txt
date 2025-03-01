cmake_minimum_required(VERSION "3.10")
project(bootmem VERSION "${OURS_VERSION}" LANGUAGES C CXX)

add_library(bootmem_headers INTERFACE)
add_library(ours::kernel::phys::lib::bootmem::headers ALIAS bootmem_headers)
target_include_directories(bootmem_headers
INTERFACE 
    "include"
)
target_link_libraries(bootmem_headers
INTERFACE
    ours::kernel::common::headers
    ours::kernel::main::mem::headers
)

add_library(bootmem INTERFACE)
add_library(ours::kernel::phys::lib::bootmem ALIAS bootmem)
target_sources(bootmem
INTERFACE 
    "${CMAKE_CURRENT_SOURCE_DIR}/bootmem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/memblock.cpp"
)
target_link_libraries(bootmem
INTERFACE
    bootmem_headers 
)