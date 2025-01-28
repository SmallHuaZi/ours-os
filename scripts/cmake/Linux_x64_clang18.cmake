# set(CMAKE_SYSTEM_NAME Generic)
# set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(TRIPLE x86_64-unknown-elf)

set(CMAKE_C_COMPILER    "clang-18")
# set(CMAKE_C_COMPILER_TARGET ${TRIPLE})
set(CMAKE_CXX_COMPILER  "clang++-18")
# set(CMAKE_CXX_COMPILER_TARGET ${TRIPLE})
set(CMAKE_ASM_COMPILER  "clang-18")

set(COMMON_FLAGS 
    "-m32;"
    "-O0;"
    "-fno-use-cxa-atexit;"
    "-fno-builtin;"
    "-fno-leading-underscore;"
    "-fno-exceptions;"
    "-ffreestanding;"
    "-fno-rtti;"
    "-fno-stack-protector;"
    "-Wno-write-strings;"

    # For clang
    "-Xclang;"
    "-fdefault-calling-conv=cdecl;"
)

# set(CMAKE_C_FLAGS_INIT "-m32 -fno-use-cxa-atexit -fno-builtin -O0 -fno-exceptions -ffreestanding -fno-rtti -fno-stack-protector -Wno-write-strings -Xclang -fdefault-calling-conv=cdecl")
# set(CMAKE_CXX_FLAGS_INIT "-m32 -fno-use-cxa-atexit -fno-builtin -O0 -fno-exceptions -ffreestanding -fno-rtti -fno-stack-protector -Wno-write-strings -Xclang -fdefault-calling-conv=cdecl")
# set(CMAKE_ASM_FLAGS_INIT ${COMMON_FLAGS})
#set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld -nostdlib -static")
#set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld -nostdlib -static")
#set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld -nostdlib -static")
message("Toolchains file: Linux_x64_clang18.cmake")