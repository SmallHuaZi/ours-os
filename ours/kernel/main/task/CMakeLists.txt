set(source_set
    "thread.cpp"
    "process.cpp"
)

add_library(ours_kernel_task_headers INTERFACE)
add_library(ours::kernel::main::task::headers ALIAS ours_kernel_task_headers)
target_include_directories(ours_kernel_task_headers
INTERFACE
    "include"
)
target_link_libraries(ours_kernel_task_headers 
INTERFACE
    ours::kernel::main::arch::headers
    ours::kernel::main::mem::headers
    ours::kernel::main::sched::headers
    ours::kernel::object::headers
)

add_library(ours_kernel_task INTERFACE ${source_set})
add_library(ours::kernel::main::task ALIAS ours_kernel_task)
target_link_libraries(ours_kernel_task 
INTERFACE
    ours::kernel::main::task::headers
)