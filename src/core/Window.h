// Window.h

#pragma once
#include <SFML/Graphics.hpp>
#include <string>

/**
 * @brief 窗口管理类
 * @details 负责游戏窗口的创建、配置和管理
 * 封装SFML窗口的创建、事件处理和渲染功能
 */
class Window {
private:
    /** @brief 主游戏窗口 */
    sf::RenderWindow mainWindow;
    
    /** @brief 窗口宽度 */
    unsigned int windowWidth;
    
    /** @brief 窗口高度 */
    unsigned int windowHeight;

public:
    /**
     * @brief 构造函数
     * @param width 窗口宽度
     * @param height 窗口高度
     * @param title 主窗口标题
     */
    Window(unsigned int width, unsigned int height, 
           const std::string& title = "NoitaSimulator - 主游戏",
           const std::string& = "");
    
    /**
     * @brief 析构函数
     */
    ~Window();
    
    /**
     * @brief 初始化窗口
     * @details 配置窗口属性，设置垂直同步和帧率限制
     */
    void initialize();
    
    /**
     * @brief 获取主游戏窗口
     * @return 主游戏窗口的引用
     */
    sf::RenderWindow& getMainWindow();
    

    
    /**
     * @brief 获取主窗口尺寸
     * @return 主窗口的尺寸
     */
    sf::Vector2u getMainWindowSize() const;
    

    
    /**
     * @brief 检查主窗口是否打开
     * @return true如果窗口打开
     */
    bool isMainWindowOpen() const;
    

    
    /**
     * @brief 关闭所有窗口
     */
    void closeAll();
    
    /**
     * @brief 关闭所有窗口（别名）
     */
    void close();
    
    /**
     * @brief 处理窗口事件
     * @param event 要处理的事件
     * @return true如果窗口应该继续运行
     */
    bool handleEvent(const sf::Event& event);
    
    /**
     * @brief 处理所有窗口的事件
     */
    void handleEvents();
    
    /**
     * @brief 清空主窗口
     * @param color 清屏颜色
     */
    void clearMainWindow(const sf::Color& color = sf::Color::White);
    

    
    /**
     * @brief 显示主窗口内容
     */
    void displayMainWindow();
    

};