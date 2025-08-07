#pragma once

#include <SFML/System.hpp>

/**
 * @brief 时间管理类 - 封装游戏时间相关的所有功能
 * 
 * 该类负责管理游戏运行时间、FPS计算、暂停状态等时间相关功能。
 * 提供统一的时间接口，避免在多个地方重复实现时间逻辑。
 */
class TimeManager {
public:
    /**
     * @brief 构造函数
     * 
     * 初始化时间管理器，设置默认的时间参数。
     */
    TimeManager();
    
    /**
     * @brief 析构函数
     * 
     * 清理资源，使用默认实现。
     */
    ~TimeManager() = default;
    
    /**
     * @brief 更新时间管理器
     * 
     * 每帧调用一次，更新所有时间相关的计算。
     * 包括FPS计算、游戏时间更新等。
     */
    void update();
    
    /**
     * @brief 重置所有计时器
     * 
     * 重置游戏时间、FPS计数器等，用于关卡重置或游戏重启。
     */
    void reset();
    
    /**
     * @brief 暂停/继续游戏
     * 
     * @param paused true暂停游戏，false继续游戏
     */
    void setPaused(bool paused);
    
    /**
     * @brief 获取当前游戏时间（秒）
     * 
     * @return 当前游戏运行时间，排除暂停时间
     */
    float getGameTime() const;
    
    /**
     * @brief 获取当前FPS
     * 
     * @return 当前帧率
     */
    int getFPS() const;
    
    /**
     * @brief 获取上一帧的时间增量
     * 
     * @return 上一帧的耗时（秒）
     */
    float getDeltaTime() const;
    
    /**
     * @brief 检查游戏是否暂停
     * 
     * @return true游戏暂停，false游戏运行中
     */
    bool isPaused() const;
    
    /**
     * @brief 设置固定时间步长
     * 
     * @param step 最大允许的时间步长（秒）
     */
    void setFixedTimeStep(float step);

private:
    sf::Clock gameClock;        ///< 游戏主时钟
    sf::Clock fpsClock;         ///< FPS计算时钟
    sf::Clock deltaClock;       ///< 帧时间增量时钟
    
    float gameTime;           ///< 当前游戏时间（秒）
    float totalPausedTime;    ///< 总暂停时间（秒）
    float deltaTime;          ///< 上一帧时间增量（秒）
    float fixedTimeStep;      ///< 固定时间步长限制
    
    int fps;                  ///< 当前FPS值
    int frameCount;           ///< 帧计数器
    bool paused;              ///< 暂停状态标志
    
    /**
     * @brief 更新FPS计算
     * 
     * 每秒更新一次FPS显示。
     */
    void updateFPS();
};