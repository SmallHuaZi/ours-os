add_library(cft_asmflags INTERFACE)
add_library(cft::asmflags ALIAS cft_asmflags)
target_compile_definitions(cft_asmflags
INTERFACE
    $<$<COMPILE_LANGUAGE:ASM>:__ASSEMBLY__>
)

add_library(cft_nopie INTERFACE)
add_library(cft::nopie ALIAS cft_nopie)
target_link_options(cft_nopie
INTERFACE
    "-Wl,--no-pie"
    "-Wl,--no-relax"
)