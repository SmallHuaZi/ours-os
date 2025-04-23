set(source_set
    "cpu-local.cpp"
    "fault.cpp"
    "init_pmm.cpp"
    "init_vmm.cpp"
    "memory_model.cpp"
    "pmm.cpp"
    "object-cache.cpp"
    "vmm.cpp"
    "stack.cpp"
    "early-mem.cpp"
    "pm_frame.cpp"
    "pm_zone.cpp"
    "pm_node.cpp"

    # VMM
    "vm_aspace.cpp"
    "vm_area.cpp"
    "vm_mapping.cpp"
    "vm_object.cpp"
    "vm_object_paged.cpp"
    "vm_cow_pages.cpp"
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
)