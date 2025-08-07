#include "Player.h"
#include <SFML/Graphics.hpp>

// 构造函数: 初始化玩家位置和外观
Player::Player(sf::Vector2f pos) : Entity("player", pos, TILE * 0.8f, TILE * 0.8f) {
    shape.setFillColor(sf::Color(128, 0, 128)); // 紫色填充 (RGB)
    shape.setOutlineColor(sf::Color(128, 0, 128)); // 紫色轮廓 (RGB)
    
    maxEnergy = 150.0f;
    currentEnergy = maxEnergy;
    energyConsumptionRate = 100.0f; // 每秒消耗能量
    energyRegenRate = 500.0f; // 每秒恢复能量
    isFlying = false;

}

// 更新函数: 应用重力并移动玩家
// 参数: dt - 时间增量(秒)
void Player::update(float dt){

    // 飞行能量管理
    // 检查是否在飞行（不在地面上且向上移动）
    bool actuallyFlying = (velocity.y < 0 && !onGround);
    
    if (actuallyFlying && currentEnergy > 0) {
        currentEnergy -= energyConsumptionRate * dt;
        if (currentEnergy < 0) currentEnergy = 0;
    } else if (onGround && currentEnergy < maxEnergy) {
        // 站在地面上时恢复能量
        currentEnergy += energyRegenRate * dt; // 地面恢复更快
        if (currentEnergy > maxEnergy) currentEnergy = maxEnergy;
    } 
    isFlying = actuallyFlying;

    // 下落速度限制
    if (velocity.y < maxFallSpeed) {
        velocity.y += GRAVITY * dt;  // 应用重力加速度

    }

    // 计算新位置
    sf::Vector2f newPos = shape.getPosition() + velocity * dt;

    shape.setPosition(newPos);   // 设置最终位置

    // 调用父类的更新函数
    // Entity::update(dt);
}

void Player::handleInput(float dt, bool aiMode, float aiMoveX, bool aiUseEnergy) {
    sf::Vector2f newVelocity = getVelocity();
    
    if (aiMode) {
        // AI控制模式：使用AI输入
        newVelocity.x = aiMoveX * MOVE_SPEED;
        
        if (aiUseEnergy && getCurrentEnergy() > 0) {
            newVelocity.y = -JUMP_VELOCITY * 0.45f;
        }
    } else {
        // 键盘控制模式
        sf::Vector2f dir{0, 0};
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dir.x -= 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dir.x += 1;
        
        if (dir.x != 0) dir.x = dir.x > 0 ? 1.f : -1.f;
        newVelocity.x = dir.x * MOVE_SPEED;
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            if (getCurrentEnergy() > 0) {
                newVelocity.y = -JUMP_VELOCITY * 0.45f;
            }
        }
    }
    
    setVelocity(newVelocity);
}
 

