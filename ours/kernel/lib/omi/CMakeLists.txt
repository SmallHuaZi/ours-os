add_library(omi_headers INTERFACE)
add_library(ours::kernel::lib::omi::headers ALIAS omi_headers)
target_include_directories(omi_headers
INTERFACE
    "include"
)
target_link_libraries(omi_headers
INTERFACE
    ours::kernel::common::headers
)

add_library(omi INTERFACE)
add_library(ours::kernel::lib::omi ALIAS omi)
target_link_libraries(omi
INTERFACE
    omi_headers 
)