cmake_minimum_required(VERSION 3.20)

# Generic kernel tools library
project(gktl VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(gktl)
add_library(ours::kernel::lib::gktl ALIAS gktl)

target_include_directories(gktl 
PUBLIC "include"
)

target_sources(gktl
PUBLIC
    "include/gktl/canary.hpp"
    "include/gktl/counter.hpp"
    "include/gktl/cpu_local.hpp"
    "include/gktl/init_hook.hpp"
    "include/gktl/range.hpp"

PRIVATE
    "cpu_local.cpp"
    "static_objects.cpp"
)

target_link_libraries(gktl
PUBLIC
    ours::kernel::common::headers
)