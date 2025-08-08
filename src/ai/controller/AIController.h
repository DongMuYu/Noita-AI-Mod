#pragma once

#include "../../entity/Player.h"
#include "../../core/Map.h"
#include "../pathfinding/RayCasting.h"
#include <vector>
#include <memory>
#include <random> 
#include <deque>
#include <SFML/System/Vector2.hpp>

// 前向声明
class DataCollector;

// 序列训练数据结构
struct SequenceTrainingData {
    std::vector<std::vector<float>> stateSequence;  // 150帧状态序列
    std::vector<std::vector<float>> actionSequence; // 150帧动作序列
    std::vector<float> targetAction;                // 下一帧目标动作
    int sequenceLength = 150;                       // 序列长度
};

// 历史状态缓冲区
class HistoryBuffer {
public:
    static constexpr int HISTORY_SIZE = 150;
    
    HistoryBuffer();
    
    void addState(const std::vector<float>& state);
    void addAction(const std::vector<float>& action);
    std::vector<std::vector<float>> getStateSequence() const;
    std::vector<std::vector<float>> getActionSequence() const;
    bool isFull() const;
    void clear();
    
private:
    std::deque<std::vector<float>> stateHistory;
    std::deque<std::vector<float>> actionHistory;
};

class AIController {
public:
    AIController();
    virtual ~AIController();

    // AI控制器的动作结构体
    struct Action {
        int moveX;      // -1: 左移, 0: 不动, 1: 右移
        int useEnergy;  // 0: 不飞行, 1: 飞行
    };

    struct OriginalActionData {
        float moveX;
        float useEnergy;
    };
    
    // 包含原始数据的动作结果
    struct ActionResult {
        Action action;
        OriginalActionData originalData;
    };
    
    // 根据当前游戏状态决定AI动作
    Action decideAction(Player& player, Map& map, RayCasting& rayCaster);
    
    // 根据当前游戏状态决定AI动作（包含原始数据）
    ActionResult decideActionWithDetails(Player& player, Map& map, RayCasting& rayCaster);
    
    // 加载训练好的模型
    void loadModel(const std::string& filename);
    
    // 设置是否使用AI控制（true=AI控制，false=人工控制）
    void setAIEnabled(bool enabled);
    
    // 检查是否启用AI控制
    bool isAIEnabled() const;

private:
    // 将游戏状态转换为模型输入特征
    std::vector<float> extractFeatures(const Player& player, Map& map, RayCasting& rayCaster);
    
    // 模型单帧预测
    Action predictAction(const std::vector<float>& features);
    
    // 模型单帧预测（包含原始数据）
    ActionResult predictActionWithDetails(const std::vector<float>& features);

    // 模型序列信息预测
    ActionResult predictSequenceAction(const std::vector<std::vector<float>>& stateSequence);
    
    // 简单的随机策略（无模型时备用）
    Action getRandomAction();

    // 序列特征提取
    std::vector<float> extractSequenceFeatures(const std::vector<std::vector<float>>& stateSequence);

    private:
    // 模型权重和偏置（4层网络）
    std::vector<std::vector<float>> modelWeights;
    std::vector<std::vector<float>> modelBias;
    
    // 控制状态
    bool aiEnabled;
    bool modelLoaded;

    // 历史状态缓冲区
    std::unique_ptr<HistoryBuffer> historyBuffer;

    static constexpr int HISTORY_SIZE = 150;
    
    // 随机数生成器
    std::mt19937 rng;
};

// AI训练数据结构
struct AITrainingData {
    std::vector<float> features;
    AIController::Action action;
    AIController::OriginalActionData originalData;
};