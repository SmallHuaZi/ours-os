target_sources(kernel_main_arch
INTERFACE
    "uspace.S"
    "context.S"
    "syscall.S"
    "exception.S"
)