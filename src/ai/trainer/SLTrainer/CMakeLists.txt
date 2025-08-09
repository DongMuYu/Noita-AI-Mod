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
set(DATA_DIR "${CMAKE_SOURCE_DIR}/../../data")
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
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../..
    ${CMAKE_SOURCE_DIR}/../../core
)

# 传统监督学习源文件
set(TRADITIONAL_TRAINING_SOURCES
    train_sl.cpp
    SLTrainer.cpp
    SLTrainer.h
)

# 序列学习源文件
set(SEQUENCE_TRAINING_SOURCES
    train_sequence.cpp
    SequenceTrainer.cpp
    SequenceTrainer.h
    TrainingManager.cpp
    TrainingManager.h
)

# 根据构建类型决定构建哪些目标
option(BUILD_SEQUENCE_TRAINER "Build sequence trainer" OFF)
option(BUILD_TRADITIONAL_TRAINER "Build traditional supervised learning trainer" OFF)

# 如果没有指定构建类型，默认构建所有
if(NOT BUILD_SEQUENCE_TRAINER AND NOT BUILD_TRADITIONAL_TRAINER)
    set(BUILD_SEQUENCE_TRAINER ON)
    set(BUILD_TRADITIONAL_TRAINER ON)
endif()

# 创建传统监督学习可执行文件
if(BUILD_TRADITIONAL_TRAINER)
    add_executable(sl_trainer
        ${TRADITIONAL_TRAINING_SOURCES}
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
        COMMENT "Running traditional supervised learning training..."
        DEPENDS sl_trainer
    )
endif()

# 创建序列学习可执行文件
if(BUILD_SEQUENCE_TRAINER)
    add_executable(sequence_trainer
        ${SEQUENCE_TRAINING_SOURCES}
        SLTrainer.cpp
        SLTrainer.h
    )

    # 链接必要的库
    target_link_libraries(sequence_trainer PRIVATE
        Threads::Threads
    )

    # 设置编译选项
    if(MSVC)
        target_compile_options(sequence_trainer PRIVATE /utf-8)
        target_compile_definitions(sequence_trainer PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()

    # 创建自定义目标用于训练
    add_custom_target(train_sequence
        COMMAND sequence_trainer
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running sequence learning training..."
        DEPENDS sequence_trainer
    )
endif()

# 创建训练管理器可执行文件（支持混合训练）
if(BUILD_SEQUENCE_TRAINER AND BUILD_TRADITIONAL_TRAINER)
    add_executable(training_manager
        train_manager.cpp
        SLTrainer.cpp
        SequenceTrainer.cpp
        TrainingManager.cpp
        SLTrainer.h
        SequenceTrainer.h
        TrainingManager.h
    )

    # 链接必要的库
    target_link_libraries(training_manager PRIVATE
        Threads::Threads
    )

    # 设置编译选项
    if(MSVC)
        target_compile_options(training_manager PRIVATE /utf-8)
        target_compile_definitions(training_manager PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()

    # 创建自定义目标用于训练
    add_custom_target(train_all
        COMMAND training_manager
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running hybrid training (traditional + sequence)..."
        DEPENDS training_manager
    )
endif()

# 安装规则（可选）
if(BUILD_TRADITIONAL_TRAINER)
    install(TARGETS sl_trainer DESTINATION bin)
endif()
if(BUILD_SEQUENCE_TRAINER)
    install(TARGETS sequence_trainer DESTINATION bin)
endif()
if(BUILD_SEQUENCE_TRAINER AND BUILD_TRADITIONAL_TRAINER)
    install(TARGETS training_manager DESTINATION bin)
endif()

# 打印信息
message(STATUS "SL Trainer Configuration:")
message(STATUS "  Data directory: ${DATA_DIR}")
message(STATUS "  Models directory: ${MODELS_DIR}")
message(STATUS "  Binary output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "  Build sequence trainer: ${BUILD_SEQUENCE_TRAINER}")
message(STATUS "  Build traditional trainer: ${BUILD_TRADITIONAL_TRAINER}")

if(BUILD_TRADITIONAL_TRAINER)
    message(STATUS "  Available: sl_trainer")
endif()
if(BUILD_SEQUENCE_TRAINER)
    message(STATUS "  Available: sequence_trainer")
endif()
if(BUILD_SEQUENCE_TRAINER AND BUILD_TRADITIONAL_TRAINER)
    message(STATUS "  Available: training_manager")
endif()
