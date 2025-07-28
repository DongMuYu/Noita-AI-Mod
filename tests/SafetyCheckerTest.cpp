#include <iostream>
#include <vector>
#include <cassert>

// 简化的常量定义
constexpr int TILE = 15;

// 简化的实体类
class SimpleEntity {
public:
    float x, y;
    float width, height;
    
    SimpleEntity(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}
    
    bool intersects(float otherX, float otherY, float otherW, float otherH) const {
        return x < otherX + otherW && x + width > otherX &&
               y < otherY + otherH && y + height > otherY;
    }
};

// 安全检查器测试
class SafetyCheckerTest {
public:
    static void runAllTests() {
        std::cout << "=== SafetyChecker 逻辑正确性测试 ===" << std::endl;
        
        testMapBounds();
        testMapEdge();
        testWallCollision();
        testValidGround();
        
        std::cout << "测试完成!" << std::endl;
    }

private:
    static void testMapBounds() {
        SimpleEntity inside(30, 30, 30, 30);
        std::cout << "Inside bounds test: " << (isWithinMapBounds(inside, 5, 5) ? "true" : "false") << std::endl;
        
        SimpleEntity outside(-10, -10, 30, 30);
        std::cout << "Outside bounds test: " << (!isWithinMapBounds(outside, 5, 5) ? "true" : "false") << std::endl;
        
        std::cout << "地图边界测试完成" << std::endl;
    }
    
    static void testMapEdge() {
        // 地图大小: 5*15 = 75x75
        // 边缘阈值: 2*15 = 30
        SimpleEntity nearEdge(10, 10, 20, 20);
        std::cout << "Near edge test: " << (isNearMapEdge(nearEdge, 5, 5) ? "true" : "false") << std::endl;
        
        SimpleEntity farFromEdge(40, 40, 20, 20);
        std::cout << "Far from edge test: " << (isNearMapEdge(farFromEdge, 5, 5) ? "true" : "false") << std::endl;
        
        std::cout << "地图边缘测试完成" << std::endl;
    }
    
    static void testWallCollision() {
        SimpleEntity wall(0, 0, 30, 30);
        std::cout << "Wall collision test: " << (isCollidingWithWall(wall) ? "true" : "false") << std::endl;
        
        SimpleEntity open(45, 45, 30, 30);
        std::cout << "Open area test: " << (!isCollidingWithWall(open) ? "true" : "false") << std::endl;
        
        std::cout << "墙壁碰撞测试完成" << std::endl;
    }
    
    static void testValidGround() {
        SimpleEntity onGround(45, 45, 30, 30);
        std::cout << "Valid ground test: " << (isOnValidGround(onGround, 5, 5) ? "true" : "false") << std::endl;
        
        SimpleEntity inWall(0, 0, 30, 30);
        std::cout << "Invalid ground test: " << (!isOnValidGround(inWall, 5, 5) ? "true" : "false") << std::endl;
        
        std::cout << "有效地面测试完成" << std::endl;
    }
    
    static bool isWithinMapBounds(const SimpleEntity& entity, int mapWidth, int mapHeight) {
        float mapPixelWidth = static_cast<float>(mapWidth * TILE);
        float mapPixelHeight = static_cast<float>(mapHeight * TILE);
        
        return (entity.x >= 0 && entity.y >= 0 && 
                entity.x + entity.width <= mapPixelWidth && 
                entity.y + entity.height <= mapPixelHeight);
    }
    
    static bool isNearMapEdge(const SimpleEntity& entity, int mapWidth, int mapHeight) {
        float mapPixelWidth = static_cast<float>(mapWidth * TILE);
        float mapPixelHeight = static_cast<float>(mapHeight * TILE);
        float edgeThreshold = static_cast<float>(TILE * 2);
        
        return (entity.x < edgeThreshold || 
                entity.x + entity.width > mapPixelWidth - edgeThreshold ||
                entity.y < edgeThreshold || 
                entity.y + entity.height > mapPixelHeight - edgeThreshold);
    }
    
    static bool isCollidingWithWall(const SimpleEntity& entity) {
        // 简化的边界墙检测
        return entity.x < TILE || entity.y < TILE ||
               entity.x + entity.width > 4 * TILE ||
               entity.y + entity.height > 4 * TILE;
    }
    
    static bool isOnValidGround(const SimpleEntity& entity, int mapWidth, int mapHeight) {
        // 简化的地面检测 - 只要实体在地图范围内就认为地面有效
        float mapPixelWidth = static_cast<float>(mapWidth * TILE);
        float mapPixelHeight = static_cast<float>(mapHeight * TILE);
        
        return (entity.x >= 0 && entity.y >= 0 && 
                entity.x + entity.width <= mapPixelWidth && 
                entity.y + entity.height <= mapPixelHeight);
    }
};

int main() {
    SafetyCheckerTest::runAllTests();
    return 0;
}