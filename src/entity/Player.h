#pragma once
#include <SFML/Graphics.hpp>
#include "../core/Constants.h"
#include "Entity.h"

/**
 * @file Player.h
 * @brief 玩家实体类
 * @details 负责玩家的物理行为、输入响应、状态管理和渲染
 */
class Player : public Entity {
private:
    
    /** @brief 当前能量值 */
    float currentEnergy;
    
    /** @brief 最大能量值 */
    float maxEnergy;
    
    /** @brief 能量消耗速率（单位/秒） */
    float energyConsumptionRate;
    
    /** @brief 能量恢复速率（单位/秒） */
    float energyRegenRate;
    
    /** @brief 飞行状态标志 */
    bool isFlying;
    
public:
    /**
     * @brief 构造函数
     * @param pos 初始位置
     */
    Player(sf::Vector2f pos);
    
    /**
     * @brief 更新玩家状态
     * @param dt 时间增量（秒）
     */
    void update(float dt) override;
    
    void handleInput(float dt, bool aiMode = false, float aiMoveX = 0.0f, bool aiUseEnergy = false);
    
    // Getters
    /** @brief 获取当前能量值 */
    float getCurrentEnergy() const { return currentEnergy; }
    
    /** @brief 获取最大能量值 */
    float getMaxEnergy() const { return maxEnergy; }
    
    /** @brief 获取飞行状态 */
    bool getIsFlying() const { return isFlying; }
    
};
