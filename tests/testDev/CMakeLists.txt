cmake_minimum_required(VERSION 3.16)
project(sfml_test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# 让 CMake 找到 SFML
set(CMAKE_PREFIX_PATH "D:/Libraries/SFML-2.5.1")

find_package(SFML 2.5.1 COMPONENTS graphics window system REQUIRED)

add_executable(sfml_test
    sfml_test.cpp
    Square.cpp
    Triangle.cpp
    Circle.cpp
)

# 动态链接（默认 /MD）
target_link_libraries(sfml_test PRIVATE sfml-graphics sfml-window sfml-system)