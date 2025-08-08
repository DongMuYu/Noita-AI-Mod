#include "AIController.h"
#include "DataCollector.h"
#include "../../core/Constants.h"
#include <random>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cmath>
#include <sstream>

// HistoryBuffer实现
HistoryBuffer::HistoryBuffer() {
    stateHistory.clear();
    actionHistory.clear();
}

void HistoryBuffer::addState(const std::vector<float>& state) {
    stateHistory.push_back(state);
    if (stateHistory.size() > HISTORY_SIZE) {
        stateHistory.pop_front();
    }
}

void HistoryBuffer::addAction(const std::vector<float>& action) {
    actionHistory.push_back(action);
    if (actionHistory.size() > HISTORY_SIZE) {
        actionHistory.pop_front();
    }
}

std::vector<std::vector<float>> HistoryBuffer::getStateSequence() const {
    return std::vector<std::vector<float>>(stateHistory.begin(), stateHistory.end());
}

std::vector<std::vector<float>> HistoryBuffer::getActionSequence() const {
    return std::vector<std::vector<float>>(actionHistory.begin(), actionHistory.end());
}

bool HistoryBuffer::isFull() const {
    return stateHistory.size() >= HISTORY_SIZE;
}

void HistoryBuffer::clear() {
    stateHistory.clear();
    actionHistory.clear();
}

AIController::AIController() : aiEnabled(false), modelLoaded(false),  rng(std::random_device{}()) {
    historyBuffer = std::make_unique<HistoryBuffer>();
}

AIController::~AIController() {
}

AIController::Action AIController::decideAction(Player& player, Map& map, RayCasting& rayCaster) {
    if (!aiEnabled) {
        // 如果AI未启用，返回默认动作
        return AIController::Action{0, 0};
    }

    try {
        // 直接提取特征
        std::vector<float> features = extractFeatures(player, map, rayCaster);
        
        // 使用模型预测动作
        if (modelLoaded) {
            return predictAction(features);
        } else {
            // 无模型时使用随机策略
            std::cerr << "AI decision error: Model not loaded" << std::endl;
            return getRandomAction();
        }
    } catch (const std::exception& e) {
        std::cerr << "AI decision error: " << e.what() << std::endl;
        return getRandomAction();
    }
}

// 包含原始数据返回的动作预测函数（用于调试）
AIController::ActionResult AIController::decideActionWithDetails(Player& player, Map& map, RayCasting& rayCaster) {
    if (!aiEnabled) {
        // 如果AI未启用，返回默认动作
        return AIController::ActionResult{{0, 0}, {0.0f, 0.0f}};
    }

    try {
        // 提取当前帧特征
        std::vector<float> features = extractFeatures(player, map, rayCaster);
        
        // 更新历史缓冲区
        historyBuffer->addState(features);
        
        // 获取状态序列
        auto stateSequence = historyBuffer->getStateSequence();
        
        // 根据配置选择预测方式
        if (historyBuffer->isFull() && modelLoaded) {
            // 使用序列学习模式
            return predictSequenceAction(stateSequence);
        } else {
            // 使用传统单帧预测
            return predictActionWithDetails(features);
        }
    } catch (const std::exception& e) {
        std::cerr << "AI decision error: " << e.what() << std::endl;
        Action randomAction = getRandomAction();
        return AIController::ActionResult{randomAction, {0.0f, 0.0f}};
    }
}

std::vector<float> AIController::extractFeatures(const Player& player, Map& map, RayCasting& rayCaster) {
    std::vector<float> features;
    
    // 获取玩家位置
    sf::Vector2f position = player.getPosition();
    sf::Vector2f velocity = player.getVelocity();
    float energy = player.getCurrentEnergy();
    bool isGrounded = player.isOnGround();
    
    // 获取目标位置
    sf::Vector2f target = map.getTargetPosition();
    
    // 计算到目标的距离和角度
    sf::Vector2f diff = target - position;
    float distanceToTarget = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    float angleToTarget = std::atan2(diff.y, diff.x);
    
    // 获取射线检测结果
    sf::Vector2f playerCenter = position + sf::Vector2f(7.5f, 7.5f); // 玩家中心点
    auto rayResults = rayCaster.castRays(playerCenter, map.getLevelData());
    
    // 位置特征 (2维)
    features.push_back(position.x);
    features.push_back(position.y);
    
    // 速度特征 (2维)
    features.push_back(velocity.x);
    features.push_back(velocity.y);
    
    // 能量特征 (1维)
    features.push_back(energy);

    // 地面接触状态 (1维)
    features.push_back(isGrounded ? 1.0f : 0.0f);
    
    // 目标相关特征 (4维)
    features.push_back(distanceToTarget);
    features.push_back(angleToTarget);
    features.push_back(target.x);
    features.push_back(target.y);
    
    // 射线检测结果特征 (120维 = 60距离 + 60命中状态)
    for (const auto& ray : rayResults) {
        features.push_back(ray.distance);
    }
    
    for (const auto& ray : rayResults) {
        features.push_back(ray.hit ? 1.0f : 0.0f);
    }

    // 共130维的数据
    
    // 特征标准化
    const float maxRayDistance = 150.0f;
    const float maxDistance = 1350.0f;
    const float maxVelocity = 240.832f;
    const float maxEnergy = 150.0f;
    const float maxAngle = 3.142f;
    
    for (size_t i = 0; i < features.size(); ++i) {
        if (i < 2) { // 位置 (0-1)
            features[i] = features[i] / maxDistance;
        } else if (i >= 2 && i < 4) { // 速度 (2-3)
            features[i] = features[i] / maxVelocity;
        } else if (i == 4) { // 能量 (4)
            features[i] = features[i] / maxEnergy;
        } else if (i == 5) { // 地面状态 (5) - 已经是0/1，无需标准化
            // 保持不变
        } else if (i == 6) { // 目标距离 (6)
            features[i] = std::min(features[i] / maxDistance * 1.4143f, 1.0f);
        } else if (i == 7) { // 目标角度 (7)
            features[i] = features[i] / maxAngle; // 归一化到[-1, 1]
        } else if (i >= 8 && i < 10) { // 目标坐标 (8-9)
            features[i] = features[i] / maxDistance;
        } else if (i >= 10 && i < 70) { // 射线距离 (10-69)
            features[i] = std::min(features[i] / maxRayDistance, 1.0f);
        } else if (i >= 70) { // 射线命中 (70-129)
            // 已经是0/1，无需标准化
        }
    }
    
    return features;
}

// 序列特征提取 - 扩展特征维度以包含时序信息
std::vector<float> AIController::extractSequenceFeatures(const std::vector<std::vector<float>>& stateSequence) {
    std::vector<float> sequenceFeatures;
    
    if (stateSequence.empty()) {
        return std::vector<float>(132 * HISTORY_SIZE, 0.0f);
    }
    
    // 为每帧添加扩展特征
    for (const auto& frame : stateSequence) {
        std::vector<float> extendedFrame = frame;
        
        // 添加时序特征：位置变化率、速度变化率
        if (extendedFrame.size() >= 6) {
            // 计算位置变化率 (需要历史数据)
            extendedFrame.push_back(0.0f);  // dx/dt placeholder
            extendedFrame.push_back(0.0f);  // dy/dt placeholder
        }
        
        // 确保每帧有132个特征
        while (extendedFrame.size() < 132) {
            extendedFrame.push_back(0.0f);
        }
        
        sequenceFeatures.insert(sequenceFeatures.end(), extendedFrame.begin(), extendedFrame.begin() + 132);
    }
    
    // 填充到150帧
    while (sequenceFeatures.size() < 132 * HISTORY_SIZE) {
        sequenceFeatures.push_back(0.0f);
    }
    
    return sequenceFeatures;
}

AIController::Action AIController::predictAction(const std::vector<float>& features) {
    if (modelWeights.empty() || modelBias.empty()) {
        return getRandomAction();
    }
    
    try {
        ActionResult result = predictActionWithDetails(features);
        return result.action;
    } catch (const std::exception& e) {
        std::cerr << "Model prediction error: " << e.what() << std::endl;
        return getRandomAction();
    }
}

AIController::ActionResult AIController::predictActionWithDetails(const std::vector<float>& features) {
    if (modelWeights.empty() || modelBias.empty()) {
        Action randomAction = getRandomAction();
        return ActionResult{randomAction, {0.0f, 0.0f}};
    }
    
    try {
        // 深度神经网络前向传播 - 与SLTrainer一致的网络结构
        // 网络结构: 130输入 -> 256 -> 128 -> 64 -> 32 -> 16 -> 2输出
        
        const int inputDim = 130;
        const int hiddenDim1 = 256;
        const int hiddenDim2 = 128;
        const int hiddenDim3 = 64;
        const int hiddenDim4 = 32;
        const int hiddenDim5 = 16;
        const int outputDim = 2;
        
        // 验证输入维度
        if (features.size() != inputDim) {
            std::cerr << "Input feature dimension mismatch: " << features.size() << " vs " << inputDim << std::endl;
            Action randomAction = getRandomAction();
            return ActionResult{randomAction, {0.0f, 0.0f}};
        }
        
        // 验证模型参数完整性（6层网络）
        if (modelWeights.size() < 6 || modelBias.size() < 6) {
            std::cerr << "Model parameters incomplete" << std::endl;
            Action randomAction = getRandomAction();
            return ActionResult{randomAction, {0.0f, 0.0f}};
        }
        
        // 第一层：130 -> 256 (ReLU)
        std::vector<float> hidden1(hiddenDim1);
        for (int i = 0; i < hiddenDim1; ++i) {
            hidden1[i] = modelBias[0][i];
            for (int j = 0; j < inputDim; ++j) {
                hidden1[i] += features[j] * modelWeights[0][j * hiddenDim1 + i];
            }
            hidden1[i] = std::max(0.0f, hidden1[i]); // ReLU激活
        }
        
        // 第二层：256 -> 128 (ReLU)
        std::vector<float> hidden2(hiddenDim2);
        for (int i = 0; i < hiddenDim2; ++i) {
            hidden2[i] = modelBias[1][i];
            for (int j = 0; j < hiddenDim1; ++j) {
                hidden2[i] += hidden1[j] * modelWeights[1][j * hiddenDim2 + i];
            }
            hidden2[i] = std::max(0.0f, hidden2[i]); // ReLU激活
        }
        
        // 第三层：128 -> 64 (ReLU)
        std::vector<float> hidden3(hiddenDim3);
        for (int i = 0; i < hiddenDim3; ++i) {
            hidden3[i] = modelBias[2][i];
            for (int j = 0; j < hiddenDim2; ++j) {
                hidden3[i] += hidden2[j] * modelWeights[2][j * hiddenDim3 + i];
            }
            hidden3[i] = std::max(0.0f, hidden3[i]); // ReLU激活
        }
        
        // 第四层：64 -> 32 (ReLU)
        std::vector<float> hidden4(hiddenDim4);
        for (int i = 0; i < hiddenDim4; ++i) {
            hidden4[i] = modelBias[3][i];
            for (int j = 0; j < hiddenDim3; ++j) {
                hidden4[i] += hidden3[j] * modelWeights[3][j * hiddenDim4 + i];
            }
            hidden4[i] = std::max(0.0f, hidden4[i]); // ReLU激活
        }
        
        // 第五层：32 -> 16 (ReLU)
        std::vector<float> hidden5(hiddenDim5);
        for (int i = 0; i < hiddenDim5; ++i) {
            hidden5[i] = modelBias[4][i];
            for (int j = 0; j < hiddenDim4; ++j) {
                hidden5[i] += hidden4[j] * modelWeights[4][j * hiddenDim5 + i];
            }
            hidden5[i] = std::max(0.0f, hidden5[i]); // ReLU激活
        }
        
        // 第六层：16 -> 2 (线性输出)
        std::vector<float> output(outputDim);
        for (int i = 0; i < outputDim; ++i) {
            output[i] = modelBias[5][i];
            for (int j = 0; j < hiddenDim5; ++j) {
                output[i] += hidden5[j] * modelWeights[5][j * outputDim + i];
            }
        }
        
        // 创建包含原始数据和离散化动作的结果
        AIController::Action action;
        AIController::OriginalActionData originalData;
        originalData.moveX = output[0];
        originalData.useEnergy = output[1];
        
        // 离散化 moveX 值，根据符号转换为 -1, 0 或 1
        if (output[0] > 0.33f) {
            action.moveX = 1;
        } else if (output[0] < -0.33f) {
            action.moveX = -1;
        } else {
            action.moveX = 0;
        }
        
        // 离散化 useEnergy 值，大于 0 则使用能量，否则不使用
        action.useEnergy = (output[1] > 0.5f) ? 1 : 0;
        
        return ActionResult{action, originalData};
        
    } catch (const std::exception& e) {
        std::cerr << "Model prediction error: " << e.what() << std::endl;
        Action randomAction = getRandomAction();
        return ActionResult{randomAction, {0.0f, 0.0f}};
    }
}

AIController::Action AIController::getRandomAction() {
    std::uniform_int_distribution<int> moveDist(-1, 1);
    std::uniform_int_distribution<int> energyDist(0, 1);
    
    return Action{moveDist(rng), energyDist(rng)};
}

// 基于序列预测动作
AIController::ActionResult AIController::predictSequenceAction(const std::vector<std::vector<float>>& stateSequence) {
    if (modelWeights.empty() || modelBias.empty()) {
        Action randomAction = getRandomAction();
        return ActionResult{randomAction, {0.0f, 0.0f}};
    }
    
    try {
        // 检查历史缓冲区是否已满
        if (!historyBuffer->isFull()) {
            // 使用传统方法作为后备
            auto currentFeatures = stateSequence.empty() ? 
                std::vector<float>(130, 0.0f) : stateSequence.back();
            return predictActionWithDetails(currentFeatures);
        }
        
        // 提取序列特征
        std::vector<float> sequenceFeatures = extractSequenceFeatures(stateSequence);
        
        // 序列网络前向传播
        // 网络结构: [150×132, LSTM256, LSTM128, 64, 32, 16, 2]
        const int inputDim = 132 * 150;
        const int lstm1Dim = 256;
        const int lstm2Dim = 128;
        const int hiddenDim3 = 64;
        const int hiddenDim4 = 32;
        const int hiddenDim5 = 16;
        const int outputDim = 2;
        
        // 验证序列模型参数
        if (modelWeights.size() < 12) {  // 序列网络需要更多参数
            return predictActionWithDetails(stateSequence.back());
        }
        
        // 简化的序列处理 - 使用现有权重结构
        // 在实际应用中需要专门的LSTM权重加载
        return predictActionWithDetails(stateSequence.back());
        
    } catch (const std::exception& e) {
        std::cerr << "Sequence prediction error: " << e.what() << std::endl;
        Action randomAction = getRandomAction();
        return ActionResult{randomAction, {0.0f, 0.0f}};
    }
}

bool AIController::isAIEnabled() const {
    return aiEnabled;
}


void AIController::loadModel(const std::string& filename) {
    try {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Cannot open model file: " << filename << std::endl;
            modelLoaded = false;
            return;
        }
        
        modelWeights.clear();
        modelBias.clear();
        
        // 加载二进制格式的模型权重和偏置
        // 网络结构: 130输入 -> 256 -> 128 -> 64 -> 32 -> 16 -> 2输出
        const int layerSizes[6] = {130*256, 256*128, 128*64, 64*32, 32*16, 16*2};
        const int biasSizes[6] = {256, 128, 64, 32, 16, 2};
        
        modelWeights.resize(6);
        modelBias.resize(6);
        
        // 加载权重
        for (int layer = 0; layer < 6; ++layer) {
            size_t size;
            file.read(reinterpret_cast<char*>(&size), sizeof(size));
            
            if (size != layerSizes[layer]) {
                std::cerr << "Model weight dimension mismatch: " << size << " vs " << layerSizes[layer] << std::endl;
                modelLoaded = false;
                return;
            }
            
            modelWeights[layer].resize(size);
            file.read(reinterpret_cast<char*>(modelWeights[layer].data()), size * sizeof(float));
        }
        
        // 加载偏置
        for (int layer = 0; layer < 6; ++layer) {
            size_t size;
            file.read(reinterpret_cast<char*>(&size), sizeof(size));
            
            if (size != biasSizes[layer]) {
                std::cerr << "Model bias dimension mismatch: " << size << " vs " << biasSizes[layer] << std::endl;
                modelLoaded = false;
                return;
            }
            
            modelBias[layer].resize(size);
            file.read(reinterpret_cast<char*>(modelBias[layer].data()), size * sizeof(float));
        }
        
        modelLoaded = true;
        std::cout << "Successfully loaded model: " << filename << std::endl;
        std::cout << "Network structure: 130 -> 256 -> 128 -> 64 -> 32 -> 16 -> 2" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading model: " << e.what() << std::endl;
        modelLoaded = false;
    }
}

void AIController::setAIEnabled(bool enabled) {
    aiEnabled = enabled;
    std::cout << "[DEBUG] AI " << (enabled ? "enabled" : "disabled") << std::endl;
}

