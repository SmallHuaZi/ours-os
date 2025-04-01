add_library(lz4_freestanding INTERFACE)
add_library(third_party::lz4_freestanding ALIAS lz4_freestanding)
target_include_directories(lz4_freestanding
INTERFACE
    "lz4"
)
target_compile_definitions(lz4_freestanding
INTERFACE
    "LZ4_FREESTANDING=1"
)