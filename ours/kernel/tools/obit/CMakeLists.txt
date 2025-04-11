cmake_minimum_required(VERSION 3.10)
project(obit VERSION 0.1.0 LANGUAGES CXX)

add_executable(obit "main.cpp" "create.cpp" "kpkg.cpp")
target_link_libraries(obit
PRIVATE
    "kernel::lib::omitl"
    "cxxopts::cxxopts"
    "lief::lief"
    "spdlog::spdlog"
)
