/**
 * @file RayCasting.cpp
 * @brief 射线投射系统实现文件
 * 提供完整的射线检测功能实现，包括360度全方位射线投射和可视化
 */

#include "RayCasting.h"
#include "../../core/Constants.h"
#include <algorithm>
#include <set>

/**
 * @brief 构造函数
 * 初始化射线投射系统，无需特殊初始化
 */
RayCasting::RayCasting() {}

/**
 * @brief 从指定位置进行全方位射线检测
 * @param origin 射线起点坐标（世界坐标系）
 * @param levelData 关卡数据，二维字符数组表示地形
 * @param raysPerQuadrant 每个象限发射的射线数量
 * @return 返回所有射线的命中信息数组
 * 
 * @details
 * - 将360度分为4个象限，每个象限发射指定数量的射线
 * - 射线角度在象限内均匀分布
 * - 总共发射raysPerQuadrant*4条射线
 * - 使用castSingleRay投射每条射线
 */
std::vector<RayHitInfo> RayCasting::castRays(const sf::Vector2f& origin, 
                                             const std::vector<std::string>& levelData,
                                             int raysPerQuadrant) const {
    std::vector<RayHitInfo> results;
    
    // 空关卡直接返回空结果
    if (levelData.empty()) return results;
    
    // 定义四个象限的角度范围（弧度制）
    const float PI = 3.14159265358979323846f;
    const float quadrantAngles[4][2] = {
        {0, PI/2},           // 第一象限 (0-90度，右上)
        {PI/2, PI},          // 第二象限 (90-180度，左上)
        {PI, 3*PI/2},        // 第三象限 (180-270度，左下)
        {3*PI/2, 2*PI}       // 第四象限 (270-360度，右下)
    };
    
    /**
     * @brief 为每个象限发射射线
     * 遍历4个象限，在每个象限内均匀发射指定数量的射线
     */
    for (int quadrant = 0; quadrant < 4; ++quadrant) {
        float startAngle = quadrantAngles[quadrant][0];
        float endAngle = quadrantAngles[quadrant][1];
        
        // 在象限内均匀分布射线角度
        for (int i = 0; i < raysPerQuadrant; ++i) {
            // 计算当前射线的角度（线性插值）
            float angle = startAngle + (endAngle - startAngle) * i / (raysPerQuadrant);
            
            // 计算方向向量
            sf::Vector2f direction(std::cos(angle), std::sin(angle));
            
            // 投射单条射线并收集结果
            RayHitInfo hit = const_cast<RayCasting*>(this)->castSingleRay(origin, direction, levelData);
            results.push_back(hit);
        }
    }
    
    return results;
}

/**
 * @brief 投射单条射线
 * @param origin 射线起点坐标
 * @param direction 射线方向向量（需标准化）
 * @param levelData 关卡数据
 * @return 返回该射线的详细命中信息
 * 
 * @details
 * 使用步进算法逐步检测射线路径上的每个点：
 * 1. 从起点开始，沿方向向量逐步前进
 * 2. 每一步检测当前位置是否超出边界
 * 3. 检查当前网格位置是否为障碍物
 * 4. 如果命中障碍物，记录命中信息并返回
 * 5. 如果到达最大距离仍未命中，返回未命中状态
 */
RayHitInfo RayCasting::castSingleRay(const sf::Vector2f& origin, 
                                     const sf::Vector2f& direction,
                                     const std::vector<std::string>& levelData) {
    RayHitInfo result;
    result.direction = direction;
    result.hit = false;
    result.distance = MAX_DISTANCE;
    
    sf::Vector2f currentPos = origin;
    
    // 射线步进检测：从起点开始逐步前进
    for (float distance = 0; distance < MAX_DISTANCE; distance += STEP_SIZE) {
        // 计算当前射线位置
        currentPos = origin + direction * distance;
        
        // 将世界坐标转换为网格坐标
        int gridX = static_cast<int>(currentPos.x / TILE);
        int gridY = static_cast<int>(currentPos.y / TILE);
        
        // 检查是否超出关卡边界
        if (gridX < 0 || gridY < 0 || 
            gridY >= static_cast<int>(levelData.size()) ||
            gridX >= static_cast<int>(levelData[gridY].size())) {
            break;  // 超出边界，停止检测
        }
        
        // 检查当前位置是否为障碍物
        if (isObstacle(levelData, gridX, gridY)) {
            result.hit = true;
            result.hitPoint = currentPos;
            result.distance = distance;
            break;  // 命中障碍物，返回结果
        }
    }
    
    // 如果未命中任何障碍物，设置终点为最大距离处
    if (!result.hit) {
        result.hitPoint = origin + direction * MAX_DISTANCE;
        result.distance = MAX_DISTANCE;
    }
    
    return result;
}

/**
 * @brief 检查指定网格坐标是否为障碍物
 * @param levelData 关卡数据，二维字符数组
 * @param x 网格X坐标
 * @param y 网格Y坐标
 * @return true表示该位置是障碍物，false表示可通过
 * 
 * @details
 * - 如果坐标超出关卡边界，视为障碍物（安全策略）
 * - 字符'1'表示普通障碍物
 * - 字符'M'表示特殊障碍物（如移动平台或特殊地形）
 */
bool RayCasting::isObstacle(const std::vector<std::string>& levelData, int x, int y) {
    // 检查边界：超出关卡边界视为障碍物
    if (y < 0 || y >= static_cast<int>(levelData.size()) ||
        x < 0 || x >= static_cast<int>(levelData[y].size())) {
        return true; // 边界外视为障碍物（安全策略）
    }
    
    // 获取该位置的字符
    char cell = levelData[y][x];
    
    // '1'表示普通障碍物，'M'表示特殊障碍物
    return (cell == '1' || cell == 'M' || cell == 'W' || cell == '3' || cell == '4'); // 可根据需要扩展更多障碍物类型
}

/**
 * @brief 可视化射线投射结果（调试用）
 * @param window SFML渲染窗口
 * @param rays 射线命中信息数组
 * @param origin 射线起点坐标
 * 
 * @details
 * - 绿色线条：从起点到命中点的完整射线
 * - 黄色圆点：射线命中障碍物时的命中点
 * - 黄色半透明矩形：被射线检测到的障碍物瓦片
 * - 圆点半径为3像素，便于观察
 */
void RayCasting::drawRays(sf::RenderWindow& window, const std::vector<RayHitInfo>& rays, const sf::Vector2f& origin) {
    // 用于存储被射线命中的瓦片位置，避免重复绘制
    std::set<std::pair<int, int>> hitTiles;
    
    // 遍历所有射线进行绘制
    for (const auto& ray : rays) {
        // 绘制从起点到命中点的完整射线（绿色线条）
        sf::Vertex line[] = {
            sf::Vertex(origin, sf::Color::Green),           // 起点
            sf::Vertex(ray.hitPoint, sf::Color::Green)     // 命中点
        };
        window.draw(line, 2, sf::Lines);
        
        // 如果射线命中障碍物，绘制命中点和被命中的瓦片
        if (ray.hit) {
            // 创建黄色圆点表示命中位置
            sf::CircleShape hitPoint(3);
            hitPoint.setFillColor(sf::Color::Yellow);
            hitPoint.setPosition(ray.hitPoint.x - 1.5f, ray.hitPoint.y - 1.5f);
            window.draw(hitPoint);
            
            // 计算命中的瓦片坐标
            int tileX = static_cast<int>(ray.hitPoint.x / TILE);
            int tileY = static_cast<int>(ray.hitPoint.y / TILE);
            
            // 避免重复绘制同一个瓦片
            if (hitTiles.find(std::make_pair(static_cast<int>(tileX), static_cast<int>(tileY))) == hitTiles.end()) {
                hitTiles.insert(std::make_pair(static_cast<int>(tileX), static_cast<int>(tileY)));
                
                // 创建黄色半透明矩形表示被命中的瓦片
                sf::RectangleShape tileRect(sf::Vector2f(TILE, TILE));
                tileRect.setFillColor(sf::Color(255, 255, 0, 128)); // 黄色半透明
                tileRect.setPosition(tileX * TILE, tileY * TILE);
                tileRect.setOutlineThickness(1);
                tileRect.setOutlineColor(sf::Color::Yellow);
                window.draw(tileRect);
            }
        }
    }
}