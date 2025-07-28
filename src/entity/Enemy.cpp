#include "Enemy.h"

Enemy::Enemy(const std::string& id, sf::Vector2f position, int health, float speed)
    : Entity(id, position, 32.0f, 32.0f), health(health), isAlive(true) {
    // 设置敌人外观
    getShapeRef().setFillColor(sf::Color::Red);
    getShapeRef().setOutlineColor(sf::Color(139, 0, 0)); // 深红色
    
    // 设置速度（可以根据需要设置）
    setVelocity(0.0f, 0.0f);
}

bool Enemy::takeDamage(int damage) {
    if (!isAlive) return false;
    
    health -= damage;
    if (health <= 0) {
        health = 0;
        isAlive = false;
        return true; // 被击败
    }
    return false; // 未被击败
}

void Enemy::update(float dt) {
    // 这里可以添加敌人AI逻辑
    // 例如：简单的朝向玩家移动
    // 基础更新逻辑（如移动、碰撞检测等）
    
    // 示例：简单的随机移动
    static float timer = 0.0f;
    timer += dt;
    
    if (timer > 1.0f) { // 每秒改变一次方向
        float randomX = (rand() % 100 - 50) * 0.01f;
        float randomY = (rand() % 100 - 50) * 0.01f;
        setVelocity(randomX * 100.0f, randomY * 100.0f);
        timer = 0.0f;
    }
    
    // 更新位置
    sf::Vector2f newPos = getPosition() + getVelocity() * dt;
    setPosition(newPos);
}