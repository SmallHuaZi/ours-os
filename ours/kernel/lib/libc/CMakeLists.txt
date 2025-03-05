cmake_minimum_required(VERSION 3.20)
project(libc VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(libc INTERFACE)
add_library(ours::kernel::lib::libc ALIAS libc)
target_sources(libc 
INTERFACE
    "string.cpp"
    "cpp_dependencies.cpp"
)