#ifndef ENEMY_H
#define ENEMY_H

#include <SFML/System/Vector2.hpp>
#include "Entity.h"

/**
 * @brief 敌人实体类
 * 表示游戏世界中的敌对NPC
 */
class Enemy : public Entity {
private:
    int health;             // 敌人生命值
    bool isAlive;           // 是否存活

public:
    /**
     * @brief 默认构造函数
     */
    Enemy() = default;

    /**
     * @brief 构造函数
     * @param id 敌人ID
     * @param position 初始位置
     * @param health 初始生命值
     * @param speed 移动速度
     */
    Enemy(const std::string& id, sf::Vector2f position, int health, float speed);

    /**
     * @brief 获取敌人生命值
     * @return 当前生命值
     */
    int getHealth() const { return health; }

    /**
     * @brief 减少敌人生命值
     * @param damage 伤害值
     * @return 是否被击败
     */
    bool takeDamage(int damage);

    /**
     * @brief 检查敌人是否存活
     * @return true表示存活，false表示已被击败
     */
    bool alive() const { return isAlive; }

    /**
     * @brief 更新敌人状态
     * @param dt 时间增量（秒）
     */
    void update(float dt) override;
};

#endif // ENEMY_H