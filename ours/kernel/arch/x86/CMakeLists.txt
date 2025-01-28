add_library(ours_kernel_arch_headers INTERFACE)
add_library(ours::kernel::arch::headers ALIAS ours_kernel_arch_headers)
target_include_directories(ours_kernel_arch_headers
INTERFACE
    "include"
)

add_library(ours_kernel_arch INTERFACE)
add_library(ours::kernel::arch ALIAS ours_kernel_arch)
target_link_libraries(ours_kernel_arch
INTERFACE
    ours::kernel::arch::headers
    ours::kernel::mem::headers
    ours::kernel::task::headers
    ours::kernel::lib::multiboot
    ours::kernel::lib::gktl
    ours::kernel::lib::logz4
    ours::kernel::lib::arch
)

target_sources(ours_kernel_arch 
INTERFACE
    # Dependents
    "link.ld"

    # Sources
    "multiboot2.S"
    "start.S"
    # "${CMAKE_CURRENT_SOURCE_DIR}/gdt.S"
    "main.cpp"
    "init.cpp"
    "descriptor.cpp"
    "early_console.cpp"
    "vm_aspace.cpp"

    # "${CMAKE_CURRENT_SOURCE_DIR}/page_table.cpp"
)

target_compile_options(ours_kernel_arch
INTERFACE
    "-m32"
    "-O0"
    "-fno-use-cxa-atexit"
    "-fno-builtin"
    # "-fno-leading-underscore"
    "-fno-exceptions"
    "-ffreestanding"
    "-fno-rtti"
    "-fno-stack-protector"
    "-Wno-write-strings"
    "-stdlib=libc++"

    # For clang
    "-Xclang" 
    "-fdefault-calling-conv=cdecl"
)

target_link_options(ours_kernel_arch
INTERFACE
    "-m32"
    "-nodefaultlibs;"
    "-no-pie;"
    "-Wl,--build-id=none;"
    "-nostartfiles"
    "-ffreestanding"
    "-fno-register-global-dtors-with-atexit"
    "-T${CMAKE_CURRENT_SOURCE_DIR}/link.ld"
)
