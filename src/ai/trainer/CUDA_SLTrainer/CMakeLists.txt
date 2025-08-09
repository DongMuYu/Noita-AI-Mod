# CUDA序列训练器CMake配置文件
# 用于构建基于CUDA加速的序列预测模型训练器

cmake_minimum_required(VERSION 3.18)
project(CUDASequenceTrainer)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找CUDA Toolkit
find_package(CUDAToolkit REQUIRED)

# 设置CUDA架构
set(CMAKE_CUDA_ARCHITECTURES 70 75 80)  # 支持的GPU架构

# CUDA编译选项
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# 启用CUDA语言
enable_language(CUDA)

# 包含目录
include_directories(${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 源文件
set(CUDA_SOURCES
    CUDASequenceTrainer.cpp
)

set(CPP_SOURCES
    train_cuda_sequence.cpp
)

# CUDA库
set(CUDA_LIBRARIES
    CUDA::cudart
    CUDA::cublas
    CUDA::cudnn
)

# 创建CUDA序列训练器库
add_library(CUDASequenceTrainer STATIC
    ${CUDA_SOURCES}
)

target_link_libraries(CUDASequenceTrainer ${CUDA_LIBRARIES})
set_target_properties(CUDASequenceTrainer PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
)

# 创建训练器可执行文件
add_executable(train_cuda_sequence
    ${CPP_SOURCES}
)

target_link_libraries(train_cuda_sequence
    CUDASequenceTrainer
    ${CUDA_LIBRARIES}
)

# 设置输出目录
set_target_properties(CUDASequenceTrainer PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

set_target_properties(train_cuda_sequence PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 编译定义
add_definitions(-DCUDA_ENABLED)

# 安装规则
install(TARGETS CUDASequenceTrainer
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(TARGETS train_cuda_sequence
    RUNTIME DESTINATION bin
)

# 打印配置信息
message(STATUS "CUDA Sequence Trainer Configuration:")
message(STATUS "  CUDA Version: ${CMAKE_CUDA_VERSION}")
message(STATUS "  CUDA Include: ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}")
message(STATUS "  CUDA Libraries: ${CUDA_LIBRARIES}")
message(STATUS "  CUDA Architectures: ${CMAKE_CUDA_ARCHITECTURES}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")

# 调试模式配置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
    target_compile_options(CUDASequenceTrainer PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-G -g>)
    target_compile_options(train_cuda_sequence PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-G -g>)
    message(STATUS "  Debug mode enabled")
else()
    add_definitions(-DNDEBUG)
    target_compile_options(CUDASequenceTrainer PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-O3>)
    target_compile_options(train_cuda_sequence PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-O3>)
    message(STATUS "  Release mode enabled")
endif()

# 性能优化选项
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # 启用Tensor Cores
    add_definitions(-DENABLE_TENSOR_CORES)
    
    message(STATUS "  Performance optimizations enabled")
    message(STATUS "  Tensor Cores enabled")
endif()

# 内存管理选项
option(ENABLE_MEMORY_POOL "Enable CUDA memory pool" ON)
if(ENABLE_MEMORY_POOL)
    add_definitions(-DENABLE_MEMORY_POOL)
    message(STATUS "  CUDA memory pool enabled")
endif()

# 错误检查选项
option(ENABLE_CUDA_ERROR_CHECKING "Enable comprehensive CUDA error checking" ON)
if(ENABLE_CUDA_ERROR_CHECKING)
    add_definitions(-DENABLE_CUDA_ERROR_CHECKING)
    message(STATUS "  CUDA error checking enabled")
endif()

# 构建测试选项
option(BUILD_CUDA_TESTS "Build CUDA sequence trainer tests" OFF)
if(BUILD_CUDA_TESTS)
    enable_testing()
    add_subdirectory(tests)
    message(STATUS "  CUDA tests enabled")
endif()

# 构建示例选项
option(BUILD_CUDA_EXAMPLES "Build CUDA sequence trainer examples" OFF)
if(BUILD_CUDA_EXAMPLES)
    add_subdirectory(examples)
    message(STATUS "  CUDA examples enabled")
endif()

# 依赖检查
if(NOT CUDAToolkit_FOUND)
    message(FATAL_ERROR "CUDA not found. Please install CUDA toolkit.")
endif()

if(NOT TARGET CUDA::cudnn)
    message(FATAL_ERROR "cuDNN not found. Please install cuDNN library.")
endif()

# 输出构建完成信息
message(STATUS "CUDA Sequence Trainer CMake configuration completed")