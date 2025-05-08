message("Enter PreDefines.cmake")

set(OX_PREDEFINES 
    "-D_LIBCPP_HAS_NO_LOCALIZATION=1" 
    "-D_LIBCPP_HAS_LOCALIZATION=0"
    "-D_LIBCPP_HAS_NO_THREADS=1"
    # "-D_LIBCPP_HAS_WIDE_CHARACTERS=0"
    "-D_LIBCPP_HAS_UNICODE=0"
    "-D_LIBCPP_HAS_RANDOM_DEVICE=0" # Random device is not supported in freestanding environment
    "-D_LIBCPP_HAS_FILESYSTEM=0" # Filesystem is not supported in freestanding environment
    # "-D_LIBCPP_HAS_NO_WIDE_CHARACTERS=1"
    "-DBOOST_CONTAINER_USER_DEFINED_THROW_CALLBACKS=1"
    "-DBOOST_DISABLE_ASSERTS=1"
    "-DBOOST_ENABLE_ASSERT_HANDLER=1"
    "-DBOOST_NO_IOSTREAM=1"
    "-DBOOST_NO_EXCEPTIONS=1"

    "-DCNL_USE_INT128=1"
    "-DWIDE_INTEGER_DISABLE_IOSTREAM=1"
)

add_definitions("-DOURS_CONFIG_TARGET_64BIT=1")
add_definitions("-DOURS_CONFIG_MAX_PAGE_SHIFT=21")

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