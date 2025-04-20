target_link_options(kernel_main
PRIVATE
    "-Wl,-T${CMAKE_CURRENT_SOURCE_DIR}/tests.ld"
)

target_include_directories(kernel_main
PRIVATE
    "include"
)