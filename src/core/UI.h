#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <memory>

class UI {
public:
    struct EnergyBarData {
        float currentEnergy;
        float maxEnergy;
        sf::Vector2f position;
        sf::Vector2f size;
    };

    struct TextData {
        std::string text;
        sf::Vector2f position;
        unsigned int characterSize;
        sf::Color color;
        std::string fontName;
    };

    struct WarningData {
        std::string message;
        sf::Vector2f position;
        float dangerTimer;
        float maxTimer;
    };

    struct DebugInfoData {
        std::vector<std::string> lines;
        sf::Vector2f position;
        unsigned int characterSize;
        sf::Color color;
    };

    struct LineData {
        sf::Vector2f start;
        sf::Vector2f end;
        sf::Color color;
    };

    UI();
    ~UI() = default;

    // 初始化
    void initialize(sf::Font& font);

    // 渲染方法
    void renderEnergyBar(sf::RenderWindow& window, const EnergyBarData& data);
    void renderText(sf::RenderWindow& window, const TextData& data);
    void renderWarning(sf::RenderWindow& window, const WarningData& data);
    void renderDebugInfo(sf::RenderWindow& window, const DebugInfoData& data);
    void renderLine(sf::RenderWindow& window, const LineData& data);
    void renderShape(sf::RenderWindow& window, const sf::Shape& shape);

    // 便捷方法
    void renderEnergyBarAtTopRight(sf::RenderWindow& window, float currentEnergy, float maxEnergy, const sf::Vector2u& windowSize);
    void renderTimer(sf::RenderWindow& window, float time, const sf::Vector2f& position);
    void renderFPS(sf::RenderWindow& window, float fps, const sf::Vector2f& position);
    void renderDangerWarning(sf::RenderWindow& window, const std::string& message, float dangerTimer, const sf::Vector2u& windowSize);
    void renderPlayerTargetLine(sf::RenderWindow& window, const sf::Vector2f& playerPos, const sf::Vector2f& targetPos, float tileSize);

    // 设置字体
    void setFont(sf::Font& font);

private:
    sf::Font* font;
    sf::Text reusableText;
    sf::RectangleShape reusableRect;
    sf::VertexArray reusableLine;

    void setupDefaultStyles();
};