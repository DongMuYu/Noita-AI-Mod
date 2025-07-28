#pragma once
#include <SFML/Graphics.hpp>
#include "../core/Constants.h"
#include <string>

/**
 * @file Entity.h
 * @brief 实体基类
 * @details 所有游戏实体的父类，提供通用的属性和方法
 */
class Entity {
protected:
    /** @brief 实体图形表示 */
    sf::RectangleShape shape;
    
    /** @brief 实体速度向量（像素/秒） */
    sf::Vector2f velocity;

    /** @brief 跳跃冷却时间（秒） 
     *  @details 实体跳跃后冷却时间，防止连续跳跃  // 已经将弹跳移除，替换为飞行 
    */
    float jumpCooldown = 0.0f;

    /** @brief 最大下落速度（像素/秒） */
    float maxFallSpeed = 500.0f;

    /** @brief 实体是否在地面上 */
    bool onGround = false;
    
    /** @brief 实体唯一标识符 */
    std::string entityId;
    
    /** @brief 实体宽度（像素） */
    float width;
    
    /** @brief 实体高度（像素） */
    float height;
    
public:
    /**
     * @brief 构造函数
     * @param id 实体ID
     * @param pos 初始位置
     * @param w 宽度
     * @param h 高度
     */
    Entity(const std::string& id, sf::Vector2f pos, float w = 32.0f, float h = 32.0f)
        : entityId(id), width(w), height(h) {
        shape.setPosition(pos);
        shape.setSize(sf::Vector2f(w, h));
        shape.setOrigin(0.0f, 0.0f);  // 使用左上角作为原点
    }
    
    virtual ~Entity() = default;
    
    /**
     * @brief 更新实体状态
     * @param dt 时间增量（秒）
     */
    virtual void update(float dt) {}
    
    /**
     * @brief 渲染实体
     * @param window 渲染窗口
     */
    virtual void render(sf::RenderWindow& window) {
        window.draw(shape);
    }
    
    // 位置相关方法
    /**
     * @brief 获取实体位置
     * @return 实体中心位置坐标
     */
    sf::Vector2f getPosition() const {
        return shape.getPosition();
    }
    
    /**
     * @brief 设置实体位置
     * @param pos 新位置坐标
     */
    void setPosition(const sf::Vector2f& pos) {
        shape.setPosition(pos);
    }
    
    /**
     * @brief 设置实体位置
     * @param x X坐标
     * @param y Y坐标
     */
    void setPosition(float x, float y) {
        shape.setPosition(x, y);
    }
    
    // 尺寸相关方法
    /**
     * @brief 获取实体宽度
     * @return 实体宽度（像素）
     */
    float getWidth() const { return width; }
    
    /**
     * @brief 获取实体高度
     * @return 实体高度（像素）
     */
    float getHeight() const { return height; }

    /**
     * @brief 实体跳跃
     * @details 应用向上的速度，受跳跃冷却时间限制
     */
    void jump();

    /**
     * @brief 实体移动
     * @param velocity 移动速度向量
     */
    void move(sf::Vector2f velocity);

    /** @brief 检查实体是否在地面上 */
    bool isOnGround() const { return onGround; }
    
    /** @brief 设置实体地面状态 */
    void setOnGround(bool groundState) { onGround = groundState; }
    
    /**
     * @brief 设置实体尺寸
     * @param w 宽度
     * @param h 高度
     */
    void setSize(float w, float h) {
        width = w;
        height = h;
        shape.setSize(sf::Vector2f(w, h));
        shape.setOrigin(0.0f, 0.0f);  // 使用左上角作为原点
    }
    
    /**
     * @brief 获取实体边界框
     * @return 实体边界矩形
     */
    sf::FloatRect getBounds() const {
        return shape.getGlobalBounds();
    }
    
    // 速度相关方法
    /**
     * @brief 获取当前速度向量
     * @return 速度向量
     */
    const sf::Vector2f& getVelocity() const { return velocity; }
    
    /**
     * @brief 设置速度向量
     * @param vel 新速度向量
     */
    void setVelocity(const sf::Vector2f& vel) { velocity = vel; }
    
    /**
     * @brief 设置速度向量
     * @param x X方向速度
     * @param y Y方向速度
     */
    void setVelocity(float x, float y) { velocity.x = x; velocity.y = y; }
    
    /**
     * @brief 添加速度
     * @param deltaVel 速度增量
     */
    void addVelocity(const sf::Vector2f& deltaVel) {
        velocity += deltaVel;
    }
    
    /**
     * @brief 获取实体ID
     * @return 实体唯一标识符
     */
    const std::string& getEntityId() const { return entityId; }
    
    /**
     * @brief 设置实体ID
     * @param id 新实体ID
     */
    void setEntityId(const std::string& id) { entityId = id; }
    
    /**
     * @brief 获取实体图形形状
     * @return 实体的矩形形状
     */
    const sf::RectangleShape& getShape() const { return shape; }
    
    /**
     * @brief 获取实体图形形状（可修改）
     * @return 实体的矩形形状引用
     */
    sf::RectangleShape& getShapeRef() { return shape; }

    /** @brief 设置实体图形形状 */
    void setShape(const sf::RectangleShape& newShape) { shape = newShape; }
    
    /**
     * @brief 设置实体颜色
     * @param color 新颜色
     */
    void setColor(const sf::Color& color) {
        shape.setFillColor(color);
    }
    
    /**
     * @brief 获取实体颜色
     * @return 当前颜色
     */
    const sf::Color& getColor() const {
        return shape.getFillColor();
    }
};