add_library(lz4_headers INTERFACE)
add_library(ours::kernel::lib::lz4::headers ALIAS lz4_headers)
target_include_directories(lz4_headers
INTERFACE
    "include"
)

add_library(lz4)
add_library(ours::kernel::lib::lz4 ALIAS lz4)
target_sources(lz4
PRIVATE
    "lz4.cpp"
)
target_link_libraries(lz4
PRIVATE
    third_party::lz4_freestanding
PUBLIC
    lz4_headers 
    ours::kernel::common::headers
)