#ifndef RAYCASTING_H
#define RAYCASTING_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

/**
 * @brief 射线命中信息结构体
 * 存储射线投射的结果信息，包括命中点、距离、是否命中等关键数据
 */
struct RayHitInfo {
    sf::Vector2f hitPoint;    // 射线命中点的坐标（世界坐标系）
    float distance;          // 从起点到命中点的距离（像素单位）
    bool hit;               // 是否命中障碍物（true=命中，false=未命中）
    sf::Vector2f direction;  // 射线方向向量（已标准化）
};

/**
 * @brief 射线投射系统类
 * 提供高效的射线检测功能，用于AI视觉感知、碰撞检测、路径规划等场景
 * 支持360度全方位射线投射，可配置射线密度和最大距离
 */
class RayCasting {
public:
    /**
     * @brief 构造函数
     * 初始化射线投射系统，设置默认参数
     */
    RayCasting();
    
    /**
     * @brief 从指定位置进行全方位射线检测
     * @param origin 射线起点坐标（世界坐标系）
     * @param levelData 关卡数据，二维字符数组表示地形
     * @param raysPerQuadrant 每个象限发射的射线数量（默认4条）
     * @return 返回所有射线的命中信息数组
     * @note 总共发射raysPerQuadrant*4条射线，覆盖360度范围
     */
    std::vector<RayHitInfo> castRays(const sf::Vector2f& origin, 
                                   const std::vector<std::string>& levelData,
                                   int raysPerQuadrant = 4);
    
    /**
     * @brief 可视化射线投射结果（调试用）
     * @param window SFML渲染窗口
     * @param rays 射线命中信息数组
     * @param origin 射线起点坐标
     * @note 绿色线条表示射线，黄色圆点表示命中点
     */
    void drawRays(sf::RenderWindow& window, const std::vector<RayHitInfo>& rays, const sf::Vector2f& origin);
    
private:
    /**
     * @brief 检查指定网格坐标是否为障碍物
     * @param levelData 关卡数据
     * @param x 网格X坐标
     * @param y 网格Y坐标
     * @return true表示该位置是障碍物，false表示可通过
     */
    bool isObstacle(const std::vector<std::string>& levelData, int x, int y);
    
    /**
     * @brief 投射单条射线
     * @param origin 射线起点
     * @param direction 射线方向向量（需标准化）
     * @param levelData 关卡数据
     * @return 返回该射线的命中信息
     * @note 使用步进算法，逐步检测路径上的障碍物
     */
    RayHitInfo castSingleRay(const sf::Vector2f& origin, 
                           const sf::Vector2f& direction,
                           const std::vector<std::string>& levelData);
    
    static constexpr float MAX_DISTANCE = 150.0f;  // 最大射线距离（像素单位）
    static constexpr float STEP_SIZE = 1.0f;      // 射线步长（像素单位），影响精度和性能
};

#endif // RAYCASTING_H