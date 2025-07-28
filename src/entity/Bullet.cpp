#include "Bullet.h"
#include <cmath>

Bullet::Bullet(const std::string& id, sf::Vector2f pos, float dir, float spd, 
               float dmg, float life) 
    : Entity(id, pos, 8.0f, 8.0f), direction(dir), speed(spd), 
      damage(dmg), lifetime(life), age(0.0f) {
    // 设置子弹外观
    getShapeRef().setFillColor(sf::Color::Yellow);
    getShapeRef().setOutlineColor(sf::Color::Red);
    
    // 计算初始速度向量
    float vx = speed * std::cos(direction);
    float vy = speed * std::sin(direction);
    setVelocity(vx, vy);
}

void Bullet::update(float dt) {
    // 更新存在时间
    age += dt;
    
    // 根据速度向量更新位置
    sf::Vector2f newPos = getPosition() + getVelocity() * dt;
    setPosition(newPos);
}

bool Bullet::isExpired() const {
    return age >= lifetime;
}