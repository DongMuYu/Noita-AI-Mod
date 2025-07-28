// Map.cpp

#include "Map.h"
#include "../core/Constants.h"
#include <iostream>

Map::Map() {
}

Map::~Map() {
}

void Map::resetMap() {
    levelData.clear();
    tiles.clear();
    playerPos = sf::Vector2f(-1.0f, -1.0f);
    targetPosition = sf::Vector2f(-1.0f, -1.0f);
}

void Map::draw(sf::RenderWindow& window)
{
    // 如果关卡数据为空，先获取关卡数据
    if (levelData.empty()) {
        levelData = Parser::parseLevel();
        
        // 在重新加载后验证地图数据
        if (!levelData.empty()) {
            bool hasPlayer = false;
            for (const auto& row : levelData) {
                if (row.find('P') != std::string::npos) {
                    std::cout << "P found" << std::endl;
                    hasPlayer = true;
                    break;
                }
            }
            
            if (!hasPlayer) {
                std::cout << "Warning: Map has no player starting position character 'P'" << std::endl;
            }
        }
    }

    // 清空现有瓦片
    tiles.clear();
    
    // 遍历关卡数据的二维网格，创建对应的游戏对象
    // y坐标：行索引，对应游戏世界的垂直位置
    for (size_t y = 0; y < levelData.size(); ++y) {
        // x坐标：列索引，对应游戏世界的水平位置
        for (size_t x = 0; x < levelData[y].size(); ++x) {
            char c = levelData[y][x];  // 获取当前单元格的标识字符
            
            // 瓦片创建部分，根据字符创建不同类型的瓦片
            // '1'表示创建固体瓦片(黑色方块)
            if (c == '1') {
                // 创建瓦片对象，大小为TILE x TILE像素
                sf::RectangleShape tile({(float)TILE, (float)TILE});
                // 设置瓦片位置：网格坐标(x,y)转换为像素坐标
                tile.setPosition(x * TILE, y * TILE);
                tile.setFillColor(sf::Color::Black);  // 设置瓦片颜色为黑色
                tiles.emplace_back(tile);  // 添加到瓦片列表
            } 
            else if (c == 'M')
            {
                // 创建被标记的瓦片
                sf::RectangleShape tile({(float)TILE, (float)TILE});
                tile.setPosition(x * TILE, y * TILE);
                tile.setFillColor(sf::Color::Red);  // 设置瓦片颜色为红色
                tiles.emplace_back(tile);  // 添加到瓦片列表
            }
            else if (std::string("0PTEI").find(c) != std::string::npos)
            {
                // 创建空白瓦片（用于渲染但排除碰撞）
                sf::RectangleShape tile({(float)TILE, (float)TILE});
                tile.setPosition(x * TILE, y * TILE);
                tile.setFillColor(sf::Color::Transparent);  // 透明标记 Transparent (暂时设定为黄色用于辨识)
                tiles.emplace_back(tile);  // 添加到瓦片列表
            }
            
            // 实体创建部分
            // 'P'表示玩家起始位置
            if (c == 'P') {
                playerPos = sf::Vector2f(x * TILE, y * TILE);
                // std::cout << "Player: (" << x * TILE << ", " << y * TILE << ")" << std::endl;
            } 
            // 'T'表示目标点位置(终点)
            else if (c == 'T') {
                // 记录目标点像素坐标
                targetPosition = sf::Vector2f(x * TILE, y * TILE);
            }
        }
    }

    // 绘制所有瓦片
    for (const auto& tile : tiles) {
        window.draw(tile);
    }
}
