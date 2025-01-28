set(source_set
    "thread.cpp"
    "process.cpp"
)

add_library(ours_kernel_task_headers INTERFACE)
add_library(ours::kernel::task::headers ALIAS ours_kernel_task_headers)
target_include_directories(ours_kernel_task_headers
INTERFACE
    "include"
)

add_library(ours_kernel_task INTERFACE ${source_set})
add_library(ours::kernel::task ALIAS ours_kernel_task)
target_link_libraries(ours_kernel_task
INTERFACE
    ours_kernel_task_headers
)