add_library(omi_headers INTERFACE)
add_library(kernel::lib::omi::headers ALIAS omi_headers)
target_include_directories(omi_headers
INTERFACE
    "include"
)
target_sources(omi_headers
INTERFACE
    "include/omi/asm.hpp"
    "include/omi/defines.hpp"
    "include/omi/header.hpp"
)
target_link_libraries(omi_headers
INTERFACE
    kernel::main::headers
    kernel::lib::arch::headers
)

add_library(omi)
add_library(kernel::lib::omi ALIAS omi)
target_sources(omi
PRIVATE
    "obi.cpp"
)
target_link_libraries(omi
PUBLIC
    omi_headers 
)