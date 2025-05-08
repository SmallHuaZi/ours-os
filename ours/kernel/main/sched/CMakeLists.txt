set(source_set
    "eevdf.cpp"
    "scheduler.cpp"
    "mod.cpp"
)

add_library(kernel_sched INTERFACE)
add_library(kernel::main::sched ALIAS kernel_sched)
target_sources(kernel_sched 
INTERFACE
    ${source_set}
)
target_link_libraries(kernel_sched
INTERFACE
    kernel::main::headers
    kernel::lib::cnl
)