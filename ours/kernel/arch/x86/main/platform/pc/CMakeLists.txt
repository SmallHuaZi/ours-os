target_sources(kernel_main_arch
INTERFACE
    "init.cpp"
    "memory.cpp"
    "interrupt.cpp"
)

target_link_libraries(kernel_main_arch
INTERFACE
    kernel::lib::acpi
)