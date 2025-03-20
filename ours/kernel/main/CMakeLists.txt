add_library(kernel_main_headers INTERFACE)
add_library(kernel::main::headers ALIAS kernel_main_headers)
target_include_directories(kernel_main_headers 
INTERFACE
    "include"
)

file(GLOB ALL_SUBDIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(SUBDIR ${ALL_SUBDIRS})
    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR}/include)
        target_include_directories(kernel_main_headers  INTERFACE ${SUBDIR}/include)
    endif()
endforeach()

target_link_libraries(kernel_main_headers
INTERFACE
    system::public::headers
    kernel::object::headers
    kernel::lib::ktl::headers
    kernel::lib::gktl::headers
    kernel::lib::ustl::headers
    kernel::lib::arch::headers
)

add_executable(kernel_main)
add_executable(kernel::main ALIAS kernel_main)

target_link_libraries(kernel_main
PRIVATE
    kernel::main::headers
)

add_subdirectory(irq)
add_subdirectory(mem)
add_subdirectory(task)
add_subdirectory(sched)

target_compile_definitions(kernel_main
PRIVATE
    "OURS_CURRENT_TARGET_IS_MAIN=1"
    "OURS_CURRENT_TARGET_IS_64BIT=OURS_CONFIG_TARGET_64BIT"
)
target_sources(kernel_main
PRIVATE
    "main.ld"
    "panic.cpp"
    "start.cpp"
    "handoff.cpp"
    "cpu_local.cpp"
)
target_link_libraries(kernel_main
PRIVATE
    kernel::main::arch
    kernel::main::irq
    kernel::main::mem
    kernel::main::task
    kernel::main::sched
    kernel::phys::headers
    kernel::lib::gktl
    kernel::lib::ktl
    kernel::lib::ustl

    kernel::lib::arch
    kernel::lib::libc
    kernel::lib::logz4

    "libgcc.a"

    cft::asmflags 
    # cft::nopie
)

target_compile_options(kernel_main
PRIVATE
    "-mcmodel=kernel"
    "-O0"
    "-stdlib=libc++"
    "-ffreestanding"
    "-fPIE"
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

target_link_options(kernel_main
PRIVATE
    "-v"
    "-pie"
    "-Wl,--build-id=none"
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

    # This a temporary base address will be removed,
    # and it should provides by cofiguration files.
    "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/main.ld"
    "-Wl,-z,max-page-size=${MAX_PAGE_SIZE}"
    "-Wl,--gc-sections"
    "-Wl,--emit-relocs"
)

make_obi(kernel_main_image
    TARGET kernel_main
)