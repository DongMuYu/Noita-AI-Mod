// Window.cpp

#include "Window.h"

/**
 * @brief 构造函数
 * @param width 窗口宽度
 * @param height 窗口高度
 * @param title 主窗口标题
 * @param pointCloudTitle 点云窗口标题（已废弃，保留兼容）
 */
Window::Window(unsigned int width, unsigned int height, 
               const std::string& title, const std::string&)
    : windowWidth(width), windowHeight(height),
      mainWindow(sf::VideoMode(width, height), title) {
    
    initialize();
}

/**
 * @brief 析构函数
 */
Window::~Window() = default;

/**
 * @brief 初始化窗口
 * @details 配置窗口属性，设置垂直同步和帧率限制
 */
void Window::initialize() {
    // 配置主窗口
    mainWindow.setVerticalSyncEnabled(true);
    mainWindow.setFramerateLimit(60);
    

}

/**
 * @brief 获取主游戏窗口
 * @return 主游戏窗口的引用
 */
sf::RenderWindow& Window::getMainWindow() {
    return mainWindow;
}



/**
 * @brief 获取主窗口尺寸
 * @return 主窗口的尺寸
 */
sf::Vector2u Window::getMainWindowSize() const {
    return mainWindow.getSize();
}



/**
 * @brief 检查主窗口是否打开
 * @return true如果窗口打开
 */
bool Window::isMainWindowOpen() const {
    return mainWindow.isOpen();
}



/**
 * @brief 关闭所有窗口
 */
void Window::closeAll() {
    mainWindow.close();
}

/**
 * @brief 关闭所有窗口（别名）
 */
void Window::close() {
    closeAll();
}

/**
 * @brief 处理窗口事件
 * @param event 要处理的事件
 * @return true如果窗口应该继续运行
 */
bool Window::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::Closed) {
        closeAll();
        return false;
    }
    return true;
}

/**
 * @brief 清空主窗口
 * @param color 清屏颜色
 */
void Window::clearMainWindow(const sf::Color& color) {
    mainWindow.clear(color);
}



/**
 * @brief 显示主窗口内容
 */
void Window::displayMainWindow() {
    mainWindow.display();
}



/**
 * @brief 处理所有窗口的事件
 */
void Window::handleEvents() {
    sf::Event event;
    
    // 处理主窗口事件
    while (mainWindow.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            closeAll();
        }
    }
}