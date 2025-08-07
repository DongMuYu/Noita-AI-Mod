cmake_minimum_required(VERSION 3.16)
project(SLTrainer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录到项目根目录的bin文件夹
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../../../../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../../../../bin)

# 设置Release配置下的输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/../../../../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/../../../../bin)

# 确保输出目录存在
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# 定义路径
set(DATA_DIR "${CMAKE_SOURCE_DIR}/../../data/SL_data")
set(MODELS_DIR "${CMAKE_SOURCE_DIR}/../../models/SL_models")

# 添加编译定义
add_compile_definitions(
    DATA_DIR="${DATA_DIR}"
    MODELS_DIR="${MODELS_DIR}"
)

# 查找必要的库
find_package(Threads REQUIRED)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/src
)

# 创建监督学习的训练数据结构和函数
set(TRAINING_SOURCES
    ../train_sl.cpp
    SLTrainer.cpp
    SLTrainer.h
)

# 创建可执行文件用于监督学习训练
add_executable(sl_trainer
    ${TRAINING_SOURCES}
)

# 链接必要的库
target_link_libraries(sl_trainer PRIVATE
    Threads::Threads
)

# 设置编译选项
if(MSVC)
    target_compile_options(sl_trainer PRIVATE /utf-8)
    target_compile_definitions(sl_trainer PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# 创建自定义目标用于训练
add_custom_target(train_sl
    COMMAND sl_trainer
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running supervised learning training..."
    DEPENDS sl_trainer
)

# 安装规则（可选）
install(TARGETS sl_trainer DESTINATION bin)

# 打印信息
message(STATUS "SL Trainer Configuration:")
message(STATUS "  Data directory: ${DATA_DIR}")
message(STATUS "  Models directory: ${MODELS_DIR}")
message(STATUS "  Binary output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
