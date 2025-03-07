add_library(ours_kernel_main_irq_headers INTERFACE)
add_library(ours::kernel::main::irq::headers ALIAS ours_kernel_main_irq_headers)
target_include_directories(ours_kernel_main_irq_headers
INTERFACE
    "include"
)

add_library(ours_kernel_main_irq INTERFACE)
add_library(ours::kernel::main::irq ALIAS ours_kernel_main_irq)
target_link_libraries(ours_kernel_main_irq 
INTERFACE
    ours_kernel_main_irq_headers
)
