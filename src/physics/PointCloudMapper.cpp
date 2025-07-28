// PointCloudMapper.cpp

#include "PointCloudMapper.h"
#include <fstream>
#include <sstream>
#include <climits>

PointCloudMapper::PointCloudMapper() {
    // 初始化渲染纹理
    pointCloudTexture.create(800, 600);  // 默认大小，可根据需要调整
}

void PointCloudMapper::addScanPoint(const sf::Vector2f& worldPos, bool isObstacle) {
    // 将世界坐标转换为网格坐标
    int gridX = static_cast<int>(worldPos.x / 32.0f);  // TILE = 32
    int gridY = static_cast<int>(worldPos.y / 32.0f);
    
    // 创建键值对
    std::pair<int, int> key = {gridX, gridY};
    
    // 更新或创建点数据
    auto& point = pointCloud[key];
    point.scanCount++;
    
    // 更新置信度
    if (isObstacle) {
        point.confidence = std::min(1.0f, point.confidence + 0.1f);
        point.isObstacle = true;
        point.color = sf::Color(255, 0, 0, static_cast<sf::Uint8>(point.confidence * 255));
    } else {
        point.confidence = std::max(0.0f, point.confidence - 0.05f);
        point.color = sf::Color(0, 255, 0, static_cast<sf::Uint8>(point.confidence * 255));
    }
    
    textureDirty = true;
}

void PointCloudMapper::addRayScanResults(const std::vector<RayHitInfo>& rayHits) {
    for (const auto& hit : rayHits) {
        if (hit.hit) {
            addScanPoint(hit.hitPoint, true);
        }
    }
}

const std::unordered_map<std::pair<int, int>, PointCloudMapper::PointData, PointCloudMapper::PairHash>& PointCloudMapper::getPointCloud() const {
    return pointCloud;
}

void PointCloudMapper::clear() {
    pointCloud.clear();
    scanPoints.clear();
    pointVertices.clear();
    textureDirty = true;
}

void PointCloudMapper::render(sf::RenderWindow& window, const sf::Vector2f& playerPos) {
    // 设置视图以玩家为中心
    sf::View view = window.getDefaultView();
    view.setCenter(playerPos);
    window.setView(view);
    
    // 绘制点云
    for (const auto& pair : pointCloud) {
        const auto& key = pair.first;
        const auto& point = pair.second;
        if (point.confidence > 0.1f) {  // 只显示置信度较高的点
            sf::RectangleShape cell(sf::Vector2f(31.0f, 31.0f));
            cell.setPosition(key.first * 32.0f + 0.5f, key.second * 32.0f + 0.5f);
            cell.setFillColor(point.color);
            window.draw(cell);
        }
    }
    
    // 绘制玩家位置标记
    sf::CircleShape playerMarker(5);
    playerMarker.setFillColor(sf::Color::Blue);
    playerMarker.setPosition(playerPos.x - 2.5f, playerPos.y - 2.5f);
    window.draw(playerMarker);
}

sf::FloatRect PointCloudMapper::getBounds() const {
    if (pointCloud.empty()) {
        return sf::FloatRect(0, 0, 0, 0);
    }
    
    int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;
    
    for (const auto& pair : pointCloud) {
        const auto& key = pair.first;
        const auto& point = pair.second;
        minX = std::min(minX, key.first);
        minY = std::min(minY, key.second);
        maxX = std::max(maxX, key.first);
        maxY = std::max(maxY, key.second);
    }
    
    return sf::FloatRect(
        minX * 32.0f, 
        minY * 32.0f, 
        (maxX - minX + 1) * 32.0f, 
        (maxY - minY + 1) * 32.0f
    );
}

void PointCloudMapper::exportToCSV(const std::string& filename) const {
    std::ofstream file(filename);
    file << "GridX,GridY,WorldX,WorldY,ScanCount,Confidence,IsObstacle\n";
    
    for (const auto& pair : pointCloud) {
        const auto& key = pair.first;
        const auto& point = pair.second;
        file << key.first << "," << key.second << ","
             << key.first * 32.0f << "," << key.second * 32.0f << ","
             << point.scanCount << "," << point.confidence << ","
             << (point.isObstacle ? "1" : "0") << "\n";
    }
    
    file.close();
}