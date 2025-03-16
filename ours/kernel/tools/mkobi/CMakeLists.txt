function(target_to_binary target binary_output)
    # 获取目标的输出文件路径
    set(target_dir "$<TARGET_FILE_DIR:${target}>")
    set(target_file "$<TARGET_FILE:${target}>")

    # 确保 objcopy 工具可用
    find_program(OBJCOPY_EXECUTABLE objcopy)
    if(NOT OBJCOPY_EXECUTABLE)
        message(FATAL_ERROR "objcopy tool not found!")
    endif()

    # 添加自定义命令，使用 objcopy 提炼二进制
    add_custom_target(${binary_output} ALL
        COMMAND ${OBJCOPY_EXECUTABLE} -O binary ${target_file} ${target_dir}/${binary_output}
        COMMENT "Extracting pure binary from target ${target} using objcopy"
        DEPENDS ${target}
    )

    # 打印生成的二进制文件路径
    message(STATUS "Generated binary: ${binary_output}")
endfunction()

# Define an action to convert an ELF file to a raw binary image file.
# Parameters
#   Deps 
#       Required: Dependencies leading to a linking target (e.g. executable).
#   OutName
#       Optional: Basename of the output file.
#       Type: string
#       Default: $target_name
function(make_obi image_name)
    # 定义参数类型
    set(oneValueArgs TARGET)
    set(multiValueArgs DEPENDS)
    
    # 解析参数
    cmake_parse_arguments(
        MakeImage           # 前缀
        "${options}"        # 选项参数
        "${oneValueArgs}"   # 单值参数
        "${multiValueArgs}" # 多值参数
        ${ARGN}             # 输入参数
    )

    # 检查必要参数
    if(NOT MakeImage_TARGET)
        message(FATAL_ERROR "Missing required argument: TARGET")
    endif()

    target_to_binary(${MakeImage_TARGET} ${MakeImage_TARGET}.bin)

    set(target_binary "$<TARGET_FILE_DIR:${MakeImage_TARGET}>/${MakeImage_TARGET}.bin")
    add_executable(${image_name}.elf)
    target_compile_definitions(${image_name}.elf
    PRIVATE
        "IMAGE_FILE=\"${target_binary}\""
    )

    target_sources(${image_name}.elf
    PRIVATE
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/image.ld"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/image.S"
    )
    target_link_libraries(${image_name}.elf
    PRIVATE
        kernel::lib::omi::headers
    )
    target_link_options(${image_name}.elf
    PRIVATE
        "-v"
        "-Wl,--no-pie;"
        "-Wl,--no-relax"
        "-Wl,--no-warn-rwx-segments"
        "-Wl,--build-id=none;"
        "-static"
        "-mcmodel=kernel"
        "-nostartfiles"
        "-nodefaultlibs"
        "-T${CMAKE_CURRENT_FUNCTION_LIST_DIR}/image.ld"
        "-Wl,--just-symbols=$<TARGET_FILE:${MakeImage_TARGET}>"
    )
    add_dependencies(${image_name}.elf  ${MakeImage_TARGET}.bin)

    set(ExeDir "$<TARGET_FILE_DIR:${MakeImage_TARGET}>")
    target_to_binary(${image_name}.elf ${image_name}.bin)
    add_custom_target(${image_name}
        COMMAND ${CMAKE_COMMAND} -E echo "Generating image" 
        DEPENDS ${MakeImage_TARGET} ${image_name}.bin
        WORKING_DIRECTORY "${ExeDir}"
    )

endfunction()