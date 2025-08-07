// Renderer.h

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "../ai/pathfinding/RayCasting.h"

// 前向声明
class Map;
class Player;
class UI;

/**
 * @brief 增强版渲染器类
 * @details 负责游戏所有视觉元素的渲染，整合了原Game.cpp中的渲染逻辑
 * 提供统一的渲染接口，包括地图、玩家、UI、调试信息等
 */
class Renderer {
public:
    Renderer();
    ~Renderer();
    
    /**
     * @brief 初始化渲染资源
     * @return 初始化是否成功
     */
    bool initialize();
    
    /**
     * @brief 获取字体对象
     * @return 字体对象的引用
     */
    sf::Font& getFont();
    
    /**
     * @brief 渲染完整游戏画面
     * @param window SFML渲染窗口
     * @param map 地图对象
     * @param player 玩家对象
     * @param ui UI对象
     * @param gameTime 游戏时间
     * @param fps 帧率
     * @param showRayDebug 是否显示射线调试
     * @param rayHits 射线碰撞数据
     */
    void renderMainWindow(sf::RenderWindow& window, 
                        Map& map, 
                        Player& player, 
                        UI& ui,
                        float gameTime, 
                        float fps,
                        bool showRayDebug,
                        const std::vector<RayHitInfo>& rayHits);
    
    /**
     * @brief 清空屏幕
     * @param window SFML渲染窗口
     * @param color 背景颜色
     */
    void clear(sf::RenderWindow& window, const sf::Color& color = sf::Color::Black);
    
    /**
     * @brief 显示渲染结果
     * @param window SFML渲染窗口
     */
    void display(sf::RenderWindow& window);
    
    /**
     * @brief 设置玩家出生点位置（用于渲染出生点标记）
     * @param pos 玩家出生点坐标
     */
    void setPlayerSpawnPosition(const sf::Vector2f& pos);
    
    /**
     * @brief 设置危险状态（用于渲染危险警告）
     * @param isInDanger 是否处于危险状态
     * @param dangerTimer 危险计时器
     */
    void setDangerState(bool isInDanger, float dangerTimer);

private:
    /**
     * @brief 渲染地图瓦片
     */
    void renderMap(sf::RenderWindow& window, Map& map);
    
    /**
     * @brief 渲染玩家对象
     */
    void renderPlayer(sf::RenderWindow& window, Player& player, UI& ui);
    
    /**
     * @brief 渲染目标点
     */
    void renderTarget(sf::RenderWindow& window, Map& map, UI& ui);
    
    /**
     * @brief 渲染能量条
     */
    void renderEnergyBar(sf::RenderWindow& window, 
                        UI& ui, 
                        float currentEnergy, 
                        float maxEnergy, 
                        const sf::Vector2u& windowSize);
    
    /**
     * @brief 渲染计时器
     */
    void renderTimer(sf::RenderWindow& window, 
                    UI& ui, 
                    float gameTime, 
                    const sf::Vector2f& position);
    
    /**
     * @brief 渲染FPS
     */
    void renderFPS(sf::RenderWindow& window, 
                  UI& ui, 
                  float fps, 
                  const sf::Vector2f& position);
    
    /**
     * @brief 渲染玩家到目标的连线
     */
    void renderPlayerTargetLine(sf::RenderWindow& window, 
                               UI& ui,
                               const sf::Vector2f& playerPos,
                               const sf::Vector2f& targetPos);
    
    /**
     * @brief 渲染射线调试信息
     */
    void renderRayDebug(sf::RenderWindow& window, 
                       UI& ui,
                       const std::vector<RayHitInfo>& rayHits,
                       const sf::Vector2f& playerPos);
    
    /**
     * @brief 渲染危险警告
     */
    void renderDangerWarning(sf::RenderWindow& window, 
                            UI& ui, 
                            const sf::Vector2u& windowSize);
    
    /**
     * @brief 渲染玩家出生点标记
     */
    void renderPlayerSpawnPoint(sf::RenderWindow& window, UI& ui, sf::Vector2f& position);
    
    /**
     * @brief 渲染墙标记
     * @param window SFML渲染窗口
     * @param map 地图对象
     * @param wallMap 墙标记信息
     */
    void renderWallMarkers(sf::RenderWindow& window, Map& map, const std::vector<std::string>& wallMap);

    // 成员变量
    sf::Font font;
    bool initialized;
    sf::Vector2f playerSpawnPosition;
    bool playerInDanger;
    float dangerTimer;
    
    // 预定义颜色常量 - 在.cpp文件中定义
};