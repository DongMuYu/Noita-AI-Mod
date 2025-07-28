// src/world/Parser.cpp
// 地图生成器：使用随机游走算法
// 负责根据难度生成随机关卡，并标记墙体结构供渲染/逻辑使用
// 对外接口：parseLevel() 和 nextLevel()
// -----------------------------------------------------------------
#include "Parser.h"
#include "../core/Constants.h"
#include <random>
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

using namespace std;
namespace Parser {

/*================ 常量与工具 =================*/
constexpr char WALL   = '1';                      // 墙体标记
constexpr char PATH   = '0';                      // 可通行路径标记
constexpr char PLAYER = 'P';                      // 玩家出生点标记
constexpr char TARGET = 'T';                      // 目标点标记
constexpr int MIN_WALL_NEIGHBORS = 2;             // 墙体平滑阈值
constexpr int CLEAR_RADIUS = 1;                   // 玩家/目标周围清空半径
static std::mt19937 rng(std::random_device{}());  // 全局随机引擎

/*================ 辅助工具函数 ================*/
// 判断坐标 (x, y) 是否在宽为 w、高为 h 的区域范围内（包含边界）
bool isInBounds(int x, int y, int w, int h)   { return x >= 0 && x < w && y >= 0 && y < h; }
// 判断坐标 (x, y) 是否在宽为 w、高为 h 的区域有效范围内（不包含边界）
bool isValid(int x, int y, int w, int h)      { return x > 0 && x < w-1 && y > 0 && y < h-1; }

// 统计 8 邻域内的墙数量
int countWallNeighbors(const std::vector<std::string>& m, int x, int y) {
    int cnt = 0;
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx)
            if (dx || dy) cnt += (m[y+dy][x+dx] == WALL);
    return cnt;
}

/*================ 地形生成函数 ================*/

// 随机游走生成洞穴状基础地形，并记录玩家和目标位置
struct WalkResult {
    std::vector<std::string> map;
    std::pair<int, int> playerPos;
    std::pair<int, int> targetPos;
};

WalkResult drunkardsWalkWithPositions(int w, int h, int steps) {
    std::vector<std::string> m(h, std::string(w, WALL));
    int x = w / 2, y = h / 2;
    
    // 记录随机位置用于玩家和目标
    std::vector<std::pair<int, int>> pathPositions;
    
    for (int i = 0; i < steps; ++i) {
        m[y][x] = PATH;
        pathPositions.emplace_back(x, y);
        
        int dir = std::uniform_int_distribution<int>(0, 3)(rng);
        switch (dir) {
            case 0: y = std::max(1, y - 1); break;
            case 1: x = std::min(w-2, x + 1); break;
            case 2: y = std::min(h-2, y + 1); break;
            case 3: x = std::max(1, x - 1); break;
        }
    }
    
    // 从随机路径中随机选择玩家和目标位置
    std::pair<int, int> playerPos, targetPos;
    
    if (!pathPositions.empty()) {
        // 随机选择玩家位置
        int playerIndex = std::uniform_int_distribution<int>(0, (int)pathPositions.size()-1)(rng);
        playerPos = pathPositions[playerIndex];
        cout << "Parser : Player: (" << playerPos.first << ", " << playerPos.second << ")" << endl;
        
        // 随机选择目标位置，排除玩家位置
        std::vector<std::pair<int, int>> availableTargets;
        for (const auto& pos : pathPositions) {
            if (pos != playerPos) {
                availableTargets.push_back(pos);
            }
        }
        
        if (!availableTargets.empty()) {
            int targetIndex = std::uniform_int_distribution<int>(0, (int)availableTargets.size()-1)(rng);
            targetPos = availableTargets[targetIndex];
        } else {
            // 如果所有位置都被玩家位置占用，使用备用位置
            targetPos = {playerPos.first + 1, playerPos.second + 1};
        }
        cout << "Parser : Target: (" << targetPos.first << ", " << targetPos.second << ")" << endl;

    } else {
        // 如果没有路径位置，使用中心附近的安全位置
        cout << "Parser : No path positions found. Using center." << endl;
        playerPos = {x, y};
        targetPos = {x + 1, y + 1};
    }
    
    return {m, playerPos, targetPos};
}

// 平滑处理：减少孤立墙体
void smoothMap(std::vector<std::string>& m) {
    int w = m[0].size(), h = m.size();
    for (int y = 1; y < h-1; ++y)
        for (int x = 1; x < w-1; ++x)
            if (m[y][x] == WALL && countWallNeighbors(m, x, y) < MIN_WALL_NEIGHBORS)
                m[y][x] = PATH;
}

// 以(cx,cy)为中心，半径r的圆形区域清空
void clearArea(std::vector<std::string>& m, int cx, int cy, int r) {
    int w = m[0].size(), h = m.size();
    for (int dy = -r; dy <= r; ++dy)
        for (int dx = -r; dx <= r; ++dx) {
            int nx = std::clamp(cx + dx, 0, w-1);
            int ny = std::clamp(cy + dy, 0, h-1);
            // 避免覆盖玩家或目标标记
            if (m[ny][nx] != PLAYER && m[ny][nx] != TARGET) {
                m[ny][nx] = PATH;
            }
        }
}

/*================ 主生成函数 ================*/
std::vector<std::string> generateRandomMap(float difficulty) {
    // 确保有足够的步数生成可通行区域
    int minSteps = W * H * 0.0001f;  // 最小的地图应该是可通行的
    int steps = std::max(minSteps, static_cast<int>(W * H * difficulty));
    
    // 使用随机游走算法生成基础地图
    auto result = drunkardsWalkWithPositions(W, H, steps);
    auto map = result.map;
    
    smoothMap(map);
    
    // 确保地图中心区域有足够的可通行空间
    int centerX = W/2, centerY = H/2;
    clearArea(map, centerX, centerY, 1);

    // 使用随机游走算法生成的位置作为基础
    auto playerPos = result.playerPos;
    auto targetPos = result.targetPos;
    
    // 统一处理玩家和目标位置
    auto placeEntity = [&](std::pair<int, int>& pos, char marker) {
        // 保存原始位置用于调试
        int originalX = pos.first;
        int originalY = pos.second;
        
        // 约束坐标到有效范围内
        pos.first = std::clamp(pos.first, 1, W-2);
        pos.second = std::clamp(pos.second, 1, H-2);
        
        // 确保位置是可通行的
        if (map[pos.second][pos.first] != PATH) {
            map[pos.second][pos.first] = PATH;
        }

        // 清空周围区域，但避免覆盖其他实体
        clearArea(map, pos.first, pos.second, CLEAR_RADIUS);
        
        // 放置实体标记
        map[pos.second][pos.first] = marker;
        
        std::cout << "[DEBUG] " << (marker == PLAYER ? "Player" : "Target") 
                  << " original: (" << originalX << ", " << originalY 
                  << ") final: (" << pos.first << ", " << pos.second << ")\n";
    };
    
    placeEntity(playerPos, PLAYER);
    placeEntity(targetPos, TARGET);
    
    std::cout << "[DEBUG] After placeEntity - Player at (" << playerPos.first << "," << playerPos.second 
              << ") Target at (" << targetPos.first << "," << targetPos.second << ")" << std::endl;
    
    // 验证P和T是否正确放置
    bool hasPlayer = false, hasTarget = false;
    int playerX = -1, playerY = -1, targetX = -1, targetY = -1;
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            if (map[y][x] == PLAYER) {
                hasPlayer = true;
                playerX = x;
                playerY = y;
            }
            if (map[y][x] == TARGET) {
                hasTarget = true;
                targetX = x;
                targetY = y;
            }
        }
    }
    std::cout << "[DEBUG] Pre-validation - P found at (" << playerX << "," << playerY 
              << ") T found at (" << targetX << "," << targetY << ")" << std::endl;
    
    if (!hasPlayer || !hasTarget) {
        std::cout << "\033[31m[ERROR] Parser : Map validation failed - P:" << hasPlayer << " T:" << hasTarget << "\033[0m" << std::endl;
        // 强制在中心放置
        if (!hasPlayer) {
            map[H/2][W/2] = PLAYER;
            playerPos = {W/2, H/2};
        }
        if (!hasTarget) {
            map[H/2 + 2][W/2 + 2] = TARGET;
            targetPos = {W/2 + 2, H/2 + 2};
        }
    }
    
    std::cout << "\033[34m[DEBUG] Parser : Final map validation - P:" << playerPos.first << "," << playerPos.second 
              << " T:" << targetPos.first << "," << targetPos.second << "\033[0m" << std::endl;

    // 统一设置边界墙
    auto setBoundaryWalls = [&](std::vector<std::string>& m) {
        for (int x = 0; x < W; ++x) {
            m[0][x]   = (m[0][x]   == PLAYER || m[0][x]   == TARGET) ? m[0][x]   : WALL;
            m[H-1][x] = (m[H-1][x] == PLAYER || m[H-1][x] == TARGET) ? m[H-1][x] : WALL;
        }
        for (int y = 0; y < H; ++y) {
            m[y][0]   = (m[y][0]   == PLAYER || m[y][0]   == TARGET) ? m[y][0]   : WALL;
            m[y][W-1] = (m[y][W-1] == PLAYER || m[y][W-1] == TARGET) ? m[y][W-1] : WALL;
        }
    };
    
    setBoundaryWalls(map);
    
    // 最终验证：确保P和T在返回前仍然存在
    bool finalHasPlayer = false, finalHasTarget = false;
    for (const auto& row : map) {
        if (row.find(PLAYER) != std::string::npos) finalHasPlayer = true;
        if (row.find(TARGET) != std::string::npos) finalHasTarget = true;
    }
    
    if (!finalHasPlayer || !finalHasTarget) {
        std::cout << "\033[33m[WARNING] Parser(Final) : Final validation failed - P:" << finalHasPlayer 
                  << " T:" << finalHasTarget << " - Forcing placement\033[0m" << std::endl;
        // 边界墙设置后强制放置
        if (!finalHasPlayer) {
            int safeX = std::clamp(W/2, 1, W-2);
            int safeY = std::clamp(H/2, 1, H-2);
            map[safeY][safeX] = PLAYER;
        }
        if (!finalHasTarget) {
            int safeX = std::clamp(W/2 + 2, 1, W-2);
            int safeY = std::clamp(H/2 + 2, 1, H-2);
            map[safeY][safeX] = TARGET;
        }
    }
    
    std::cout << "\033[33m[INFO] Parser(Final) : Map generation complete - P:" << (finalHasPlayer||true) 
              << " T:" << (finalHasTarget||true) << "\033[0m" << std::endl;

    return map;
}



/*================ 对外接口 =================*/
static float currentDifficulty = 0.01f;

std::vector<std::string> parseLevel() {
    return generateRandomMap(currentDifficulty);
}

void nextLevel() {
    currentDifficulty = std::min(currentDifficulty + 0.0001f, 4.0f);
}

} // namespace Parser