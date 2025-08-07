// Renderer.cpp
#include "Renderer.h"
#include "Map.h"
#include "UI.h"
#include "../entity/Player.h"
#include "../core/Constants.h"
#include "../world/Parser.h"
#include <iostream>
#include <set>

// 常量定义
const sf::Color BACKGROUND_COLOR = sf::Color(100, 149, 237); // 天蓝色
const sf::Color TARGET_COLOR = sf::Color::Red;
const sf::Color PLAYER_SPAWN_COLOR = sf::Color(255, 255, 0); // 黄色
const sf::Color DANGER_WARNING_COLOR = sf::Color(255, 0, 0); // 红色
const sf::Color WALL_MARKER_COLOR = sf::Color(0, 255, 255); // 青色 - 墙标记颜色

Renderer::Renderer() : initialized(false), playerInDanger(false), dangerTimer(0.0f) {
}

Renderer::~Renderer() = default;

bool Renderer::initialize() {
    // 尝试多个路径加载字体
    std::vector<std::string> fontPaths = {
        "src/assets/fonts/SourceHanSansSC-Regular.otf",
        "../src/assets/fonts/SourceHanSansSC-Regular.otf",
        "../../src/assets/fonts/SourceHanSansSC-Regular.otf"
    };
    
    for (const auto& path : fontPaths) {
        if (font.loadFromFile(path)) {
            initialized = true;
            return true;
        }
    }
    
    std::cerr << "警告：无法加载字体文件，将使用默认字体" << std::endl;
    initialized = true; // 即使没有字体也标记为初始化成功
    return false;
}

sf::Font& Renderer::getFont() {
    return font;
}

void Renderer::renderMainWindow(sf::RenderWindow& window, 
                              Map& map, 
                              Player& player, 
                              UI& ui,
                              float gameTime, 
                              float fps,
                              bool showRayDebug,
                              const std::vector<RayHitInfo>& rayHits) {
    
    sf::Vector2u windowSize = window.getSize();
    
    // 清空屏幕
    clear(window, BACKGROUND_COLOR);
    
    // 渲染地图
    renderMap(window, map);
    
    // 渲染玩家
    renderPlayer(window, player, ui);
    
    // 渲染目标点
    renderTarget(window, map, ui);
    
    // 渲染能量条
    renderEnergyBar(window, ui, player.getCurrentEnergy(), player.getMaxEnergy(), windowSize);
    
    // 渲染计时器
    renderTimer(window, ui, gameTime, sf::Vector2f(windowSize.x - 190, 40));
    
    // 渲染FPS
    renderFPS(window, ui, fps, sf::Vector2f(windowSize.x - 190, 70));
    
    // 渲染玩家到目标的连线
    sf::Vector2f targetPos = map.getTargetPosition();
    renderPlayerTargetLine(window, ui, player.getPosition(), targetPos);
    
    // 渲染射线调试信息
    if (showRayDebug) {
        sf::Vector2f playerPos = player.getPosition() + sf::Vector2f(TILE/2, TILE/2);
        renderRayDebug(window, ui, rayHits, playerPos);
    }
    
    // 渲染危险警告
    renderDangerWarning(window, ui, windowSize);
    
    // 渲染玩家出生点
    renderPlayerSpawnPoint(window, ui, playerSpawnPosition);
    
    // 显示渲染结果
    display(window);
}

void Renderer::clear(sf::RenderWindow& window, const sf::Color& color) {
    window.clear(color);
}

void Renderer::display(sf::RenderWindow& window) {
    window.display();
}

void Renderer::setPlayerSpawnPosition(const sf::Vector2f& pos) {
    playerSpawnPosition = pos;
}

void Renderer::setDangerState(bool isInDanger, float timer) {
    playerInDanger = isInDanger;
    dangerTimer = timer;
}

void Renderer::renderMap(sf::RenderWindow& window, Map& map) {
    map.draw(window);
    
    // 获取地图数据并渲染墙标记
    const std::vector<std::string>& mapData = map.getLevelData();
    if (!mapData.empty()) {
        renderWallMarkers(window, map, mapData);
    }
}

void Renderer::renderPlayer(sf::RenderWindow& window, Player& player, UI& ui) {
    ui.renderShape(window, player.getShape());
}

void Renderer::renderTarget(sf::RenderWindow& window, Map& map, UI& ui) {
    sf::Vector2f targetPos = map.getTargetPosition();
    sf::RectangleShape targetShape(sf::Vector2f(TILE * 0.8f, TILE * 0.8f));
    targetShape.setPosition(targetPos.x + TILE * 0.1f, targetPos.y + TILE * 0.1f);
    targetShape.setFillColor(TARGET_COLOR);
    ui.renderShape(window, targetShape);
}

void Renderer::renderEnergyBar(sf::RenderWindow& window, 
                             UI& ui, 
                             float currentEnergy, 
                             float maxEnergy, 
                             const sf::Vector2u& windowSize) {
    ui.renderEnergyBarAtTopRight(window, currentEnergy, maxEnergy, windowSize);
}

void Renderer::renderTimer(sf::RenderWindow& window, 
                          UI& ui, 
                          float gameTime, 
                          const sf::Vector2f& position) {
    ui.renderTimer(window, gameTime, position);
}

void Renderer::renderFPS(sf::RenderWindow& window, 
                        UI& ui, 
                        float fps, 
                        const sf::Vector2f& position) {
    ui.renderFPS(window, fps, position);
}

void Renderer::renderPlayerTargetLine(sf::RenderWindow& window, 
                                    UI& ui,
                                    const sf::Vector2f& playerPos,
                                    const sf::Vector2f& targetPos) {
    if (targetPos.x >= 0 && targetPos.y >= 0) {
        ui.renderPlayerTargetLine(window, playerPos, targetPos, TILE);
    }
}

void Renderer::renderRayDebug(sf::RenderWindow& window, 
                            UI& ui,
                            const std::vector<RayHitInfo>& rayHits,
                            const sf::Vector2f& playerPos) {
    
    if (!rayHits.empty()) {
        // 用于存储被射线命中的瓦片位置，避免重复绘制
        std::set<std::pair<int, int>> hitTiles;
        
        // 绘制所有射线（完全为绿色）
        for (const auto& hit : rayHits) {
            sf::Vertex line[2];
            line[0].position = playerPos;
            line[0].color = sf::Color::Green;
            line[1].position = hit.hit ? hit.hitPoint : (playerPos + hit.direction * 150.0f);
            line[1].color = sf::Color::Green;  // 所有射线都是绿色
            window.draw(line, 2, sf::Lines);
            
            // 如果射线命中障碍物，记录命中的瓦片
            if (hit.hit) {
                int tileX = static_cast<int>(hit.hitPoint.x / TILE);
                int tileY = static_cast<int>(hit.hitPoint.y / TILE);
                hitTiles.insert(std::make_pair(static_cast<int>(tileX), static_cast<int>(tileY)));
                
                // 绘制黄色命中点
                sf::CircleShape hitPoint(3);
                hitPoint.setFillColor(sf::Color::Yellow);
                hitPoint.setPosition(hit.hitPoint.x - 1.5f, hit.hitPoint.y - 1.5f);
                window.draw(hitPoint);
            }
        }
        
        // 绘制被射线命中的瓦片（黄色半透明矩形）
        for (const auto& tilePos : hitTiles) {
            sf::RectangleShape tileRect(sf::Vector2f(TILE, TILE));
            tileRect.setFillColor(sf::Color(255, 255, 0, 128)); // 黄色半透明
            tileRect.setPosition(tilePos.first * TILE, tilePos.second * TILE);
            tileRect.setOutlineThickness(1);
            tileRect.setOutlineColor(sf::Color::Yellow);
            window.draw(tileRect);
        }
        
        // 显示射线信息
        UI::DebugInfoData debugData;
        debugData.position = sf::Vector2f(10, 100);
        debugData.characterSize = 12;
        debugData.color = sf::Color::Black;
        
        debugData.lines.push_back("Raycast Debug (R to toggle)");
        debugData.lines.push_back("Total rays: " + std::to_string(rayHits.size()));
        
        int hitCount = 0;
        float avgDistance = 0;
        for (const auto& hit : rayHits) {
            if (hit.hit) {
                hitCount++;
                avgDistance += hit.distance;
            }
        }
        
        if (hitCount > 0) {
            avgDistance /= hitCount;
            debugData.lines.push_back("Hits: " + std::to_string(hitCount));
            debugData.lines.push_back("Avg distance: " + std::to_string(static_cast<int>(avgDistance)) + "px");
        }
        
        ui.renderDebugInfo(window, debugData);
    }
}

void Renderer::renderDangerWarning(sf::RenderWindow& window, UI& ui, const sf::Vector2u& windowSize) {
    if (playerInDanger) {
        ui.renderDangerWarning(window, "", dangerTimer, windowSize);
    }
}

void Renderer::renderPlayerSpawnPoint(sf::RenderWindow& window, UI& ui, sf::Vector2f& position) {
    // if (position.x >= 0 && position.y >= 0) {
    //     sf::CircleShape playerSpawnShape(TILE / 3.0f, 5); // 五角星形状
    //     playerSpawnShape.setPosition(position.x + TILE/2 - TILE/6, position.y + TILE/2 - TILE/6);
    //     playerSpawnShape.setFillColor(PLAYER_SPAWN_COLOR);    // 亮黄色
    //     playerSpawnShape.setOutlineColor(sf::Color(255, 165, 0)); // 橙色边框
    //     playerSpawnShape.setOutlineThickness(2.0f);
    //     ui.renderShape(window, playerSpawnShape);
    // }
}

void Renderer::renderWallMarkers(sf::RenderWindow& window, Map& map, const std::vector<std::string>& mapData) {
    // if (mapData.empty()) return;
    
    // int width = mapData[0].size();
    // int height = mapData.size();
    
    // // 创建半透明的青色矩形来标记墙
    // sf::RectangleShape wallMarker(sf::Vector2f(TILE, TILE));
    // wallMarker.setFillColor(sf::Color(WALL_MARKER_COLOR.r, WALL_MARKER_COLOR.g, WALL_MARKER_COLOR.b, 128)); // 半透明青色
    // wallMarker.setOutlineThickness(1);
    // wallMarker.setOutlineColor(WALL_MARKER_COLOR);
    
    // // 遍历地图数据，渲染'W', '3', '4'标记的墙结构
    // for (int y = 0; y < height; ++y) {
    //     for (int x = 0; x < width; ++x) {
    //         if (mapData[y][x] == 'W' || mapData[y][x] == '3' || mapData[y][x] == '4') {
    //             wallMarker.setPosition(x * TILE, y * TILE);
    //             if (mapData[y][x] == '3' || mapData[y][x] == '4') {
    //                 wallMarker.setFillColor(sf::Color(0, 0, 139, 128)); // 深蓝色半透明
    //             } else {
    //                 wallMarker.setFillColor(sf::Color(WALL_MARKER_COLOR.r, WALL_MARKER_COLOR.g, WALL_MARKER_COLOR.b, 128));
    //             }
    //             window.draw(wallMarker);
    //         }
    //     }
    // }
}






