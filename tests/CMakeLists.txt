cmake_minimum_required(VERSION 3.16)
project(SafetyCheckerTests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)  # 强制MSVC使用UTF-8
endif()

# 设置源文件
set(TEST_SOURCES
    SafetyCheckerTest.cpp
)

# 创建测试可执行文件
add_executable(safety_checker_test ${TEST_SOURCES})