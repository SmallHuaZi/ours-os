add_subdirectory(arch)
add_subdirectory(mem)
add_subdirectory(task)
add_subdirectory(sched)

add_executable(ours_kernel_main)
add_executable(ours::kernel::main ALIAS ours_kernel_main)
target_sources(ours_kernel_main
PRIVATE
    "main.ld"

    "panic.cpp"
    "start.cpp"
    "handoff.cpp"
    "cpu_local.cpp"
)
target_link_libraries(ours_kernel_main
PRIVATE
    ours::kernel::main::arch
    ours::kernel::main::mem
    ours::kernel::main::task
    ours::kernel::main::sched
    ours::kernel::phys::headers
    ours::kernel::lib::arch
    ours::kernel::lib::heap
)

target_compile_options(ours_kernel_main
PRIVATE
    "-mcmodel=kernel"
    "-O0"
    "-stdlib=libc++"
    # "-fPIC"
    # "-fno-PIE" 
    "-fno-rtti"
    "-fno-builtin"
    # "-fno-leading-underscore"
    "-fno-exceptions"
    "-fno-stack-protector"
    "-fno-use-cxa-atexit"
    "-ffreestanding"
    "-Wno-write-strings"

    # For clang
    "-Xclang" 
    "-fdefault-calling-conv=cdecl"
)

target_link_options(ours_kernel_main
PRIVATE
    "-v"
    "-Wl,--no-pie;"
    "-Wl,--no-relax"
    "-Wl,--build-id=none;"
    "-Wl,--no-warn-rwx-segments"
    "-Wl,-z,noexecstack"
    "-Wl,--emit-relocs"
    "-static"
    # Clang needs -mcmodel=kernel to tell it to use the right safe-stack
    # ABI for the kernel.
    "-mcmodel=kernel"
    "-nodefaultlibs;"
    "-nostartfiles"
    "-nostdlib++"
    "-mno-red-zone"
    "-fno-register-global-dtors-with-atexit"
    "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/main.ld"

    # This a temporary base address will be removed,
    # and it should provides by cofiguration files.
    "-Wl,-defsym,MAIN_BASE=0xffffff8000100000"
    "-Wl,-z,max-page-size=0x1000"
    "-Wl,--gc-sections"
)

add_custom_command(
    TARGET ours::kernel::main PRE_BUILD
    COMMAND
)

add_custom_target(ours_kernel_raw_main_image
    TARGET ours::kernel::main POST_BUILD
    # Strip all unneeded sections
    COMMAND ${CMAKE_OBJCOPY} ARGS --strip-unneeded ${CMAKE_BINARY_DIR}/ours_kernel_main ${CMAKE_BINARY_DIR}/${OURA_ARCH}_raw_main_image
    # Compress main
    COMMAND 
    COMMENT "Build raw `main` image which is without bootable header" 
)

add_executable(ours_kernel_main_image)
add_executable(ours::kernel::main::image ALIAS ours_kernel_main_image)
target_sources(ours_kernel_main_image
PRIVATE
    "arch/${OURS_ARCH}/image.S"
    "image.ld"
)
target_link_options(ours_kernel_main_image
PRIVATE
    "-v"
    "-nostartfiles"
    "-nodefaultlibs"
    "-T${CMAKE_CURRENT_SOURCE_DIR}/main.ld"
    "-R,${CMAKE_BINARY_DIR}/ours_kernel_main"
)

add_dependencies(ours_kernel_main_image ours_kernel_raw_main_image)