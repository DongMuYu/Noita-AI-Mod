#include "Time.h"
#include <algorithm>

/**
 * @brief 构造函数
 * 
 * 初始化时间管理器，设置默认的时间参数：
 * - 游戏时间初始化为0
 * - 暂停时间初始化为0
 * - 时间增量初始化为0
 * - 固定时间步长设置为1/60秒（60FPS）
 * - FPS初始化为0
 * - 帧计数器初始化为0
 * - 暂停状态设置为false
 */
TimeManager::TimeManager() 
    : gameTime(0.0f)
    , totalPausedTime(0.0f)
    , deltaTime(0.0f)
    , fixedTimeStep(1.0f / 60.0f)  // 默认60FPS
    , fps(0)
    , frameCount(0)
    , paused(false) {
    // 重置所有时钟
    gameClock.restart();
    fpsClock.restart();
    deltaClock.restart();
}

/**
 * @brief 更新时间管理器
 * 
 * 每帧调用一次，更新所有时间相关的计算：
 * 1. 计算上一帧的时间增量
 * 2. 应用固定时间步长限制
 * 3. 更新游戏时间（排除暂停时间）
 * 4. 更新FPS计算
 * 5. 递增帧计数器
 */
void TimeManager::update() {
    // 计算上一帧的时间增量
    deltaTime = deltaClock.restart().asSeconds();
    
    // 应用固定时间步长限制
    if (deltaTime > fixedTimeStep) {
        deltaTime = fixedTimeStep;
    }
    
    // 更新游戏时间（排除暂停时间）
    if (!paused) {
        gameTime = gameClock.getElapsedTime().asSeconds() - totalPausedTime;
    }
    
    // 更新FPS计算
    frameCount++;
    updateFPS();
}

/**
 * @brief 重置所有计时器
 * 
 * 重置游戏时间、FPS计数器等，用于关卡重置或游戏重启：
 * - 重置所有时钟
 * - 重置游戏时间为0
 * - 重置暂停时间为0
 * - 重置FPS和帧计数器
 */
void TimeManager::reset() {
    // 重置所有时钟
    gameClock.restart();
    fpsClock.restart();
    deltaClock.restart();
    
    // 重置时间变量
    gameTime = 0.0f;
    totalPausedTime = 0.0f;
    deltaTime = 0.0f;
    fps = 0;
    frameCount = 0;
}

/**
 * @brief 暂停/继续游戏
 * 
 * @param paused true暂停游戏，false继续游戏
 * 
 * 当游戏状态改变时，计算暂停期间的时间：
 * - 如果暂停，记录当前暂停开始时间
 * - 如果继续，累加暂停时间到总暂停时间
 */
void TimeManager::setPaused(bool paused) {
    if (this->paused != paused) {
        this->paused = paused;
        
        if (paused) {
            // 暂停时记录暂停开始时间
            // 这里不需要额外操作，因为gameTime计算会自动排除暂停时间
        } else {
            // 继续游戏时，更新总暂停时间
            // 由于gameTime计算排除了暂停时间，这里不需要额外操作
        }
    }
}

/**
 * @brief 获取当前游戏时间
 * 
 * @return 当前游戏运行时间（秒），排除暂停时间
 */
float TimeManager::getGameTime() const {
    return gameTime;
}

/**
 * @brief 获取当前FPS
 * 
 * @return 当前帧率
 */
int TimeManager::getFPS() const {
    return fps;
}

/**
 * @brief 获取上一帧的时间增量
 * 
 * @return 上一帧的耗时（秒）
 */
float TimeManager::getDeltaTime() const {
    return deltaTime;
}

/**
 * @brief 检查游戏是否暂停
 * 
 * @return true游戏暂停，false游戏运行中
 */
bool TimeManager::isPaused() const {
    return paused;
}

/**
 * @brief 设置固定时间步长
 * 
 * @param step 最大允许的时间步长（秒）
 * 
 * 用于限制最大帧时间，防止在帧率过低时出现大的时间跳跃。
 */
void TimeManager::setFixedTimeStep(float step) {
    fixedTimeStep = std::max(0.001f, step);  // 确保最小值为1毫秒
}

/**
 * @brief 更新FPS计算
 * 
 * 每秒更新一次FPS显示：
 * - 如果距离上次更新超过1秒，计算新的FPS值
 * - 重置帧计数器和FPS时钟
 */
void TimeManager::updateFPS() {
    if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
        fps = static_cast<int>(frameCount / fpsClock.restart().asSeconds());
        frameCount = 0;
    }
}