set(LIEF_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/cmake/LIEF")
message(STATUS "LIEF_DIR: ${LIEF_DIR}")
find_package(LIEF REQUIRED)

add_library(lief INTERFACE)
add_library(lief::lief ALIAS lief)
target_link_libraries(lief
INTERFACE
    LIEF::LIEF
)