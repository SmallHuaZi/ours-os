cmake_minimum_required(VERSION 3.20)

project(lockdep VERSION "${OURS_VERSION}" LANGUAGES CXX)

# add_library(lockdep_headers INTERFACE)
# add_library(kernel::lib::arch::headers ALIAS lockdep_headers)
# target_include_directories(lockdep_headers
# INTERFACE
#     "include"
# )
# target_link_libraries(lockdep_headers
# INTERFACE
#     kernel::main::headers
#     # This is an error dependency.
#     kernel::main::headers
# )

# add_library(lockdep)
# add_library(kernel::lib::arch ALIAS lockdep)
# target_link_libraries(lockdep
# PUBLIC
#     lockdep_headers
# )