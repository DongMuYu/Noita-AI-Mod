cmake_minimum_required(VERSION 3.16)
project(aiDev)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置SFML路径
list(APPEND CMAKE_PREFIX_PATH "D:/Libraries/SFML-2.5.1")

if(MSVC)
    add_compile_options(/utf-8)  # 强制MSVC使用UTF-8
endif()

# 查找SFML组件
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# 显式列出源文件
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/core/Game.cpp
    src/core/Window.cpp
    src/core/SafetyChecker.cpp
    src/core/UI.cpp
    src/core/Map.cpp
    src/core/Renderer.cpp
    # src/core/Resource.cpp
    # src/core/Level.cpp
    # src/core/Time.cpp
    src/entity/Player.cpp
    src/entity/Enemy.cpp
    src/entity/Bullet.cpp
    src/physics/Collision.cpp
    src/ai/pathfinding/RayCasting.cpp

    src/scene/GameScene.cpp
    src/world/Parser.cpp
)

# 链接SFML库
target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

# 定义资源目录
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)