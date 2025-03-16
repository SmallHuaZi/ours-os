cmake_minimum_required(VERSION 3.20)

# Generic kernel tools library
project(gktl VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(gktl_headers INTERFACE)
add_library(kernel::lib::gktl::headers ALIAS gktl_headers)
target_sources(gktl_headers
INTERFACE
    "include/gktl/canary.hpp"
    "include/gktl/counter.hpp"
    "include/gktl/init_hook.hpp"
    "include/gktl/range.hpp"
)
target_include_directories(gktl_headers
INTERFACE 
    "include"
)


add_library(gktl_goi INTERFACE)
add_library(kernel::lib::gktl::goi ALIAS gktl_goi)
target_sources(gktl_goi
INTERFACE
    "static_objects.cpp"
)
target_link_libraries(gktl_goi
INTERFACE
    gktl_headers
    kernel::main::headers
)

add_library(gktl INTERFACE)
add_library(kernel::lib::gktl ALIAS gktl)
target_sources(gktl
INTERFACE
    "init_hook.cpp"
)
target_link_libraries(gktl
INTERFACE
    gktl_headers
    kernel::main::headers
    kernel::lib::gktl::goi
)