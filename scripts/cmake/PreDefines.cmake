message("Enter PreDefines.cmake")

add_definitions("-DBOOST_CONTAINER_USER_DEFINED_THROW_CALLBACKS=1")
add_definitions("-DBOOST_DISABLE_ASSERTS=1")
add_definitions("-DBOOST_ENABLE_ASSERT_HANDLER=1")
add_definitions("-DBOOST_NO_EXCEPTIONS=1")
add_definitions("-D_LIBCPP_HAS_NO_LOCALIZATION=1")
# add_definitions("-D_LIBCPP_HAS_NO_WIDE_CHARACTERS=1")

add_definitions("-DOURS_CONFIG_TARGET_64BIT=1")

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64") 
    set(OURS_ARCH "x86")
else()
    set(OURS_ARCH ${CMAKE_SYSTEM_PROCESSOR})
endif()

include(${OURS_ARCH}PreDefines)

add_definitions("-DOURS_DEV=1")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions("-DOURS_DEBUG=1")
else (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_definitions("-DOURS_RELEASE=1")
endif ()

add_definitions("-DOURS_CONFIG_NUMA=1")

message("Leave PreDefines.cmake")