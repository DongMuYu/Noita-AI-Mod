// PointCloudMapper.h

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "../core/Constants.h"
#include "../pathfinding/RayCasting.h"

/**
 * @brief 激光雷达点云地形建模系统
 * @details 通过射线检测构建环境的点云地图，支持实时更新和可视化
 * 类似于激光雷达的工作原理，通过扫描获取环境的三维信息
 */
class PointCloudMapper {
private:
    /**
     * @brief 点云数据结构
     * @details 使用哈希表存储每个瓦片位置的扫描信息
     * 键：瓦片坐标对 (x, y)
     * 值：扫描次数和置信度
     */
    struct PointData {
        int scanCount = 0;          ///< 该位置被扫描到的次数
        float confidence = 0.0f;    ///< 置信度（0-1）
        bool isObstacle = false;    ///< 是否确认为障碍物
        sf::Color color;            ///< 显示颜色（基于置信度）
    };

    struct PairHash {
        size_t operator()(const std::pair<int, int>& p) const {
            return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
        }
    };
    
    std::unordered_map<std::pair<int, int>, PointData, PairHash> pointCloud;
    
    std::vector<sf::Vector2f> scanPoints;   ///< 所有扫描点的世界坐标
    std::vector<sf::Vertex> pointVertices;  ///< 用于渲染的点云顶点
    
    sf::RenderTexture pointCloudTexture;    ///< 点云纹理缓存
    bool textureDirty = true;              ///< 纹理是否需要更新

public:
    /**
     * @brief 构造函数
     * @details 初始化点云映射器，设置渲染目标
     */
    PointCloudMapper();
    
    /**
     * @brief 添加扫描点
     * @param worldPos 扫描点的世界坐标
     * @param isObstacle 是否为障碍物
     * @details 将扫描点添加到点云数据结构中，更新置信度
     */
    void addScanPoint(const sf::Vector2f& worldPos, bool isObstacle);
    
    /**
     * @brief 批量添加射线扫描结果
     * @param rayHits 射线检测结果数组
     * @details 处理整个射线扫描的结果，批量更新点云数据
     */
    void addRayScanResults(const std::vector<RayHitInfo>& rayHits);
    
    /**
     * @brief 获取点云地图
     * @return 返回点云数据的引用
     */
    const std::unordered_map<std::pair<int, int>, PointData, PairHash>& getPointCloud() const;
    
    /**
     * @brief 清空点云数据
     * @details 重置所有扫描数据，重新开始地形建模
     */
    void clear();
    
    /**
     * @brief 渲染点云地图
     * @param window 渲染目标窗口
     * @param playerPos 玩家当前位置（用于相对显示）
     */
    void render(sf::RenderWindow& window, const sf::Vector2f& playerPos);
    
    /**
     * @brief 获取地图边界
     * @return 返回点云地图的边界矩形
     */
    sf::FloatRect getBounds() const;
    
    /**
     * @brief 导出点云数据
     * @param filename 导出文件名
     * @details 将点云数据导出为CSV格式，便于分析和调试
     */
    void exportToCSV(const std::string& filename) const;
};