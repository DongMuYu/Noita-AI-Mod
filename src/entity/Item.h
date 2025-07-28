#ifndef ITEM_H
#define ITEM_H

#include <SFML/System/Vector2.hpp>
#include <string>

// 物品类型枚举
enum class ItemType {
    HEALTH_POTION,
    MANA_POTION,
    WEAPON,
    ARMOR,
    KEY,
    TREASURE
};

/**
 * @brief 游戏物品类
 * 表示游戏世界中的可交互物品
 */
class Item {
private:
    sf::Vector2i position;       // 物品位置
    ItemType type;              // 物品类型
    std::string name;           // 物品名称
    int value;                  // 物品价值
    bool isCollected;           // 是否已被收集

public:
    /**
     * @brief 构造函数
     * @param pos 物品位置
     * @param itemType 物品类型
     * @param itemName 物品名称
     * @param itemValue 物品价值
     */
    Item() = default;
    Item(sf::Vector2i pos, ItemType itemType, const std::string& itemName, int itemValue);

    /**
     * @brief 获取物品位置
     * @return 物品的网格坐标
     */
    sf::Vector2i getPosition() const { return position; }

    /**
     * @brief 获取物品类型
     * @return 物品类型枚举值
     */
    ItemType getType() const { return type; }

    /**
     * @brief 获取物品名称
     * @return 物品名称字符串
     */
    std::string getName() const { return name; }

    /**
     * @brief 获取物品价值
     * @return 物品价值数值
     */
    int getValue() const { return value; }

    /**
     * @brief 检查物品是否已被收集
     * @return true表示已收集，false表示未收集
     */
    bool collected() const { return isCollected; }

    /**
     * @brief 标记物品为已收集
     */
    void collect() { isCollected = true; }
};

#endif // ITEM_H