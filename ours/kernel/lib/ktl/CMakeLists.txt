cmake_minimum_required(VERSION 3.20)

# Generic kernel tools library
project(ktl VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(ktl_headers INTERFACE)
add_library(kernel::lib::ktl::headers ALIAS ktl_headers)
target_include_directories(ktl_headers  
INTERFACE 
    "include"
)

add_library(ktl INTERFACE)
add_library(kernel::lib::ktl ALIAS ktl)
target_link_libraries(ktl
INTERFACE
    kernel::main::headers
    kernel::lib::ktl::headers
)