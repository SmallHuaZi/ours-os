# 查找所有以 _test.cpp 结尾的文件
file(GLOB TEST_SOURCES "*_test.cpp")

# 添加 Google Test
enable_testing()

# 遍历所有测试文件并生成可执行文件
foreach(TEST_SOURCE ${TEST_SOURCES})
    # 获取文件名（不包含路径）
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    # 创建测试可执行文件
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_include_directories(${TEST_NAME} PRIVATE ${GTEST_INCLUDE_DIRS})
    # 链接 Google Test 库
    target_link_libraries(${TEST_NAME} PRIVATE bootmem GTest::gtest GTest::gtest_main)
    # 添加到测试列表中
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
