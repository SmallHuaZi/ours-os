add_library(kernel_object_headers INTERFACE)
add_library(kernel::object::headers ALIAS kernel_object_headers)
target_sources(kernel_object_headers
INTERFACE
    "include/ours/object/dispatcher.hpp"
    "include/ours/object/thread_dispatcher.hpp"
)
target_include_directories(kernel_object_headers
INTERFACE
    "include"
)

add_library(kernel_object INTERFACE)
add_library(kernel::object ALIAS kernel_object)
target_sources(kernel_object
INTERFACE
    "kernel_object.cpp"
    "thread_dispatcher.cpp"
)

target_link_libraries(kernel_object
INTERFACE
    kernel::main::headers
    kernel::object::headers
    kernel::lib::logz4
)