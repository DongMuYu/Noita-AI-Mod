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

    // 更新跳跃冷却
    if(jumpCooldown > 0) jumpCooldown -= dt;
    

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

// 处理玩家输入
// 参数: dt - 时间增量(秒)
void Player::handleInput(float dt) {
    // 初始化方向向量(默认为零向量)
    // dir.x: 水平方向(-1=左, 1=右), dir.y: 垂直方向(-1=上, 1=下)
    sf::Vector2f dir{0, 0};
    
    // 检测水平移动输入
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dir.x -= 1;  // A键: 向左移动
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dir.x += 1;  // D键: 向右移动
    
    // 标准化水平方向(确保对角线移动速度不会快于直线移动)
    if (dir.x != 0) dir.x = dir.x > 0 ? 1.f : -1.f;  // 将非零水平方向标准化为±1
    
    // 更新水平速度
    sf::Vector2f newVelocity = getVelocity();  // 获取当前速度
    newVelocity.x = dir.x * MOVE_SPEED;              // 设置水平速度 = 方向 × 移动速度常量
    setVelocity(newVelocity);                 // 应用新速度到玩家对象

    // 检测上升/飞行输入(W键)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        sf::Vector2f newVelocity = getVelocity();  // 获取当前速度
        // 检查是否有足够能量进行飞行
        if (getCurrentEnergy() > 0) {
            // 设置垂直速度 = -跳跃速度 × 0.45(飞行模式减速系数)
            newVelocity.y = -JUMP_VELOCITY * 0.45f;
            setVelocity(newVelocity);                 // 应用新速度到玩家对象
        }
    }
}
 

