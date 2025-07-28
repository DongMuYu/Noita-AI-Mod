#include "Entity.h"

// 更新函数: 应用重力并移动实体
// 参数: dt - 时间增量(秒)
void Entity::update(float dt) {
    // 更新跳跃冷却
    if(jumpCooldown > 0) jumpCooldown -= dt;
    
    
    // 下落速度限制
    if (velocity.y < maxFallSpeed) {
        velocity.y += GRAVITY * dt;  // 应用重力加速度

    // 计算新位置
    sf::Vector2f newPos = shape.getPosition() + velocity * dt;

    shape.setPosition(newPos);   // 设置最终位置

    }
}