cmake_minimum_required(VERSION 3.20)
project(cnl_wrapper VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(cnl_wrapper INTERFACE)
add_library(kernel::lib::cnl ALIAS cnl_wrapper)
target_include_directories(cnl_wrapper 
INTERFACE 
    "include"
)
target_link_libraries(cnl_wrapper 
INTERFACE
    kernel::main::headers
    Cnl
)
