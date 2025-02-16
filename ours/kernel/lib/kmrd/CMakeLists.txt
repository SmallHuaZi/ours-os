cmake_minimum_required(VERSION 3.20)

# Kernel memory reclaim damon 
project(kmrd VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(kmrd_headers INTERFACE)
add_library(ours::kernel::lib::kmrd::headers ALIAS kmrd_headers)
target_include_directories(kmrd_headers 
INTERFACE
    "include"
)
target_link_libraries(kmrd_headers 
INTERFACE
    ours::kernel::task::headers
    ours::kernel::common::headers
)

add_library(kmrd)
add_library(ours::kernel::lib::kmrd ALIAS kmrd)
target_link_libraries(kmrd
PUBLIC
    ours::kernel::lib::kmrd::headers
)
target_sources(kmrd
PRIVATE
    "damon.cpp"
)