target_include_directories(kernel_main_headers
INTERFACE
    "include"
)

add_library(kernel_main_arch INTERFACE)
add_library(kernel::main::arch ALIAS kernel_main_arch)
target_link_libraries(kernel_main_arch
INTERFACE
    kernel::main::headers
)

target_sources(kernel_main_arch
INTERFACE
    # Sources
    "start.S"
    "gdt.S"
    "idt.S"
    "idt.cpp"
    "descriptor.cpp"
    "faults.cpp"
    "feature.cpp"
    "mp.cpp"
    "init.cpp"
    "thread.cpp"
)

add_subdirectory("mem")
add_subdirectory("irq")
add_subdirectory("entry")