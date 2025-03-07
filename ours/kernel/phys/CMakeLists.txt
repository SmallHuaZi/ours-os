add_library(ours_kernel_phys_headers INTERFACE)
add_library(ours::kernel::phys::headers ALIAS ours_kernel_phys_headers)
target_include_directories(ours_kernel_phys_headers
INTERFACE
    "include"
)
target_link_libraries(ours_kernel_phys_headers
INTERFACE
    ours::kernel::common::headers
    ours::kernel::phys::lib::bootmem::headers
)

add_executable(ours_kernel_phys)
add_executable(ours::kernel::phys ALIAS ours_kernel_phys)
target_compile_options(ours_kernel_phys
PRIVATE
# "-v"
"-g"
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
)
target_link_libraries(ours_kernel_phys
PRIVATE
    ours::kernel::phys::headers
    ours::kernel::phys::arch
    ours::kernel::phys::lib::bootmem
    ours::kernel::lib::libc
    ours::kernel::lib::gktl::goi
    ours::kernel::lib::omi
    "libgcc.a"
)
target_link_options(ours_kernel_phys
PRIVATE
# "-v"
    # Link to libgcc for some builtin routine.
    "-nostdlib"
    "-nostdlib++"
    "-nodefaultlibs"
    "-Wl,--no-pie"
    "-Wl,--no-relax"
    "-Wl,--build-id=none"
    "-Wl,--no-warn-rwx-segments"
    # This is a hosted dependency flag
    "-L/usr/lib/gcc/x86_64-linux-gnu/13/32"
    "-fno-register-global-dtors-with-atexit"
    "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/phys.ld"
    "-static"
)

add_dependencies(ours_kernel_phys ours_kernel_main_image)

add_subdirectory("lib")
add_subdirectory("core")
add_subdirectory("arch")
