target_include_directories(kernel_main_headers
INTERFACE
    "include"
)
set(source_set
    "fair.cpp"
    "scheduler.cpp"
    "init.cpp"
    "mod.cpp"

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
target_link_options(kernel_task
INTERFACE
    "-T${CMAKE_CURRENT_SOURCE_DIR}/task.ld"
)