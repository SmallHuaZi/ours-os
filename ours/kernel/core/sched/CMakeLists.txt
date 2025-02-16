set(source_set
    "cpu_local.cpp"
    "eevdf.cpp"
    "main_scheduler.hpp"
    "main_scheduler.cpp"
    "mod.cpp"
)

add_library(ours_kernel_sched_headers INTERFACE)
add_library(ours::kernel::sched::headers ALIAS ours_kernel_sched_headers)
target_include_directories(ours_kernel_sched_headers
INTERFACE
    "include"
)
target_link_libraries(ours_kernel_sched_headers
INTERFACE
    ours::kernel::common::headers
)

add_library(ours_kernel_sched INTERFACE ${source_set})
add_library(ours::kernel::sched ALIAS ours_kernel_sched)
target_link_libraries(ours_kernel_sched
INTERFACE
    ours::kernel::sched::headers
)