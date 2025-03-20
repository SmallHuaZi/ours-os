set(source_set
    "cpu_local.cpp"
    "fault.cpp"
    "init_pmm.cpp"
    "init_vmm.cpp"
    "memory_model.cpp"
    "pmm.cpp"
    "vmm.cpp"
    "new.cpp"
    "stack.cpp"
    "early_mem.cpp"
    "pm_frame.cpp"
    "pm_zone.cpp"
    "pm_node.cpp"
    "vm_area.cpp"
    "vm_aspace.cpp"
    "vm_object.cpp"
    "vm_root_area.cpp"
)

add_library(kernel_mem INTERFACE)
add_library(kernel::main::mem ALIAS kernel_mem)
target_sources(kernel_mem 
INTERFACE
    ${source_set}
)
target_link_libraries(kernel_mem
INTERFACE
    kernel::main::headers
    kernel::lib::kmrd
    kernel::lib::heap
)