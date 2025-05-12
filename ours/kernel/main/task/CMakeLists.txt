target_include_directories(kernel_main_headers
INTERFACE
    "include"
)
set(source_set
    "thread.cpp"
    "timer.cpp"
    "timer-queue.cpp"
    "wait-queue.cpp"
)

add_library(kernel_task INTERFACE)
add_library(kernel::main::task ALIAS kernel_task)
target_sources(kernel_task 
INTERFACE
    ${source_set}
)
target_link_libraries(kernel_task 
INTERFACE
    kernel::main::headers
)