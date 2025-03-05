cmake_minimum_required(VERSION 3.20)
project(ustl VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(ustl_headers INTERFACE)
add_library(ours::kernel::lib::ustl::headers ALIAS ustl_headers)
target_include_directories(ustl_headers 
INTERFACE 
    "include"
)

add_library(ustl INTERFACE)
add_library(ours::kernel::lib::ustl ALIAS ustl)
target_link_libraries(ustl
INTERFACE
    ours::kernel::lib::ustl::headers 
)