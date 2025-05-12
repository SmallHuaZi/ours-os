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
    "descriptor.cpp"
    "faults.cpp"
    "feature.cpp"
    "gdt.S"
    "idt.cpp"
    "idt.S"
    "init.cpp"
    "mp.cpp"
    "thread.cpp"
    "syscall.cpp"
    "bootstrap.cpp"
)

add_subdirectory("mem")
add_subdirectory("irq")
add_subdirectory("entry")