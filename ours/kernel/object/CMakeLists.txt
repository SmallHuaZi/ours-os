add_library(ours_kernel_object_headers INTERFACE)
add_library(ours::kernel::object::headers ALIAS ours_kernel_object_headers)
target_sources(ours_kernel_object_headers
INTERFACE
    "include/ours/object/kernel_object.hpp"
    "include/ours/object/thread_dispatcher.hpp"
)
target_include_directories(ours_kernel_object_headers
INTERFACE
    "include"
)

add_library(ours_kernel_object INTERFACE)
add_library(ours::kernel::object ALIAS ours_kernel_object)
target_sources(ours_kernel_object
INTERFACE
    "kernel_object.cpp"
    "thread_dispatcher.cpp"
)

target_link_libraries(ours_kernel_object
INTERFACE
    ours::kernel::common::headers
    ours::kernel::object::headers
    ours::kernel::lib::logz4
)