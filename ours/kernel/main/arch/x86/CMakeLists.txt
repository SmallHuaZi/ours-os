message("Enter kerenl/arch/x86")
target_link_libraries(ours_kernel_main_arch
INTERFACE
    ours::kernel::main::arch::headers
    ours::kernel::main::task::headers
    ours::kernel::main::mem::headers
    ours::kernel::lib::gktl
    ours::kernel::lib::logz4
    ours::kernel::lib::arch
)

target_sources(ours_kernel_main_arch
INTERFACE
    # Sources
    "start.S"
    "init.cpp"
    "vm_aspace.cpp"
)

target_compile_options(ours_kernel_main_arch
INTERFACE
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

target_link_options(ours_kernel_main_arch
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

message("Leave kerenl/arch/x86")