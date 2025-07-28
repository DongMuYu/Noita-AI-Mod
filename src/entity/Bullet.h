#pragma once
#include "Entity.h"

/**
 * @file Bullet.h
 * @brief 子弹实体类
 * @details 负责子弹的移动、碰撞检测和生命周期管理
 */
class Bullet : public Entity {
private:
    /** @brief 子弹伤害值 */
    float damage;
    
    /** @brief 子弹速度（像素/秒） */
    float speed;
    
    /** @brief 子弹方向（角度，弧度） */
    float direction;
    
    /** @brief 子弹寿命（秒） */
    float lifetime;
    
    /** @brief 当前已存在时间 */
    float age;
    
public:
    /**
     * @brief 构造函数
     * @param id 子弹ID
     * @param pos 初始位置
     * @param dir 发射方向（角度，弧度）
     * @param spd 子弹速度
     * @param dmg 伤害值
     * @param life 寿命（秒）
     */
    Bullet(const std::string& id, sf::Vector2f pos, float dir, float spd = 400.0f, 
           float dmg = 10.0f, float life = 5.0f);
    
    /**
     * @brief 更新子弹状态
     * @param dt 时间增量（秒）
     */
    void update(float dt) override;
    
    /**
     * @brief 检查子弹是否已过期
     * @return true如果子弹已过期需要销毁
     */
    bool isExpired() const;
    
    /**
     * @brief 获取子弹伤害值
     * @return 伤害值
     */
    float getDamage() const { return damage; }
    
    /**
     * @brief 设置子弹伤害值
     * @param dmg 新伤害值
     */
    void setDamage(float dmg) { damage = dmg; }
    
    /**
     * @brief 获取子弹速度
     * @return 子弹速度
     */
    float getSpeed() const { return speed; }
    
    /**
     * @brief 设置子弹速度
     * @param spd 新速度
     */
    void setSpeed(float spd) { speed = spd; }
    
    /**
     * @brief 获取子弹方向
     * @return 方向角度（弧度）
     */
    float getDirection() const { return direction; }
    
    /**
     * @brief 设置子弹方向
     * @param dir 新方向角度（弧度）
     */
    void setDirection(float dir) { direction = dir; }
};