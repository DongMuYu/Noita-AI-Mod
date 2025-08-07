#pragma once

#include "../../entity/Player.h"
#include "../../core/Map.h"
#include "../pathfinding/RayCasting.h"
#include <vector>
#include <memory>
#include <random> 
#include <SFML/System/Vector2.hpp>

// 前向声明
class DataCollector;

class AIController {
public:
    AIController();
    virtual ~AIController();

    // AI控制器的动作结构体
    struct Action {
        int moveX;      // -1: 左移, 0: 不动, 1: 右移
        int useEnergy;  // 0: 不飞行, 1: 飞行
    };
    
    // 根据当前游戏状态决定AI动作
    Action decideAction(Player& player, Map& map, RayCasting& rayCaster);
    
    // 加载训练好的模型
    void loadModel(const std::string& filename);
    
    // 设置是否使用AI控制（true=AI控制，false=人工控制）
    void setAIEnabled(bool enabled);
    
    // 检查是否启用AI控制
    bool isAIEnabled() const;

private:
    // 将游戏状态转换为模型输入特征
    std::vector<float> extractFeatures(const Player& player, Map& map, RayCasting& rayCaster);
    
    // 模型预测
    Action predictAction(const std::vector<float>& features);
    
    // 简单的随机策略（无模型时备用）
    Action getRandomAction();
    
    // 模型权重和偏置（4层网络）
    std::vector<std::vector<float>> modelWeights;
    std::vector<std::vector<float>> modelBias;
    
    // 控制状态
    bool aiEnabled;
    bool modelLoaded;
    
    // 随机数生成器
    std::mt19937 rng;
};