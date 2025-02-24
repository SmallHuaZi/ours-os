add_subdirectory(arch)
add_subdirectory(mem)
add_subdirectory(task)
add_subdirectory(sched)

add_library(ours_kernel_main)
add_library(ours::kernel::main ALIAS ours_kernel_main)
target_sources(ours_kernel_main
PRIVATE
    "start.cpp"
    "cpu_local.cpp"
)
target_link_libraries(ours_kernel_main
PRIVATE
    ours::kernel::main::arch
    ours::kernel::main::mem
    ours::kernel::main::task
    ours::kernel::main::sched
    ours::kernel::phys::headers
    ours::kernel::lib::arch
)
