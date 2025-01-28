set(source_set
    "cpu_local.cpp"
    "fault.cpp"
    "init.cpp"
    "mod.cpp"
    "new.cpp"
    "pm_frame.cpp"
    "pm_zone.cpp"
    "pm_node.cpp"
    "vm_area.cpp"
    "vm_aspace.cpp"
    "vm_object.cpp"
    "vm_root_area.cpp"
)

add_library(ours_kernel_mem_headers INTERFACE)
add_library(ours::kernel::mem::headers ALIAS ours_kernel_mem_headers)
target_include_directories(ours_kernel_mem_headers
INTERFACE
    "include"
    ours::kernel::common::headers
)

add_library(ours_kernel_mem INTERFACE)
add_library(ours::kernel::mem ALIAS ours_kernel_mem)
target_sources(ours_kernel_mem 
INTERFACE
    ${source_set}
)
target_link_libraries(ours_kernel_mem
INTERFACE
    ours::kernel::mem::headers
)