// Map.h

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

#include "../world/Parser.h"

class Map {
private:
    /** @brief 关卡数据二维网格 */
    std::vector<std::string> levelData;
    /** @brief 地图瓦片集合 */
    std::vector<sf::RectangleShape> tiles;
    
    sf::Vector2f playerPos = sf::Vector2f(0.0f, 0.0f);

    /** @brief 目标点位置（玩家需要到达的位置） */
    sf::Vector2f targetPosition = sf::Vector2f(-1.0f, -1.0f);

public:
    Map();
    ~Map();

    void draw(sf::RenderWindow& window);
    void resetMap();
    const std::vector<std::string>& getLevelData() const { return levelData; }
    const std::vector<sf::RectangleShape>& getTiles() const { return tiles; }
    const sf::Vector2f& getPlayerPos() const { return playerPos; }
    const sf::Vector2f& getTargetPosition() const { return targetPosition; }
};





