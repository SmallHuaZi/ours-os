target_include_directories(kernel_main_headers
INTERFACE
    "include"
)

add_library(kernel_main_irq INTERFACE)
add_library(kernel::main::irq ALIAS kernel_main_irq)
target_link_libraries(kernel_main_irq 
INTERFACE
    kernel::main::headers
)

target_sources(kernel_main_irq 
INTERFACE
    "init.cpp"
    "mod.cpp"
    "irq_object.cpp"
    "irq_dispatcher.cpp"
)
