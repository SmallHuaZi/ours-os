message("Enter kerenl/arch/x86")
target_link_libraries(ours_kernel_main_arch
INTERFACE
    ours::kernel::main::arch::headers
    ours::kernel::main::task::headers
    ours::kernel::main::mem::headers
    ours::kernel::lib::gktl
    ours::kernel::lib::logz4
    ours::kernel::lib::arch
)

target_sources(ours_kernel_main_arch
INTERFACE
    # Sources
    "gdt.S"
    "start.S"
    "context.S"
    "smp.cpp"
    "init.cpp"
    "thread.cpp"
    "vm_aspace.cpp"
)

add_subdirectory("platform")

message("Leave kerenl/arch/x86")