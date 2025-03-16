cmake_minimum_required(VERSION 3.20)
project(logz4 VERSION "${OURS_VERSION}" LANGUAGES CXX)

add_library(logz4_headers INTERFACE)
add_library(kernel::lib::logz4::headers ALIAS logz4_headers)
target_include_directories(logz4_headers 
INTERFACE 
    "include"
)
target_link_libraries(logz4_headers
INTERFACE
    kernel::main::headers
)

add_library(logz4)
add_library(kernel::lib::logz4 ALIAS logz4)
target_sources(logz4 
PUBLIC
    "log.cpp"
    "init.cpp"
    "filter.cpp"
)
target_link_libraries(logz4
PUBLIC
    logz4_headers
)