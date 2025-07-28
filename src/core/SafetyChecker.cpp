#include "SafetyChecker.h"
#include "../entity/Entity.h"
#include "Constants.h"
#include <iostream>

void SafetyChecker::registerEntity(const std::string& entityId, std::function<Entity*()> getter) {
    entityGetters[entityId] = getter;
}

Entity* SafetyChecker::getEntity(const std::string& entityId) const {
    auto it = entityGetters.find(entityId);
    if (it != entityGetters.end() && it->second) {
        return it->second();
    }
    return nullptr;
}

bool SafetyChecker::validateSpawnPosition(const std::string& entityId,
                                        const std::vector<std::string>& levelData,
                                        const std::vector<sf::RectangleShape>& tiles) {
    Entity* entity = getEntity(entityId);
    if (!entity) {
        std::cerr << "Warning: Entity " << entityId << " not found for spawn validation" << std::endl;
        return false;
    }
    
    return isOnValidGround(*entity, levelData);
}

SafetyChecker::SafetyResult SafetyChecker::checkPositionSafety(const std::string& entityId,
                                                               const std::vector<std::string>& levelData,
                                                               const std::vector<sf::RectangleShape>& tiles) {
    SafetyResult result;
    result.isSafe = true;
    
    Entity* entity = getEntity(entityId);
    if (!entity) {
        result.isSafe = false;
        result.reason = "Entity not found";
        return result;
    }
    
    // 检查是否在地图外
    if (!isWithinMapBounds(*entity, levelData)) {
        result.isSafe = false;
        result.reason = "Out of map bounds";
        return result;
    }
    
    // 检查是否在地图边缘
    if (isNearMapEdge(*entity, levelData)) {
        result.isSafe = false;
        result.reason = "Near map edge";
        return result;
    }
    
    // 检查实体是否在墙中
    if (isCollidingWithWall(*entity, tiles)) {
        result.isSafe = false;
        result.reason = "Entity stuck in wall";
        return result;
    }
    
    return result;
}

bool SafetyChecker::updateEntitySafety(const std::string& entityId,
                                     const std::vector<std::string>& levelData,
                                     const std::vector<sf::RectangleShape>& tiles,
                                     float dt) {
    Entity* entity = getEntity(entityId);
    if (!entity) {
        return false;
    }
    
    auto& safety = entitySafetyMap[entityId];
    
    // 检查当前位置安全性
    auto result = checkPositionSafety(entityId, levelData, tiles);
    
    if (!result.isSafe) {
        if (!safety.isInDanger) {
            // 首次检测到危险位置
            safety.isInDanger = true;
            safety.dangerTimer = 0.0f;
        } else {
            // 累积危险时间
            safety.dangerTimer += dt;
            if (safety.dangerTimer >= DANGER_RESET_TIME) {
                std::cerr << "Warning: " << entityId << " in dangerous position for " 
                         << DANGER_RESET_TIME << " seconds, resetting position..." << std::endl;
                return true; // 需要重置位置
            }
        }
    } else {
        // 位置安全，重置计时器并记录安全位置
        safety.isInDanger = false;
        safety.dangerTimer = 0.0f;
        safety.lastSafePosition = entity->getPosition();
    }
    
    return false; // 不需要重置位置
}

SafetyChecker::EntitySafety SafetyChecker::getEntitySafety(const std::string& entityId) const {
    auto it = entitySafetyMap.find(entityId);
    if (it != entitySafetyMap.end()) {
        return it->second;
    }
    return EntitySafety(); // 返回默认安全状态
}

void SafetyChecker::resetEntitySafety(const std::string& entityId) {
    entitySafetyMap.erase(entityId);
}

void SafetyChecker::clearAllEntities() {
    entitySafetyMap.clear();
    entityGetters.clear();
}

// 私有辅助函数
bool SafetyChecker::isWithinMapBounds(const Entity& entity, 
                                    const std::vector<std::string>& levelData) const {
    sf::Vector2f pos = entity.getPosition();
    float mapWidth = levelData[0].size() * TILE;
    float mapHeight = levelData.size() * TILE;
    
    sf::FloatRect bounds = getEntityBounds(entity);
    
    return (bounds.left >= 0 && bounds.top >= 0 && 
            bounds.left + bounds.width <= mapWidth && 
            bounds.top + bounds.height <= mapHeight);
}

bool SafetyChecker::isNearMapEdge(const Entity& entity, 
                                const std::vector<std::string>& levelData) const {
    sf::Vector2f pos = entity.getPosition();
    float mapWidth = levelData[0].size() * TILE;
    float mapHeight = levelData.size() * TILE;
    
    sf::FloatRect bounds = getEntityBounds(entity);
    float edgeThreshold = TILE * EDGE_THRESHOLD;
    
    return (bounds.left < edgeThreshold || 
            bounds.left + bounds.width > mapWidth - edgeThreshold ||
            bounds.top < edgeThreshold || 
            bounds.top + bounds.height > mapHeight - edgeThreshold);
}

bool SafetyChecker::isCollidingWithWall(const Entity& entity, 
                                      const std::vector<sf::RectangleShape>& tiles) const {
    sf::FloatRect entityBounds = getEntityBounds(entity);
    
    for (const auto& tile : tiles) {
        // 只检测黑色墙壁方块
        if (tile.getFillColor() == sf::Color::Black) {
            sf::FloatRect tileBounds = tile.getGlobalBounds();
            if (entityBounds.intersects(tileBounds)) {
                return true;
            }
        }
    }
    
    return false;
}

bool SafetyChecker::isOnValidGround(const Entity& entity, 
                                  const std::vector<std::string>& levelData) const {
    sf::FloatRect bounds = getEntityBounds(entity);
    
    // 检查实体所在网格位置
    int leftGridX = static_cast<int>(bounds.left / TILE);
    int rightGridX = static_cast<int>((bounds.left + bounds.width - 1) / TILE);
    int bottomGridY = static_cast<int>((bounds.top + bounds.height - 1) / TILE);
    
    int actualWidth = levelData[0].size();
    int actualHeight = levelData.size();
    
    // 检查实体覆盖的所有网格位置
    for (int gridX = leftGridX; gridX <= rightGridX; ++gridX) {
        if (gridX >= 0 && gridX < actualWidth && bottomGridY >= 0 && bottomGridY < actualHeight) {
            char groundChar = levelData[bottomGridY][gridX];
            if (groundChar == '0' || groundChar == 'P' || groundChar == 'E' || groundChar == 'I') {
                return true;
            }
        }
    }
    
    return false;
}

sf::Vector2f SafetyChecker::getEntityCenter(const Entity& entity) const {
    sf::FloatRect bounds = getEntityBounds(entity);
    return sf::Vector2f(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
}

sf::FloatRect SafetyChecker::getEntityBounds(const Entity& entity) const {
    return entity.getBounds();
}