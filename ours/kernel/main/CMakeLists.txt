add_subdirectory(arch)
add_subdirectory(irq)
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
    ours::kernel::lib::libc
    ours::kernel::lib::logz4
)

target_compile_options(ours_kernel_main
PRIVATE
    "-mcmodel=kernel"
    "-O0"
    "-stdlib=libc++"
    "-ffreestanding"
    # "-fPIC"
    # "-fno-PIE" 
    "-fno-rtti"
    "-fno-builtin"
    # "-fno-leading-underscore"
    "-fno-exceptions"
    "-fno-stack-protector"
    "-fno-use-cxa-atexit"
    "-Wno-write-strings"

    # For clang
    "-Xclang" 
    "-fdefault-calling-conv=cdecl"
)

target_link_options(ours_kernel_main
PRIVATE
    "-v"
    "-Wl,--no-pie"
    "-Wl,--no-relax"
    "-Wl,--build-id=none"
    "-Wl,--no-warn-rwx-segments"
    "-Wl,-z,noexecstack"
    "-Wl,--emit-relocs"
    "-lgcc"
    "-static"
    # Clang needs -mcmodel=kernel to tell it to use the right safe-stack
    # ABI for the kernel.
    "-mcmodel=kernel"
    "-nodefaultlibs;"
    "-nostartfiles"
    "-nostdlib++"
    "-mno-red-zone"
    "-fno-register-global-dtors-with-atexit"

    # This a temporary base address will be removed,
    # and it should provides by cofiguration files.
    "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/main.ld"
    "-Wl,-defsym,MAIN_BASE=${KERNEL_BASE}"
    "-Wl,-z,max-page-size=${MAX_PAGE_SIZE}"
    "-Wl,--gc-sections"
    "-Wl,--emit-relocs"
)

make_image(ours_kernel_main_raw_image
    TARGET ours::kernel::main
)

add_executable(ours_kernel_redundant_main_image)
target_include_directories(ours_kernel_redundant_main_image
PRIVATE
    "${CMAKE_BINARY_DIR}/gen"
)
target_sources(ours_kernel_redundant_main_image
PRIVATE
    "image.ld"
    "arch/${OURS_ARCH}/image.S"
)
target_link_libraries(ours_kernel_redundant_main_image
PRIVATE
    ours::kernel::common::headers
    ours::kernel::lib::omi::headers
)
target_link_options(ours_kernel_redundant_main_image
PRIVATE
    "-v"
    "-Wl,--no-pie;"
    "-Wl,--no-relax"
    "-Wl,--no-warn-rwx-segments"
    "-Wl,--build-id=none;"
    "-static"
    "-mcmodel=kernel"
    "-nostartfiles"
    "-nodefaultlibs"
    "-T${CMAKE_CURRENT_SOURCE_DIR}/image.ld"
    "-Wl,--just-symbols=$<TARGET_FILE:ours_kernel_main>"
)

add_dependencies(ours_kernel_redundant_main_image ours_kernel_main_raw_image)

make_image(ours_kernel_main_image
    TARGET ours_kernel_redundant_main_image
)