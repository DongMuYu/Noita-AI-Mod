// Game.cpp

#include "Game.h"
#include "../physics/Collision.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "../world/Parser.h"

using namespace std;

/**
 * @brief 游戏类构造函数
 * @details 初始化游戏窗口、玩家对象和资源加载
 * 根据关卡数据动态计算窗口大小，并配置显示参数
 * 设置垂直同步和帧率限制以确保跨设备性能一致性
 */
Game::Game()
    : window(W * TILE + 200, H * TILE, "NoitaSimulator - 主游戏", "NoitaSimulator - 点云地图"),
      player(sf::Vector2f(0, 0))  
    {
    initResources();                       // 初始化游戏资源

    // 初始化渲染器
    if (!renderer.initialize()) {
        cout << "渲染器初始化失败" << endl;
    }

    // 初始化UI
    ui.initialize(renderer.getFont());

    // 初始化调试标志
    showRayDebug = false;
    paused = false;
    totalPausedTime = 0.0f;
    
    // 设置文本样式（使用渲染器中的字体）
    timerText.setFont(renderer.getFont());
    fpsText.setFont(renderer.getFont());
    timerText.setCharacterSize(18);
    fpsText.setCharacterSize(18);
    timerText.setFillColor(sf::Color::Red);
    fpsText.setFillColor(sf::Color::Blue);
    
    // 初始化玩家到终点的连接线
    playerTargetLine.setPrimitiveType(sf::Lines);
    playerTargetLine.resize(2); // 两个顶点：起点(玩家)和终点(目标)
}

/**
 * @brief 游戏类析构函数
 * @details 使用默认实现，自动释放所有成员变量资源
 */
Game::~Game() = default;

/**
 * @brief 初始化游戏资源
 * @details 解析关卡数据并创建游戏元素：
 * 1. 创建平台瓦片并设置位置和样式
 * 2. 定位玩家初始位置(P标记)
 * 3. 记录目标点位置(T标记)
 */
/**
 * @brief 初始化游戏资源和关卡数据
 * @details 解析关卡文件，创建瓦片地图，设置玩家初始位置和目标点位置
 * 该函数在游戏启动和关卡重置时被调用，负责构建游戏世界的初始状态
 * 同时初始化AI控制器，建立游戏世界与玩家的关联
 */
void Game::initResources() {    

    // 确保地图数据已加载 - 直接触发地图初始化
    map.resetMap();
    map.draw(window.getMainWindow());

    // 初始化玩家位置为安全默认值
    sf::Vector2f playerPos = map.getPlayerPos();
    sf::Vector2f targetPos = map.getTargetPosition();
    
    player.setPosition(playerPos);
    playerSpawnPosition = playerPos;
    
    // 注册玩家到安全检查器
    safetyChecker.registerEntity("player", [this]() -> Entity* { return &this->player; });

}

/**
 * @brief 处理用户输入
 * @param dt 时间步长(秒)，用于平滑控制
 * @details 响应键盘输入并转换为玩家动作：
 * - A/D键控制水平移动
 * - W键控制上升/飞行
 * 输入处理与帧率无关，使用时间步长确保移动速度一致
 */
/**
 * @brief 处理玩家输入并转换为游戏内动作
 * 
 * 检测键盘按键状态，计算移动方向向量，并更新玩家速度
 * 支持水平移动(A/D键)和上升(W键)操作
 * 
 * @param dt 时间增量(秒)，用于基于时间的输入处理
 */
void Game::handleInput(float dt) {
    // 处理窗口事件
    window.handleEvents();

    // 处理玩家输入
    player.handleInput(dt);
    
    // 射线调试开关
    static bool rPressed = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
        if (!rPressed) {
            showRayDebug = !showRayDebug;
            rPressed = true;
        }
    } else {
        rPressed = false;
    }
}

/**
 * @brief 更新游戏状态
 * @param dt 时间步长(秒)
 * @details 游戏主逻辑更新函数，处理：
 * 1. 玩家状态更新
 * 2. 碰撞检测与响应
 * 3. 目标点到达检测
 * 4. 边界检查与处理
 */
void Game::update(float dt) {
    // 物理更新：处理重力、速度积分和动画状态
    player.update(dt);
    player.setOnGround(false);  // 重置地面状态标记，碰撞检测阶段会重新评估
                                //碰撞检测存在概率性失败 通过该句建立安全默认值，避免检测失败导致穿越平台        

    // 准备碰撞检测数据
    PlayerCollisionData cd{player.getShapeRef(), player.getVelocity(), player.isOnGround()};
    // 处理玩家与平台碰撞检测与响应
    // 计算碰撞后的新位置和速度，并更新地面状态
    handlePlayerPlatformCollision(cd, map.getTiles(), map.getLevelData());
    
    // 更新玩家状态(碰撞后)
    player.setPosition(cd.shape.getPosition());      // 更新玩家位置
    player.setVelocity(cd.velocity); // 更新玩家速度
    player.setOnGround(cd.onGround); // 更新地面状态

    // 检测玩家是否到达目标点
    sf::Vector2f targetPos = map.getTargetPosition();
    sf::FloatRect targetBounds(targetPos, sf::Vector2f(TILE, TILE));
    if (player.getShape().getGlobalBounds().intersects(targetBounds)) {
        resetLevel();         // 重置关卡状态
        return;
    }

    // 边界检查：防止玩家掉落到地图边界外
    // 使用实际地图尺寸进行边界检测（左上角坐标系）
    auto playerBounds = player.getShape().getGlobalBounds();
    
    // 计算实际地图尺寸（像素）
    const auto& levelData = map.getLevelData();
    float mapWidth = levelData[0].size() * TILE;
    float mapHeight = levelData.size() * TILE;
    
    
    // 更新射线检测
    if (showRayDebug) {
        sf::Vector2f playerPos = player.getPosition() + sf::Vector2f(TILE/2, TILE/2);
        rayHits = rayCaster.castRays(playerPos, map.getLevelData(), 8);
    }
    
    // 运行时安全检查
    if (safetyChecker.updateEntitySafety("player", map.getLevelData(), map.getTiles(), dt)) {
        resetLevel();
    }
}

/**
 * @brief 渲染游戏画面
 * @details 游戏渲染主函数，按以下层级绘制：
 * 1. 清空窗口为白色背景
 * 2. 绘制所有平台瓦片
 * 3. 绘制玩家对象
 * 4. 绘制目标点(红色正方形)
 * 5. 绘制UI元素(计时器和FPS显示)
 * 最后显示渲染结果
 */
void Game::render() {
    sf::RenderWindow& mainWindowRef = window.getMainWindow();
    
    // 更新渲染器状态
    renderer.setPlayerSpawnPosition(playerSpawnPosition);
    
    auto playerSafety = safetyChecker.getEntitySafety("player");
    renderer.setDangerState(playerSafety.isInDanger, playerSafety.dangerTimer);
    
    // 使用新的渲染器接口渲染完整游戏画面
    renderer.renderMainWindow(mainWindowRef, map, player, ui, gameTime, fps, showRayDebug, rayHits);
}

/**
 * @brief 游戏主循环
 * @details 控制游戏生命周期的核心函数，实现：
 * 1. 事件处理循环(窗口关闭、键盘输入等)
 * 2. FPS计算与显示更新
 * 3. 游戏状态更新控制(暂停/帧步模式)
 * 4. 固定时间步长的游戏逻辑更新
 * 5. 画面渲染
 * 循环持续到窗口关闭
 */
void Game::resetLevel() {
    // 重新初始化地图资源 - 强制重新加载关卡数据
    map.resetMap();
    map.draw(window.getMainWindow());
    
    // 重置安全检查器状态
    safetyChecker.resetEntitySafety("player");
    
    // 重新获取玩家初始位置和目标位置
    sf::Vector2f playerPos = map.getPlayerPos();
    sf::Vector2f targetPos = map.getTargetPosition();
    
    player.setPosition(playerPos);
    playerSpawnPosition = playerPos;
    
    // 重置秒表
    gameClock.restart();
    totalPausedTime = 0.0f;
    return;
}

/**
 * @brief 游戏主循环
 * @details 控制游戏生命周期的核心函数，实现：
 * 1. 事件处理循环(窗口关闭、键盘输入等)
 * 2. FPS计算与显示更新
 * 3. 游戏状态更新控制(暂停/帧步模式)
 * 4. 固定时间步长的游戏逻辑更新
 * 5. 画面渲染
 * 循环持续到窗口关闭
 */
void Game::run() {
    sf::Clock clock;
    fpsClock.restart();
    gameClock.restart();

    while (window.isMainWindowOpen()) {
        // 处理窗口事件
        window.handleEvents();
        
        // 处理键盘输入 - F键暂停/继续
        static bool fPressed = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
            if (!fPressed) {
                paused = !paused;
                fPressed = true;
            }
        } else {
            fPressed = false;
        }
        

        // 更新FPS计数器
    frameCount++;
    if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
        fps = frameCount / fpsClock.restart().asSeconds();  // 计算FPS(帧数/秒)
        frameCount = 0;                                    // 重置帧计数器
        fpsText.setString("FPS: " + std::to_string(static_cast<int>(fps)));
    }

        // 更新游戏时间(排除暂停时间)
    if (!paused) {
        gameTime = gameClock.getElapsedTime().asSeconds() - totalPausedTime;
    }
    timerText.setString("Time: " + std::to_string(static_cast<int>(gameTime)) + "s");

        // 游戏逻辑更新控制
        float dt = clock.restart().asSeconds();  // 获取自上次循环以来的时间增量
        
        if (dt > fixedTimeStep) {
            dt = fixedTimeStep;
        }
        
        if (!paused) {
            handleInput(dt);  // 处理用户输入
            update(dt);       // 更新游戏状态
        }

        render();
    }
}