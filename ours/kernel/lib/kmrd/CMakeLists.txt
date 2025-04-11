cmake_minimum_required(VERSION 3.20)

# Kernel memory reclaim damon 
project(libkmrd VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(libkmrd_headers INTERFACE)
add_library(kernel::lib::kmrd::headers ALIAS libkmrd_headers)
target_include_directories(libkmrd_headers 
INTERFACE
    "include"
)
target_link_libraries(libkmrd_headers 
INTERFACE
    kernel::main::headers
)

add_library(libkmrd INTERFACE)
add_library(kernel::lib::kmrd ALIAS libkmrd)
target_link_libraries(libkmrd
INTERFACE
    kernel::lib::kmrd::headers
)
target_sources(libkmrd
INTERFACE
    "damon.cpp"
)