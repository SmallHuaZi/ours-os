target_sources(kernel_main_arch
INTERFACE
    "mmu.cpp"
    "fault.cpp"
    "vm_aspace.cpp"
)