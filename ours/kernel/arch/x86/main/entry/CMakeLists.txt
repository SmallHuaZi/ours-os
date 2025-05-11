target_sources(kernel_main_arch
INTERFACE
    "uspace.S"
    "context.S"
    "syscall.S"
    "exception.S"
    "bootstrap16.S"
    "start.S"
)

target_link_libraries(kernel_main_arch
INTERFACE
    cft::asmflags
)