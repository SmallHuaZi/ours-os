cmake_minimum_required(VERSION 3.20)

# Generic kernel tools library
project(gktl VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(gktl_headers INTERFACE)
add_library(ours::kernel::lib::gktl::headers ALIAS gktl_headers)
target_sources(gktl_headers
INTERFACE
    "include/gktl/canary.hpp"
    "include/gktl/counter.hpp"
    "include/gktl/init_hook.hpp"
    "include/gktl/range.hpp"
)
target_include_directories(gktl_headers
INTERFACE "include"
)

add_library(gktl INTERFACE)
add_library(ours::kernel::lib::gktl ALIAS gktl)
target_sources(gktl
INTERFACE
    "init_hook.cpp"
    "static_objects.cpp"
    "cpp_dependencies.cpp"
)
target_link_libraries(gktl
INTERFACE
    gktl_headers
    ours::kernel::common::headers
)