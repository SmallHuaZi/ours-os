add_subdirectory(mem)
add_subdirectory(task)
add_subdirectory(sched)

add_library(ours_kernel_core INTERFACE)
add_library(ours::kernel::core ALIAS ours_kernel_core)
target_sources(ours_kernel_core
INTERFACE
    "start.cpp"
    "cpu_local.cpp"
)
target_link_libraries(ours_kernel_core
INTERFACE
    ours::kernel::phys::headers
    ours::kernel::mem
    ours::kernel::task
    ours::kernel::sched
    ours::kernel::lib::arch
)
