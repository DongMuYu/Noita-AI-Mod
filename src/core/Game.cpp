// Game.cpp

/*
 * 按键功能说明
 * 
 * 玩家控制按键（键盘模式）：
 * - A键：向左移动（速度：160像素/秒）
 * - D键：向右移动（速度：160像素/秒）
 * - W键：上升/飞行（消耗能量，跳跃初速度：400像素/秒）
 * 
 * 游戏功能按键：
 * - F键：暂停/继续游戏（切换暂停状态）
 * - P键：切换AI控制模式（开启/关闭AI自动控制）
 * - R键：切换射线调试显示（显示/隐藏射线检测可视化）
 * - B键：切换数据收集功能（开启/关闭训练数据记录）
 * 
 * AI控制参数：
 * - AI水平移动：由AI控制器决定（范围：-1.0到1.0，乘以MOVE_SPEED）
 * - AI能量使用：由AI控制器决定（布尔值，决定是否消耗能量上升）
 * 
 * 物理常量：
 * - 重力加速度：1350像素/秒²
 * - 最大下落速度：由游戏逻辑限制
 * - 能量系统：最大能量150，消耗速率100单位/秒，地面恢复速率500单位/秒
 * 
 * 游戏世界尺寸：
 * - 地图宽度：90瓦片 × 15像素/瓦片 = 1350像素
 * - 地图高度：90瓦片 × 15像素/瓦片 = 1350像素
 * - 瓦片尺寸：15×15像素
 */

#include "Game.h"
#include "../physics/Collision.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <filesystem>
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
    
    // 初始化数据收集器
    dataCollector.setRecordingEnabled(false);
    std::cout << "[DEBUG] Data collection initialized and disabled" << std::endl;
    
    // 尝试加载已有的数据
    std::string dataPath = "D:\\steam\\steamapps\\common\\Noita\\mods\\NoitaCoreAI\\aiDev\\data\\sequence_data\\collected_data.bin";
    if (std::filesystem::exists(dataPath)) {
        std::cout << "[DEBUG] Loading existing data from " << dataPath << std::endl;
        dataCollector.loadEpisodeData(dataPath);
        std::cout << "[DEBUG] Loaded " << dataCollector.getTotalEpisodes() << " existing episodes" << std::endl;
    } else {
        std::cout << "[DEBUG] No existing data found, starting fresh collection" << std::endl;
    }
    
    // 初始化AI控制器并加载模型
    aiController.loadModel(AI_MODEL_PATH);
    std::cout << "[DEBUG] AI controller initialized and model loaded" << std::endl;
    
    // 初始化游戏局数计数器
    totalGamesCount = 0;
}

/**
 * @brief 游戏类析构函数
 * @details 使用默认实现，自动释放所有成员变量资源
 */
Game::~Game() {
    saveCollectedData();
}

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
    
    // 开始第一个回合，但默认不收集数据
    dataCollector.setRecordingEnabled(false);
    dataCollector.startEpisode();
    episodeStartTime = 0.0f;
    episodeFrameCount = 0;
    std::cout << "[DEBUG] New episode started - Data collection DISABLED by default" << std::endl;

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
    window.handleEvents();

    // 检查是否由AI控制
    if (aiMode) {
        // 普通AI模式 - 使用已训练的模型
        AIController::ActionResult result = aiController.decideActionWithDetails(player, map, rayCaster);
        
        // 调试输出AI动作信息（包含离散化和原始值）
        std::cout << "AI Action - Discrete: [moveX=" << result.action.moveX 
                  << ", useEnergy=" << result.action.useEnergy 
                  << "] | Raw: [moveX=" << result.originalData.moveX 
                  << ", useEnergy=" << result.originalData.useEnergy << "]" << std::endl;
        
        player.handleInput(dt, true, result.action.moveX, result.action.useEnergy);
    } else {
        player.handleInput(dt, false);
    }
    
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
    
    
    
    // 数据收集开关 - B键控制所有模式的数据收集
    static bool bPressed = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::B)) {
        if (!bPressed) {
            bool currentState = dataCollector.isRecordingEnabled();
            dataCollector.setRecordingEnabled(!currentState);
            
            if (!currentState) {
                // 开启收集时开始新回合
                dataCollector.startEpisode();
                episodeStartTime = timeManager.getGameTime();
                episodeFrameCount = 0;
                std::cout << "[DATA] Data collection ENABLED" << std::endl;
            } else {
                // 关闭收集时结束当前回合
                dataCollector.endEpisode(false, 0.0f, 0.0f);
                std::cout << "[DATA] Data collection DISABLED" << std::endl;
            }
            bPressed = true;
        }
    } else {
        bPressed = false;
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
    
    // 更新游戏状态(碰撞后)
    player.setPosition(cd.shape.getPosition());      // 更新玩家位置
    player.setVelocity(cd.velocity); // 更新玩家速度
    player.setOnGround(cd.onGround); // 更新地面状态
    
    // 增加帧计数
    episodeFrameCount++;

    // 收集训练数据
    if (dataCollector.isRecordingEnabled()) {
        // 监督学习模式 - 使用共享的数据收集器
        dataCollector.recordCurrentFrame(dataCollector.getCurrentFrameData(player, map, rayCaster));
        
        // 更新距离跟踪（从DataCollector内部获取最新距离）
        sf::Vector2f targetPos = map.getTargetPosition();
        sf::Vector2f diff = targetPos - player.getPosition();
        lastDistanceToTarget = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    }

    // 检测玩家是否到达目标点
    sf::Vector2f targetPos = map.getTargetPosition();
    sf::FloatRect targetBounds(targetPos, sf::Vector2f(TILE, TILE));
    if (player.getShape().getGlobalBounds().intersects(targetBounds)) {
        // 成功完成，记录数据
        float gameDuration = timeManager.getGameTime() - episodeStartTime;
        float averageFPS = (gameDuration > 0.0f) ? static_cast<float>(episodeFrameCount) / gameDuration : 0.0f;
        
        dataCollector.endEpisode(true, gameDuration, averageFPS);
        
        std::cout << "[DEBUG] Episode completed successfully!" << std::endl;
        std::cout << "[DEBUG] Duration: " << gameDuration << "s, FPS: " << averageFPS 
                  << ", Energy: " << player.getCurrentEnergy() << std::endl;
        
        resetLevel();         // 重置关卡状态
        return;
    }
    
    // 边界检查：防止玩家掉落到地图边界外
    // 使用实际地图尺寸进行边界检测（左上角坐标系）
    auto playerBounds = player.getShape().getGlobalBounds();
    
    // 计算实际地图尺寸（像素）
    const auto& levelData = map.getLevelData();
    
    // 更新射线检测
    if (showRayDebug) {
        sf::Vector2f playerPos = player.getPosition() + sf::Vector2f(TILE/2, TILE/2);
        rayHits = rayCaster.castRays(playerPos, map.getLevelData());
    }
    
    // 运行时安全检查
    if (safetyChecker.updateEntitySafety("player", map.getLevelData(), map.getTiles(), dt)) {
        // 安全检查失败，记录数据
        float gameDuration = timeManager.getGameTime() - episodeStartTime;
        float averageFPS = (gameDuration > 0.0f) ? static_cast<float>(episodeFrameCount) / gameDuration : 0.0f;
        
        dataCollector.endEpisode(false, gameDuration, averageFPS);
        
        std::cout << "[DEBUG] Episode ended due to safety check failure!" << std::endl;
        std::cout << "[DEBUG] Duration: " << gameDuration << "s, FPS: " << averageFPS 
                  << ", Energy: " << player.getCurrentEnergy() << std::endl;
        
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
    renderer.renderMainWindow(mainWindowRef, map, player, ui, 
                             timeManager.getGameTime(), 
                             static_cast<float>(timeManager.getFPS()), 
                             showRayDebug, rayHits);
    
    // 显示AI模式状态
    if (aiMode) {
        sf::Text aiModeText;
        aiModeText.setFont(renderer.getFont());
        aiModeText.setString("AI MODE ON");
        aiModeText.setCharacterSize(20);
        aiModeText.setFillColor(sf::Color::Green);
        aiModeText.setPosition(10, 10);
        mainWindowRef.draw(aiModeText);
    }
    
    
    
    // 显示数据收集状态
    sf::Text dataCollectionText;
    dataCollectionText.setFont(renderer.getFont());
    dataCollectionText.setString("Data: " + std::string(dataCollector.isRecordingEnabled() ? "ON" : "OFF"));
    dataCollectionText.setCharacterSize(14);
    dataCollectionText.setFillColor(dataCollector.isRecordingEnabled() ? sf::Color::Green : sf::Color::Red);
    dataCollectionText.setPosition(10, 75);
    mainWindowRef.draw(dataCollectionText);
}

/**
 * @brief 重置关卡状态
 * @details 重置游戏到初始状态，包括地图、玩家位置、时间等
 */
void Game::resetLevel() {
    // 结束当前数据收集回合
    if (episodeFrameCount > 0) {
        float gameDuration = timeManager.getGameTime() - episodeStartTime;
        float averageFPS = (gameDuration > 0.0f) ? static_cast<float>(episodeFrameCount) / gameDuration : 0.0f;
        dataCollector.endEpisode(false, gameDuration, averageFPS);
    }

    // 增加游戏局数计数
    totalGamesCount++;
    
    // 只有在数据收集启用时才自动保存
    if (dataCollector.isRecordingEnabled() && totalGamesCount % 5 == 0) {
        std::cout << "[AUTO-SAVE] Auto-saving data after " << totalGamesCount << " games..." << std::endl;
        saveCollectedData();
    }

    // 重新初始化地图资源 - 强制重新加载关卡数据
    map.resetMap();
    map.draw(window.getMainWindow());

    // 重置安全检查器状态
    safetyChecker.resetEntitySafety("player");

    // 重置距离跟踪
    lastDistanceToTarget = 0.0f;

    // 重新获取玩家初始位置和目标位置
    sf::Vector2f playerPos = map.getPlayerPos();
    sf::Vector2f targetPos = map.getTargetPosition();

    player.setPosition(playerPos);
    playerSpawnPosition = playerPos;

    // 重置时间管理器
    timeManager.reset();
    
    // 只有在数据收集启用时才开始新回合
    if (dataCollector.isRecordingEnabled()) {
        dataCollector.startEpisode();
        episodeStartTime = timeManager.getGameTime();
        episodeFrameCount = 0;
        std::cout << "[DEBUG] New episode started - Data collection ENABLED" << std::endl;
    } else {
        episodeStartTime = timeManager.getGameTime();
        episodeFrameCount = 0;
        std::cout << "[DEBUG] New episode started - Data collection DISABLED" << std::endl;
    }

    return;
}

/**
 * @brief 保存收集的数据
 * @details 将训练数据保存到文件，包括二进制数据和CSV格式
 */
void Game::saveCollectedData() {
    int totalEpisodes = dataCollector.getTotalEpisodes();
    if (totalEpisodes == 0) {
        std::cout << "[DEBUG] No episodes to save" << std::endl;
        return;
    }
    
    int successfulEpisodes = dataCollector.getSuccessfulEpisodes();
    float successRate = dataCollector.getSuccessRate();
    
    std::cout << "[DEBUG] Saving collected data..." << std::endl;
    std::cout << "[DEBUG] Total episodes collected: " << totalEpisodes << std::endl;
    std::cout << "[DEBUG] Successful episodes: " << successfulEpisodes << std::endl;
    std::cout << "[DEBUG] Success rate: " << (successRate * 100) << "%" << std::endl;
    
    std::string basePath = "D:\\steam\\steamapps\\common\\Noita\\mods\\NoitaCoreAI\\aiDev\\data\\sequence_data\\";
    
    dataCollector.saveEpisodeData(basePath + "collected_data.bin");
    dataCollector.exportTrainingDataset(basePath + "training_dataset.csv");
    
    std::cout << "[DEBUG] Data saved to " << basePath << "collected_data.bin and " << basePath << "training_dataset.csv" << std::endl;
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

    while (window.isMainWindowOpen()) {
        // 处理窗口事件
        window.handleEvents();
        
        // 更新时间管理器
        timeManager.update();

        // 处理键盘输入 - F键暂停/继续，P键切换AI模式
        static bool fPressed = false;
        static bool pPressed = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
            if (!fPressed) {
                paused = !paused;
                timeManager.setPaused(paused);
                fPressed = true;
            }
        } else {
            fPressed = false;
        }
        
        // P键切换AI控制模式
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
            if (!pPressed) {
                aiMode = !aiMode;
                aiController.setAIEnabled(aiMode);
                std::cout << "[AI] AI control mode: " << (aiMode ? "ENABLED" : "DISABLED") << std::endl;
                pPressed = true;
            }
        } else {
            pPressed = false;
        }

        // 游戏逻辑更新控制
    float dt = timeManager.getDeltaTime();
    
    if (!timeManager.isPaused()) {
        handleInput(dt);  // 处理用户输入
        update(dt);       // 更新游戏状态
    }

        render();
    }
}