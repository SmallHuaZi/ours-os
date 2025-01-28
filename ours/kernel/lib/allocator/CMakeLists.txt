cmake_minimum_required(VERSION "3.10")
project(allocator VERSION "${OURS_VERSION}" LANGUAGES C CXX)

add_library(memblock)
add_library(ours::kernel::lib::memblock ALIAS memblock)
target_sources(memblock
    PRIVATE 
        "memblock.cpp"
)
target_include_directories(memblock
    PUBLIC 
        "include"
)
target_link_libraries(memblock
    PUBLIC
        ours::kernel::lib::ustl
    PRIVATE
        ours::kernel::lib::logz4
)