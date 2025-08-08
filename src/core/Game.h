// Game.h

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "../entity/Player.h"
#include "../physics/Collision.h"
#include "../ai/pathfinding/RayCasting.h"

#include "Renderer.h"
#include "SafetyChecker.h"
#include "Time.h"

#include "Window.h"
#include "UI.h"
#include "Map.h"
#include "Constants.h"
#include "../ai/controller/DataCollector.h"
#include "../ai/controller/AIController.h"

/**
 * @brief 游戏主控制器类
 * @details 负责游戏初始化、主循环控制、事件处理、更新和渲染等核心功能
 * 是整个游戏逻辑的中枢，协调玩家、AI控制器、游戏世界和物理系统之间的交互
 */
class Game {
private:
    /** @brief 窗口管理器 */
    Window window;
    
    /** @brief 玩家对象 */
    Player player;
    
    /** @brief 玩家初始出生位置 */
    sf::Vector2f playerSpawnPosition;
    
    /** @brief 时间管理器 */
    TimeManager timeManager; 
    
    /** @brief 计时器文本对象 */
    sf::Text timerText;
    
    /** @brief FPS显示文本对象 */
    sf::Text fpsText;
    
    /** @brief 玩家到终点的连接线 */
    sf::VertexArray playerTargetLine;
    
    /** @brief 射线检测系统 */
    RayCasting rayCaster;
    
    /** @brief 射线检测结果 */
    std::vector<RayHitInfo> rayHits;
    
    /** @brief 是否显示射线调试信息 */
    bool showRayDebug = false;
    
    /** @brief 游戏状态控制 - 暂停状态 */
    bool paused = false;
    
    /** @brief 安全检测器 */
    SafetyChecker safetyChecker;
    
    /** @brief UI管理器 */
    UI ui;

    /** @brief 地图对象 */
    Map map;

    /** @brief 渲染器对象 */
    Renderer renderer;
    
    /** @brief 数据收集器 */
    DataCollector dataCollector;
    
    /** @brief AI控制器 */
    AIController aiController;
    
    /** @brief AI控制模式开关 */
    bool aiMode = false;
    
    /** @brief 当前回合开始时间 */
    float episodeStartTime = 0.0f;
    
    /** @brief 当前回合帧计数 */
    int episodeFrameCount = 0;
    
    /** @brief 游戏局数计数器 */
    int totalGamesCount = 0;
    
    /** @brief 上一帧到目标的距离，用于计算奖励 */
    float lastDistanceToTarget = 0.0f;

    /**
     * @brief 初始化游戏资源
     * @details 加载关卡数据、创建瓦片地图、设置初始游戏状态
     * 此方法在游戏构造函数中被调用，完成所有必要的资源初始化
     */
    void initResources();
    
    /**
     * @brief 处理用户输入
     * @param dt 时间增量（秒）
     * @details 响应键盘输入，控制玩家移动、跳跃等行为
     * 支持暂停/继续游戏、帧步模式切换等调试功能
     */
    void handleInput(float dt);
    
    /**
     * @brief 更新游戏状态
     * @param dt 时间增量（秒）
     * @details 更新玩家状态、碰撞检测、游戏逻辑等
     * 在帧步模式下，仅当frameStepPending为true时才处理一帧
     */
    void update(float dt);
    
    /**
     * @brief 渲染游戏画面
     * @details 清空窗口、绘制所有游戏元素、更新显示
     * 绘制顺序：背景 -> 瓦片 -> 玩家 -> UI元素
     */
    void render();
    
    /**
     * @brief 重置关卡
     * @details 清空当前关卡数据，重新初始化资源，开始新关卡
     * 重置玩家位置、计时器和所有游戏状态变量
     */
    void resetLevel();
    
    /**
     * @brief 保存收集的数据到文件
     * @details 将当前收集的所有episode数据保存到二进制和CSV文件
     */
    void saveCollectedData();
    
public:
    /**
     * @brief 构造函数
     * @details 初始化游戏窗口、玩家对象和其他游戏资源
     * 设置窗口大小、标题和帧率限制，调用initResources()完成初始化
     */
    Game();
    
    /** @brief 析构函数 */
    ~Game();
    
    /**
     * @brief 启动游戏主循环
     * @details 包含输入处理、更新、渲染的完整游戏循环
     * 控制游戏帧率，计算deltaTime并处理暂停状态
     */
    void run();
};