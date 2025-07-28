#include "UI.h"
#include <iomanip>
#include <sstream>

UI::UI() : font(nullptr) {
    setupDefaultStyles();
}

void UI::initialize(sf::Font& font) {
    this->font = &font;
    reusableText.setFont(font);
}

void UI::setFont(sf::Font& font) {
    this->font = &font;
    reusableText.setFont(font);
}

void UI::setupDefaultStyles() {
    reusableText.setCharacterSize(14);
    reusableText.setFillColor(sf::Color::Black);
    
    reusableRect.setOutlineThickness(1.f);
    reusableRect.setOutlineColor(sf::Color::Black);
    
    reusableLine.setPrimitiveType(sf::Lines);
    reusableLine.resize(2);
}

void UI::renderEnergyBar(sf::RenderWindow& window, const EnergyBarData& data) {
    if (!font) return;
    
    // 背景
    reusableRect.setSize(data.size);
    reusableRect.setPosition(data.position);
    reusableRect.setFillColor(sf::Color(50, 50, 50));
    reusableRect.setOutlineColor(sf::Color::Black);
    window.draw(reusableRect);
    
    // 填充
    float energyRatio = data.currentEnergy / data.maxEnergy;
    if (energyRatio > 0) {
        reusableRect.setSize(sf::Vector2f(data.size.x * energyRatio, data.size.y));
        reusableRect.setFillColor(sf::Color(204, 153, 0));
        window.draw(reusableRect);
    }
}

void UI::renderText(sf::RenderWindow& window, const TextData& data) {
    if (!font) return;
    
    reusableText.setString(data.text);
    reusableText.setPosition(data.position);
    reusableText.setCharacterSize(data.characterSize);
    reusableText.setFillColor(data.color);
    window.draw(reusableText);
}

void UI::renderWarning(sf::RenderWindow& window, const WarningData& data) {
    if (!font) return;
    
    reusableText.setCharacterSize(16);
    reusableText.setFillColor(sf::Color::Red);
    reusableText.setPosition(data.position);
    
    std::stringstream warning;

    // 监测持续 1秒为危险区域后才显示警告 
    // 重置的时间为 2秒
    if (data.dangerTimer >= 1.0f) {
        warning << "DANGER! Reset in " << std::fixed << std::setprecision(1) 
                << (data.maxTimer - data.dangerTimer) << "s";
    }
    reusableText.setString(warning.str());
    window.draw(reusableText);
}

void UI::renderDebugInfo(sf::RenderWindow& window, const DebugInfoData& data) {
    if (!font) return;
    
    reusableText.setCharacterSize(data.characterSize);
    reusableText.setFillColor(data.color);
    
    float yOffset = 0;
    for (const auto& line : data.lines) {
        reusableText.setString(line);
        reusableText.setPosition(data.position.x, data.position.y + yOffset);
        window.draw(reusableText);
        yOffset += data.characterSize + 2;
    }
}

void UI::renderLine(sf::RenderWindow& window, const LineData& data) {
    reusableLine[0].position = data.start;
    reusableLine[1].position = data.end;
    reusableLine[0].color = data.color;
    reusableLine[1].color = data.color;
    window.draw(reusableLine);
}

void UI::renderShape(sf::RenderWindow& window, const sf::Shape& shape) {
    window.draw(shape);
}

void UI::renderEnergyBarAtTopRight(sf::RenderWindow& window, float currentEnergy, float maxEnergy, const sf::Vector2u& windowSize) {
    EnergyBarData data;
    data.currentEnergy = currentEnergy;
    data.maxEnergy = maxEnergy;
    data.position = sf::Vector2f(windowSize.x - 190, 10);
    data.size = sf::Vector2f(180, 20);
    renderEnergyBar(window, data);
}

void UI::renderTimer(sf::RenderWindow& window, float time, const sf::Vector2f& position) {
    TextData data;
    data.text = "Time: " + std::to_string(static_cast<int>(time)) + "s";
    data.position = position;
    data.characterSize = 14;
    data.color = sf::Color::Black;
    renderText(window, data);
}

void UI::renderFPS(sf::RenderWindow& window, float fps, const sf::Vector2f& position) {
    TextData data;
    data.text = "FPS: " + std::to_string(static_cast<int>(fps));
    data.position = position;
    data.characterSize = 14;
    data.color = sf::Color::Black;
    renderText(window, data);
}

void UI::renderDangerWarning(sf::RenderWindow& window, const std::string& message, float dangerTimer, const sf::Vector2u& windowSize) {
    WarningData data;
    data.message = message;
    data.position = sf::Vector2f(10, windowSize.y - 60);
    data.dangerTimer = dangerTimer;
    data.maxTimer = 2.0f;
    renderWarning(window, data);
}

void UI::renderPlayerTargetLine(sf::RenderWindow& window, const sf::Vector2f& playerPos, const sf::Vector2f& targetPos, float tileSize) {
    LineData data;
    data.start = playerPos + sf::Vector2f(tileSize/2, tileSize/2);
    data.end = targetPos + sf::Vector2f(tileSize/2, tileSize/2);
    data.color = sf::Color::Red;
    renderLine(window, data);
}