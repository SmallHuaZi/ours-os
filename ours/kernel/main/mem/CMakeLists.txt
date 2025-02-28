set(source_set
    "cpu_local.cpp"
    "fault.cpp"
    "init_pmm.cpp"
    "init_vmm.cpp"
    "memory_model.cpp"
    "mod.cpp"
    "new.cpp"
    "early_mem.cpp"
    "pm_frame.cpp"
    "pm_zone.cpp"
    "pm_node.cpp"
    "vm_area.cpp"
    "vm_aspace.cpp"
    "vm_object.cpp"
    "vm_root_area.cpp"
)

add_library(ours_kernel_mem_headers INTERFACE)
add_library(ours::kernel::main::mem::headers ALIAS ours_kernel_mem_headers)
target_include_directories(ours_kernel_mem_headers
INTERFACE
    "include"
)
target_link_libraries(ours_kernel_mem_headers
INTERFACE
    ours::kernel::common::headers
    ours::kernel::lib::kmrd::headers
)

add_library(ours_kernel_mem INTERFACE)
add_library(ours::kernel::main::mem ALIAS ours_kernel_mem)
target_sources(ours_kernel_mem 
INTERFACE
    ${source_set}
)
target_link_libraries(ours_kernel_mem
INTERFACE
    ours::kernel::main::mem::headers
    ours::kernel::lib::kmrd
    ours::kernel::lib::heap
)