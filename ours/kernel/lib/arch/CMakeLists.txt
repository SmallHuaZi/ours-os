cmake_minimum_required(VERSION 3.20)

project(libarch VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(libarch_headers INTERFACE)
add_library(kernel::lib::arch::headers ALIAS libarch_headers)
target_include_directories(libarch_headers
INTERFACE
    "include"
)
target_link_libraries(libarch_headers
INTERFACE
    # This is an error dependency.
    kernel::main::headers
)

add_library(libarch)
add_library(kernel::lib::arch ALIAS libarch)
target_link_libraries(libarch
PUBLIC
    libarch_headers
)

add_subdirectory("tests")