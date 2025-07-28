#ifndef COLLISION_H
#define COLLISION_H

#include <SFML/Graphics.hpp>

bool intersects(const sf::RectangleShape& a, const sf::RectangleShape& b);

struct PlayerCollisionData {
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    bool onGround;
};

/**
 * @brief 处理玩家与平台之间的碰撞响应
 * @param player 玩家碰撞数据结构，包含形状、速度和地面状态
 * @param platforms 平台形状列表（已废弃，保留兼容性）
 * @param levelData 二维数组形式的关卡数据，包含方块类型信息
 * @note 实现了基于网格坐标的2D碰撞响应，处理水平/垂直碰撞分离
 */
void handlePlayerPlatformCollision(PlayerCollisionData& player, const std::vector<sf::RectangleShape>& platforms, const std::vector<std::string>& levelData);

#endif