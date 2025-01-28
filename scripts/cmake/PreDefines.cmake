message("Enter PreDefines.cmake")

add_definitions("-DOURS_DEV")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions("-DOURS_DEBUG=1")
else (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_definitions("-DOURS_RELEASE=1")
endif ()

message("Leave PreDefines.cmake")