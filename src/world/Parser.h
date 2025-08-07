// src/world/Parser.h

#pragma once
#include <vector>
#include <string>
#include <utility>

/**
 * @file Parser.h
 * @brief 关卡解析器模块
 * @details 提供关卡数据解析和关卡进度管理功能，是游戏世界与关卡数据之间的桥梁
 * 主要职责：
 * - 从文件系统或生成算法获取关卡数据
 * - 将原始数据转换为游戏可识别的瓦片地图格式
 * - 在随机游走过程中生成实体
 * - 跟踪和管理关卡进度
 * - 根据关卡进度调整游戏难度
 */
namespace Parser {

    /**
     * @brief 解析当前关卡数据
     * @return 地图瓦片数组，每行是一个字符串，每个字符代表一个瓦片类型
     * @details 实现细节：
     * 1. 从关卡文件（通常位于data/levels目录）或 procedural 生成算法获取数据
     * 2. 瓦片字符定义：
     *    - '0': 空白/空气
     *    - '1': 固体墙壁
     *    - 'P': 玩家起始位置
     *    - 'T': 关卡出口
     *    - 'E': 敌人位置
     *    - 'I': 物品位置
     *    - 'O': 障碍物位置
     *    - '+': 强化道具位置
     *    - 'X': 陷阱位置
     *    - 'D': 装饰位置
     * 3. 在随机游走过程中动态生成实体，使用概率系统控制实体密度
     * 4. 返回的向量中，每个字符串代表地图的一行，行高和列宽由具体关卡决定
     * 5. 如果解析失败，返回空向量
     */
    std::vector<std::string> parseLevel();
    
    /**
     * @brief 检测地图中的墙结构
     * @param map 地图瓦片数组
     * @return 返回墙的标记信息，每个字符代表对应位置的墙标记状态
     * @details 墙的定义：竖直方向上堆叠的连续方块，具有至少一面无阻挡的垂直面
     * 被标记为墙部分的方块将用于渲染时的青色标记
     */
    std::vector<std::string> detectWalls(const std::vector<std::string>& map);
}