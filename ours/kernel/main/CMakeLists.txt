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
add_subdirectory(tests)

set(KernelMainTestFileSet 
    "cpu-local-tests.cpp"
)

target_compile_definitions(kernel_main
PRIVATE
    ${OX_PREDEFINES}
    "OURS_CURRENT_TARGET_IS_MAIN=1"
    "OURS_CURRENT_TARGET_IS_64BIT=OURS_CONFIG_TARGET_64BIT"
)
target_sources(kernel_main
PRIVATE
    "main.ld"
    "panic.cpp"
    "start.cpp"
    "handoff.cpp"
    "cpu-local.cpp"
    "cpu-states.cpp"
    "ipi-event.cpp"

    "${KernelMainTestFileSet}"

    "main.ld"
)
target_link_libraries(kernel_main
PRIVATE
    kernel::main::arch
    kernel::main::irq
    kernel::main::mem
    kernel::main::task
    kernel::platform
    kernel::lib::gktl
    kernel::lib::ktl
    kernel::lib::ustl

    kernel::lib::arch
    kernel::lib::libc
    kernel::lib::logz4

    kernel::phys::headers

    "libgcc.a"

    cft::asmflags 
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
    "-fno-elide-constructors"
    "-Wno-write-strings"
    "-mno-sse" 
    "-mno-mmx"

    # For clang
    "-Xclang" 
    "-fdefault-calling-conv=cdecl"
)

target_link_options(kernel_main
PRIVATE
    "-v"
    "-Wl,--build-id=none"
    "-Wl,--no-warn-rwx-segments"
    "-Wl,-z,noexecstack"
    "-Wl,--emit-relocs"
    "-Wl,--gc-sections"
    "-Wl,-pie"
    "-Wl,-static"
    "-Wl,--no-dynamic-linker"
    # Clang needs -mcmodel=kernel to tell it to use the right safe-stack
    # ABI for the kernel.
    "-mcmodel=kernel"
    "-nodefaultlibs;"
    "-nostartfiles"
    "-nostdlib++"
    "-nostdlib"
    "-mno-red-zone"
    "-fno-register-global-dtors-with-atexit"

    # This a temporary base address will be removed,
    # and it should provides by cofiguration files.
    "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/main.ld"
    "-Wl,-z,max-page-size=${MAX_PAGE_SIZE}"
)

add_custom_command(TARGET kernel_main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Creating ours.obi"
    # Create the kernel_main.obi file without container header.
    COMMAND obit create -t kpkg -o "$<TARGET_FILE_DIR:kernel_main>/ours.kpkg"
    COMMAND obit kpkg -p "$<TARGET_FILE_DIR:kernel_main>/ours.kpkg" --add "$<TARGET_FILE:kernel_main>" -t elf -e -o "mmod.elf"
    # Create the container obi. 
    COMMAND obit create -f "$<TARGET_FILE_DIR:kernel_main>/ours.kpkg" -t cont -o "$<TARGET_FILE_DIR:kernel_main>/ours.obi"
    COMMAND ${CMAKE_COMMAND} -E echo "ours.obi has been created"
)