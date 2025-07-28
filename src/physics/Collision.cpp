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

// #include "../world/Parser.h"

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
 * @note 实现了分离轴定理(SAT)的2D碰撞响应，处理水平/垂直碰撞分离
 */
void handlePlayerPlatformCollision(PlayerCollisionData& player,
                                   const std::vector<sf::RectangleShape>& platforms,
                                   const std::vector<std::string>& levelData) {
    // 重置地面状态，默认假设玩家不在地面上
    player.onGround = false;

    // 遍历所有平台检测碰撞 - 按行优先顺序（从左到右，从上到下）
    for (size_t i = 0; i < platforms.size(); ++i) {
        const auto& platform = platforms[i];
        // 排除透明瓦片（空白瓦片）
        if (platform.getFillColor() == sf::Color::Transparent) continue;  // Transparent
        // 排除黄色瓦片（用于辨识，完成后可去除）
        if (platform.getFillColor() == sf::Color::Yellow) continue;  // Yellow
        
        // 快速排除不碰撞的平台
        if (!intersects(player.shape, platform)) continue;

        // 根据索引i计算二维数组坐标
        int x = static_cast<int>(i % levelData[0].size());
        int y = static_cast<int>(i / levelData[0].size());
        char c = levelData[y][x];  // 获取当前单元格的标识字符
        // cout << "c: " << c << " x: " << x << " y: " << y << " ";

        // 获取玩家和平台的全局边界（AABB）
        auto pb = player.shape.getGlobalBounds();  // 玩家边界
        auto tb = platform.getGlobalBounds();     // 平台边界

        // 计算X轴和Y轴上的重叠量
        // overlapX: 水平方向重叠像素数
        // overlapY: 垂直方向重叠像素数
        
        // 计算X轴方向的重叠长度
        // 原理：两个矩形在X轴上的重叠区域 = 右边界最小值 - 左边界最大值
        // std::min(pb.left + pb.width, tb.left + tb.width)：获取两个矩形右边界的最小值
        // std::max(pb.left, tb.left)：获取两个矩形左边界的最大值
        // 若结果为负，则表示无重叠，通过std::max(0.0f, ...)确保重叠长度非负
        float overlapX = std::max(0.0f,
            std::min(pb.left + pb.width, tb.left + tb.width) - std::max(pb.left, tb.left));

        // 计算Y轴方向的重叠长度
        // 原理：两个矩形在Y轴上的重叠区域 = 下边界最小值 - 上边界最大值
        // std::min(pb.top + pb.height, tb.top + tb.height)：获取两个矩形下边界的最小值
        // std::max(pb.top, tb.top)：获取两个矩形上边界的最大值
        // 若结果为负，则表示无重叠，通过std::max(0.0f, ...)确保重叠长度非负
        float overlapY = std::max(0.0f,
            std::min(pb.top + pb.height, tb.top + tb.height) - std::max(pb.top, tb.top));

        // cout << "overlapX: " << overlapX << " overlapY: " << overlapY << " ";

        // 忽略无重叠的情况（理论上不会发生，保险措施）
        if (overlapX <= 0 || overlapY <= 0) continue;

        // 根据重叠量判断主要碰撞方向
        // const float BOTTOM_TOLERANCE = 14.0f; // 允许少量的像素的穿透

        if (overlapX < overlapY) {
            // 水平碰撞为主：根据相对位置调整玩家水平位置
            // 左碰撞：-overlapX，右碰撞：+overlapX
            // cout << "<level>" << " " << endl;
            player.velocity.x = 0;

            player.shape.move(sf::Vector2f(pb.left < tb.left ? -overlapX : overlapX, 0.0f));

        } else {
            // 垂直碰撞为主
            // 玩家顶部碰撞平台底部（下落状态）
            if (pb.top < tb.top && player.velocity.y > 0.0f) {
                // cout << "vertical(1)" << " " << endl;

                // 计算精确的位置修正值
                float platformTop = platform.getPosition().y;
                float playerBottom = player.shape.getPosition().y + player.shape.getSize().y;
                // 将玩家上移至平台顶部
                player.shape.move(sf::Vector2f(0.0f, -(playerBottom - platformTop)));

                // 清除垂直速度，防止持续下落
                player.velocity.y = 0.0f;

                // 设置地面状态，(原版本：允许跳跃)
                player.onGround = true;
            } 
            else if (pb.top < tb.top + tb.height) {
                // 对于地图顶部边缘的瓦片，直接进行天花板检测，无需检查上方瓦片
                // cout << "vertical(2)" << " " << endl;

                // 检查是否为地图顶部边缘
                bool isTopEdge = (y == 0);
                
                if (!isTopEdge) {
                    // 获取当前平台上方瓦片的坐标
                    int upY = y - 1;
                    if (upY >= 0) {
                        // 检查玩家上方瓦片及其左右相邻方块
                        bool hasLeftWall = false;
                        bool hasRightWall = false;
                        bool hasCenterWall = false;
                        
                        // 检查中心上方方块
                        if (x >= 0 && x < levelData[0].size()) {
                            hasCenterWall = (levelData[upY][x] == '1');
                        }
                        
                        // 检查左上方方块
                        int leftX = x - 1;
                        if (leftX >= 0 && leftX < levelData[0].size()) {
                            hasLeftWall = (levelData[upY][leftX] == '1');
                        }
                        
                        // 检查右上方方块
                        int rightX = x + 1;
                        if (rightX >= 0 && rightX < levelData[0].size()) {
                            hasRightWall = (levelData[upY][rightX] == '1');
                        }
                        
                        // 计算玩家两个顶角的X坐标
                        float playerLeft = pb.left;
                        float playerRight = pb.left + pb.width;
                        float platformLeft = tb.left;
                        float platformRight = tb.left + tb.width;
                        
                        // 检查玩家的两个顶角是否都在平台的水平范围内
                        bool leftCornerInPlatform = (playerLeft >= platformLeft && playerLeft <= platformRight);
                        bool rightCornerInPlatform = (playerRight >= platformLeft && playerRight <= platformRight);
                        
                        // 根据上方方块状态选择碰撞检测方式
                        if ((hasCenterWall && hasLeftWall && hasRightWall) || (leftCornerInPlatform && rightCornerInPlatform)) {
                            // 两个顶角都在平台水平范围内 - 使用整个顶边判断碰撞
                            // cout << "vertical(2b)" << " " << endl;
                            float playerTop = pb.top;
                            float platformBottom = tb.top + tb.height;
                            
                            // 计算需要向下移动的距离
                            float moveDistance = platformBottom - playerTop;
                            player.shape.move(sf::Vector2f(0.0f, moveDistance));
                        } else if (pb.left + pb.width/2 > tb.left + tb.width*3/16 && pb.left + pb.width/2 < tb.left + tb.width*13/16) {
                            // 使用中心区域判断
                            // cout << "vertical(2c)" << " " << endl;
                            float playerTopCenterY = pb.top;
                            float platformBottom = tb.top + tb.height;
                            
                            // 计算需要向下移动的距离
                            float moveDistance = platformBottom - playerTopCenterY;
                            player.shape.move(sf::Vector2f(0.0f, moveDistance));
                        }
                    }
                } else {
                    // 地图顶部边缘的瓦片，直接作为天花板处理
                    // cout << "vertical(2-top)" << " " << endl;
                    float playerTop = pb.top;
                    float platformBottom = tb.top + tb.height;
                    
                    // 计算需要向下移动的距离
                    float moveDistance = platformBottom - playerTop;
                    player.shape.move(sf::Vector2f(0.0f, moveDistance));
                }
            }

            else {
                // cout << "vertical(3)" << " " << endl;
            }
        }
    }
}