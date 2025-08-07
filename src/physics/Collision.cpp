// src/physics/Collision.cpp
// 物理碰撞检测系统核心实现
// 负责处理游戏中实体间的碰撞检测与响应逻辑
#include "Collision.h"
#include "../core/Constants.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <vector>
#include <iostream>
using namespace std;

#include "../world/Parser.h"

/**
 * @brief 检测两个矩形形状是否发生碰撞，原理为检测两个矩形是否重叠
 * @param a 第一个矩形形状（SFML RectangleShape）
 * @param b 第二个矩形形状（SFML RectangleShape）
 * @return true 如果发生碰撞，false 否则
 * @note 使用轴对齐 bounding box (AABB) 碰撞检测算法
 */
bool intersects(const sf::RectangleShape& a, const sf::RectangleShape& b) {
    // 获取全局边界并检测交集
    return a.getGlobalBounds().intersects(b.getGlobalBounds());
}

/**
 * @brief 处理玩家与平台之间的碰撞响应
 * @param player 玩家碰撞数据结构，包含形状、速度和地面状态
 * @param platforms 平台形状列表，代表所有可碰撞平台
 * @note 实现了墙方块的特殊碰撞处理，根据方块在墙中的位置调整垂直碰撞
 */
void handlePlayerPlatformCollision(PlayerCollisionData& player,
                                   const std::vector<sf::RectangleShape>& platforms,
                                   const std::vector<std::string>& levelData) {
    // 重置地面状态
    player.onGround = false;
    
    // 获取玩家边界
    sf::FloatRect playerBounds = player.shape.getGlobalBounds();
    
    // 遍历所有平台
    for (size_t i = 0; i < platforms.size(); ++i) {
        const auto& platform = platforms[i];
        
        // 排除透明和黄色瓦片 (黄色方块由于调试)
        if (platform.getFillColor() == sf::Color::Transparent) continue;
        if (platform.getFillColor() == sf::Color::Yellow) continue;
        
        // 快速排除不碰撞的平台
        if (!intersects(player.shape, platform)) continue;

        // 根据索引i计算二维数组坐标
        int x = static_cast<int>(i % levelData[0].size());
        int y = static_cast<int>(i / levelData[0].size());

        // 边界检查
        if (y >= levelData.size() || x >= levelData[y].size()) continue;
        char c = levelData[y][x];  // 获取当前单元格的标识字符

        // std::cout << "c: " << c << " x: " << x << " y: " << y << " ";
        // std::cout << "c: " << c << std::endl;

        // 获取玩家和平台的全局边界（AABB）
        auto pb = player.shape.getGlobalBounds();  // 玩家边界
        auto tb = platform.getGlobalBounds();     // 平台边界

        // 计算X轴和Y轴上的重叠量
        // overlapX: 水平方向重叠像素数
        // overlapY: 垂直方向重叠像素数
        
        // 计算重叠区域
        float overlapX = std::max(0.0f,
            std::min(pb.left + pb.width, tb.left + tb.width) - std::max(pb.left, tb.left));
        float overlapY = std::max(0.0f,
            std::min(pb.top + pb.height, tb.top + tb.height) - std::max(pb.top, tb.top));
        
        if (overlapX <= 0 || overlapY <= 0) continue;
        
        // 判断是否为墙碰撞
        bool isWallBlock = false;
        bool isWallTop = false;
        bool isWallBottom = false;

        if (c == 'W') {
            isWallBlock = true;
        }
        else if (c == '3') {
            isWallTop = true;
        }
        else if (c == '4') {
            isWallBottom = true;
        }
            
        // 经典碰撞处理
        if (overlapX < overlapY) {
            // 水平碰撞
            player.velocity.x = 0;
            player.shape.move(sf::Vector2f(pb.left < tb.left ? -overlapX : overlapX, 0.0f));
        } else {
            // 垂直碰撞
            
            // 调试信息：碰撞检测
            // std::cout << "Collision Debug: Player(" << player.shape.getPosition().x << "," << player.shape.getPosition().y << ") ";
            // std::cout << "vs Tile(" << x << "," << y << ") ";
            // std::cout << "isWallBlock: " << isWallBlock << " ";

            if (isWallBlock) {
                // std::cout << "isWallBlock(1)" << std::endl;
                continue;
            }
            
            // 从上方碰撞（站在平台上）
            if (pb.top < tb.top && player.velocity.y >= 0 && !isWallBottom) {
                // std::cout << "Vertical(2)" << std::endl;
                float moveDistance = tb.top - (pb.top + pb.height);
                player.shape.move(sf::Vector2f(0.0f, moveDistance));
                player.velocity.y = 0.0f;
                player.onGround = true;
            }
            // 从下方碰撞（撞到天花板）
            else if (pb.top > tb.top && player.velocity.y <= 0 && !isWallTop) {
                // std::cout << "Vertical(3)" << std::endl;
                float moveDistance = (tb.top + tb.height) - pb.top;
                player.shape.move(sf::Vector2f(0.0f, moveDistance));
                player.velocity.y = 0.0f;
            }
        }
    }
}