#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include "../core/Constants.h"

class Entity;

/**
 * @brief 安全检测器类
 * 负责检测实体位置的安全性，防止出现卡住或掉出地图的情况
 * 通过实体接口获取位置、形状、尺寸等信息，避免手动计算
 */
class SafetyChecker {
public:
    struct SafetyResult {
        bool isSafe = true;
        std::string reason = "";
    };

    struct EntitySafety {
        bool isInDanger = false;
        float dangerTimer = 0.0f;
        sf::Vector2f lastSafePosition = {0.0f, 0.0f};
    };

    SafetyChecker() = default;
    
    /**
     * @brief 注册实体获取函数
     * @param entityId 实体ID
     * @param getter 获取实体对象的函数
     */
    void registerEntity(const std::string& entityId, std::function<Entity*()> getter);

    /**
     * @brief 验证生成位置是否有效
     * @param entityId 实体ID
     * @param levelData 关卡数据
     * @param tiles 瓦片集合
     * @return 位置是否有效
     */
    bool validateSpawnPosition(const std::string& entityId,
                              const std::vector<std::string>& levelData,
                              const std::vector<sf::RectangleShape>& tiles);

    /**
     * @brief 检查位置安全性
     * @param entityId 实体ID
     * @param levelData 关卡数据
     * @param tiles 瓦片集合
     * @return 安全检查结果
     */
    SafetyResult checkPositionSafety(const std::string& entityId,
                                   const std::vector<std::string>& levelData,
                                   const std::vector<sf::RectangleShape>& tiles);

    /**
     * @brief 更新实体安全状态
     * @param entityId 实体ID
     * @param levelData 关卡数据
     * @param tiles 瓦片集合
     * @param dt 时间增量
     * @return 是否需要重置位置
     */
    bool updateEntitySafety(const std::string& entityId,
                          const std::vector<std::string>& levelData,
                          const std::vector<sf::RectangleShape>& tiles,
                          float dt);

    /**
     * @brief 获取实体安全状态
     * @param entityId 实体ID
     * @return 安全状态信息
     */
    EntitySafety getEntitySafety(const std::string& entityId) const;

    /**
     * @brief 重置实体安全状态
     * @param entityId 实体ID
     */
    void resetEntitySafety(const std::string& entityId);

    /**
     * @brief 清除所有实体安全记录
     */
    void clearAllEntities();

private:
    std::unordered_map<std::string, EntitySafety> entitySafetyMap;
    std::unordered_map<std::string, std::function<Entity*()>> entityGetters;
    
    static constexpr float EDGE_THRESHOLD = 2.0f;   // 边缘安全距离（瓦片数）
    static constexpr float DANGER_RESET_TIME = 2.0f;  // 2秒后重置位置

    Entity* getEntity(const std::string& entityId) const;
    bool isWithinMapBounds(const Entity& entity, const std::vector<std::string>& levelData) const;
    bool isNearMapEdge(const Entity& entity, const std::vector<std::string>& levelData) const;
    bool isCollidingWithWall(const Entity& entity, const std::vector<sf::RectangleShape>& tiles) const;
    bool isOnValidGround(const Entity& entity, const std::vector<std::string>& levelData) const;
    sf::Vector2f getEntityCenter(const Entity& entity) const;
    sf::FloatRect getEntityBounds(const Entity& entity) const;
};