add_library(omitl_headers INTERFACE)
add_library(kernel::lib::omitl::headers ALIAS omitl_headers)
target_include_directories(omitl_headers
INTERFACE
    "include"
)
target_sources(omitl_headers
INTERFACE
    "include/omitl/macro.hpp"
    "include/omitl/obi-header.hpp"
    "include/omitl/obi-view.hpp"
    "include/omitl/obi-image.hpp"
)
target_link_libraries(omitl_headers
INTERFACE
    kernel::main::headers
    kernel::lib::arch::headers
)

add_library(omitl INTERFACE)
add_library(kernel::lib::omitl ALIAS omitl)
target_sources(omitl
INTERFACE
    "obi-header.cpp"
    "obi-type.cpp"
    "obi-view.cpp"
)
target_link_libraries(omitl
INTERFACE
    omitl_headers 
    kernel::lib::lz4
)