add_library(cft_asmflags INTERFACE)
add_library(cft::asmflags ALIAS cft_asmflags)
target_compile_definitions(cft_asmflags
INTERFACE
    $<$<COMPILE_LANGUAGE:ASM>:__ASSEMBLY__>
)
