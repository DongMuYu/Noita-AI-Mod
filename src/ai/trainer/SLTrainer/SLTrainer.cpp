// SLTrainer.cpp
// 监督学习训练器实现文件
// 实现基于行为克隆的监督学习训练功能

#include "SLTrainer.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>
#include <iostream>

// SLTrainer 构造函数
SLTrainer::SLTrainer(const TrainingConfig& config) : config(config) {
    // 初始化行为克隆代理
    agent = std::make_unique<BehaviorCloningAgent>(config);
    resetTrainingStats();
}

// SLTrainer 析构函数
SLTrainer::~SLTrainer() {
    // 自动清理资源
}

// 从回合数据训练模型
void SLTrainer::trainFromData(const std::vector<SimpleML::EpisodeData>& episodes) {
    // 分割数据集
    std::vector<SimpleML::TrainingData> trainSet, valSet;
    splitDataset(episodes, trainSet, valSet);
    
    if (trainSet.empty()) {
        std::cerr << "Training dataset is empty, cannot train." << std::endl;
        return;
    }
    
    // 数据增强
    auto augmentedTrain = augmentData(trainSet);
    
    // 预处理数据
    auto trainStates = preprocessStates(augmentedTrain);
    auto trainActions = preprocessActions(augmentedTrain);
    auto valStates = preprocessStates(valSet);
    auto valActions = preprocessActions(valSet);
    
    // 训练循环
    int bestEpoch = 0;
    float bestValLoss = std::numeric_limits<float>::max();
    int patienceCounter = 0;
    
    for (int epoch = 0; epoch < config.epochs; ++epoch) {
        // 打乱训练数据
        std::vector<size_t> indices(trainStates.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), std::mt19937{std::random_device{}()});
        
        float epochLoss = 0.0f;
        int batchCount = 0;
        
        // 批次训练
        int totalBatches = static_cast<int>((indices.size() + config.batchSize - 1) / config.batchSize);
        for (size_t i = 0; i < indices.size(); i += config.batchSize) {
            size_t end = std::min(i + config.batchSize, indices.size());
            std::vector<std::vector<float>> batchStates, batchActions;

            for (size_t j = i; j < end; ++j) {
                batchStates.push_back(trainStates[indices[j]]);
                batchActions.push_back(trainActions[indices[j]]);
            }

            float batchLoss = agent->train(batchStates, batchActions, config.learningRate, epoch * batchCount);
            epochLoss += batchLoss;
            ++batchCount;

            // 打印批次进度
            if (batchCount % 10 == 0 || batchCount == totalBatches) {
                std::cout << "Epoch " << epoch << ", Batch " << batchCount << "/" << totalBatches
                          << ", Batch Loss: " << batchLoss << std::endl;
            }
        }
        
        // 计算验证损失
        float valLoss = agent->evaluate(valStates, valActions);
        
        // 更新统计信息
        stats.trainingLoss = epochLoss / batchCount;
        stats.validationLoss = valLoss;
        stats.epochsCompleted = epoch + 1;
        
        // 早停检查
        if (valLoss < bestValLoss) {
            bestValLoss = valLoss;
            bestEpoch = epoch;
            patienceCounter = 0;
        } else {
            ++patienceCounter;
            if (patienceCounter >= config.earlyStoppingPatience) {
                std::cout << "Early stopping triggered at epoch " << epoch << std::endl;
                break;
            }
        }
        
        // 每10个epoch打印一次进度
        if (epoch % 10 == 0) {
            std::cout << "Epoch " << epoch << ": Train Loss = " << stats.trainingLoss
                     << ", Val Loss = " << stats.validationLoss << std::endl;
        }
        
        // 每20个epoch保存一次中间模型
        if (epoch % 20 == 0 && epoch > 0) {
            std::string intermediateModelPath = "models/SL_models/intermediate_model_epoch_" + std::to_string(epoch) + ".bin";
            saveModel(intermediateModelPath);
            std::cout << "Saved intermediate model at epoch " << epoch << " to " << intermediateModelPath << std::endl;
        }
    }
    
    stats.bestEpoch = bestEpoch;
    stats.bestValidationLoss = bestValLoss;
}

// 单步训练
float SLTrainer::trainStep(const std::vector<SimpleML::TrainingData>& batch, int step) {
    return trainStepWithLearningRate(batch, config.learningRate, step);
}

// 使用指定学习率的单步训练
float SLTrainer::trainStepWithLearningRate(const std::vector<SimpleML::TrainingData>& batch, 
                                          float learningRate, int step) {
    auto states = preprocessStates(batch);
    auto actions = preprocessActions(batch);
    return agent->train(states, actions, learningRate, step);
}

// 保存模型
void SLTrainer::saveModel(const std::string& filename) {
    agent->save(filename);
}

// 加载模型
void SLTrainer::loadModel(const std::string& filename) {
    agent->load(filename);
}

// 评估模型性能
float SLTrainer::evaluate(const std::vector<SimpleML::EpisodeData>& testEpisodes) {
    std::vector<SimpleML::TrainingData> testSet;
    for (const auto& episode : testEpisodes) {
        testSet.insert(testSet.end(), episode.states.begin(), episode.states.end());
    }
    
    auto states = preprocessStates(testSet);
    auto actions = preprocessActions(testSet);
    
    float loss = agent->evaluate(states, actions);
    
    // 计算预测
    std::vector<std::vector<float>> predictions;
    for (const auto& state : states) {
        predictions.push_back(agent->forward(state));
    }
    
    stats.validationAccuracy = computeAccuracy(predictions, actions);
    return loss;
}

// 使用模型进行预测
std::vector<float> SLTrainer::predict(const std::vector<float>& state) {
    return agent->predict(state);
}

// 从回合数据分割数据集
void SLTrainer::splitDataset(const std::vector<SimpleML::EpisodeData>& episodes,
                           std::vector<SimpleML::TrainingData>& trainSet,
                           std::vector<SimpleML::TrainingData>& valSet) {
    std::vector<SimpleML::TrainingData> allData;
    for (const auto& episode : episodes) {
        allData.insert(allData.end(), episode.states.begin(), episode.states.end());
    }
    
    splitDatasetFromTrainingData(allData, trainSet, valSet);
}

// 从训练数据分割数据集
void SLTrainer::splitDatasetFromTrainingData(const std::vector<SimpleML::TrainingData>& data,
                                           std::vector<SimpleML::TrainingData>& trainSet,
                                           std::vector<SimpleML::TrainingData>& valSet) {
    std::vector<SimpleML::TrainingData> shuffled = data;
    std::shuffle(shuffled.begin(), shuffled.end(), std::mt19937{std::random_device{}()});
    
    size_t trainSize = static_cast<size_t>(data.size() * (1.0f - config.validationSplit));
    
    trainSet.assign(shuffled.begin(), shuffled.begin() + trainSize);
    valSet.assign(shuffled.begin() + trainSize, shuffled.end());
}

// 设置训练配置
void SLTrainer::setConfig(const TrainingConfig& config) {
    this->config = config;
}

// 获取当前训练配置
SLTrainer::TrainingConfig SLTrainer::getConfig() const {
    return config;
}

// 获取训练统计信息
SLTrainer::TrainingStats SLTrainer::getTrainingStats() const {
    return stats;
}

// 重置训练统计信息
void SLTrainer::resetTrainingStats() {
    stats = TrainingStats();
}

// 分析特征重要性
std::vector<SLTrainer::FeatureImportance> SLTrainer::analyzeFeatureImportance(
    const std::vector<SimpleML::EpisodeData>& episodes) {
    
    std::vector<FeatureImportance> importance;
    
    // 收集所有数据
    std::vector<SimpleML::TrainingData> allData;
    for (const auto& episode : episodes) {
        allData.insert(allData.end(), episode.states.begin(), episode.states.end());
    }
    
    if (allData.empty()) {
        return importance;
    }
    
    // 简单的特征重要性分析：计算每个特征与动作的相关系数
    int stateDim = static_cast<int>(allData[0].state.size());
    int actionDim = static_cast<int>(allData[0].action.size());
    
    for (int i = 0; i < stateDim; ++i) {
        FeatureImportance fi;
        fi.name = "Feature_" + std::to_string(i);
        
        // 计算与第一个动作维度的相关性
        std::vector<float> featureValues, actionValues;
        for (const auto& data : allData) {
            featureValues.push_back(data.state[i]);
            if (!data.action.empty()) {
                actionValues.push_back(data.action[0]);
            }
        }
        
        if (!actionValues.empty()) {
            float meanFeature = std::accumulate(featureValues.begin(), featureValues.end(), 0.0f) / featureValues.size();
            float meanAction = std::accumulate(actionValues.begin(), actionValues.end(), 0.0f) / actionValues.size();
            
            float numerator = 0.0f;
            float denomFeature = 0.0f;
            float denomAction = 0.0f;
            
            for (size_t j = 0; j < featureValues.size(); ++j) {
                float diffFeature = featureValues[j] - meanFeature;
                float diffAction = actionValues[j] - meanAction;
                numerator += diffFeature * diffAction;
                denomFeature += diffFeature * diffFeature;
                denomAction += diffAction * diffAction;
            }
            
            if (denomFeature > 0 && denomAction > 0) {
                fi.correlation = numerator / std::sqrt(denomFeature * denomAction);
                fi.importance = std::abs(fi.correlation);
            } else {
                fi.correlation = 0.0f;
                fi.importance = 0.0f;
            }
        } else {
            fi.correlation = 0.0f;
            fi.importance = 0.0f;
        }
        
        importance.push_back(fi);
    }
    
    // 按重要性排序
    std::sort(importance.begin(), importance.end(), 
              [](const FeatureImportance& a, const FeatureImportance& b) {
                  return a.importance > b.importance;
              });
    
    return importance;
}

// 预处理状态数据
std::vector<std::vector<float>> SLTrainer::preprocessStates(const std::vector<SimpleML::TrainingData>& data) {
    std::vector<std::vector<float>> states;
    for (const auto& d : data) {
        states.push_back(d.state);
    }
    return states;
}

// 预处理动作数据
std::vector<std::vector<float>> SLTrainer::preprocessActions(const std::vector<SimpleML::TrainingData>& data) {
    std::vector<std::vector<float>> actions;
    for (const auto& d : data) {
        actions.push_back(d.action);
    }
    return actions;
}

// 数据增强
std::vector<SimpleML::TrainingData> SLTrainer::augmentData(const std::vector<SimpleML::TrainingData>& data) {
    // 简单的数据增强：添加小的随机噪声
    std::vector<SimpleML::TrainingData> augmented = data;
    std::mt19937 gen(std::random_device{}());
    std::normal_distribution<float> dist(0.0f, 0.01f);
    
    for (auto& sample : augmented) {
        for (auto& val : sample.state) {
            val += dist(gen);
        }
    }
    
    return augmented;
}

// 计算准确率
float SLTrainer::computeAccuracy(const std::vector<std::vector<float>>& predictions,
                               const std::vector<std::vector<float>>& targets) {
    if (predictions.empty() || targets.empty()) return 0.0f;
    
    float totalError = 0.0f;
    int totalElements = 0;
    
    for (size_t i = 0; i < predictions.size(); ++i) {
        for (size_t j = 0; j < predictions[i].size(); ++j) {
            float diff = predictions[i][j] - targets[i][j];
            totalError += std::abs(diff);
            ++totalElements;
        }
    }
    
    return 1.0f - (totalError / totalElements);
}

// 计算均方误差
float SLTrainer::computeMSE(const std::vector<std::vector<float>>& predictions,
                           const std::vector<std::vector<float>>& targets) {
    if (predictions.empty() || targets.empty()) return 0.0f;
    
    float totalError = 0.0f;
    int totalElements = 0;
    
    for (size_t i = 0; i < predictions.size(); ++i) {
        for (size_t j = 0; j < predictions[i].size(); ++j) {
            float diff = predictions[i][j] - targets[i][j];
            totalError += diff * diff;
            ++totalElements;
        }
    }
    
    return totalError / totalElements;
}

// BehaviorCloningAgent 构造函数
SLTrainer::BehaviorCloningAgent::BehaviorCloningAgent(const TrainingConfig& config) : config(config) {
    initializeNetwork();
}

// BehaviorCloningAgent 析构函数
SLTrainer::BehaviorCloningAgent::~BehaviorCloningAgent() {
    // 自动清理
}

// 初始化神经网络 - 深化网络结构以适应复杂环境
void SLTrainer::BehaviorCloningAgent::initializeNetwork() {
    // 初始化深度网络结构：130输入 -> 256 -> 128 -> 64 -> 32 -> 16 -> 动作维度输出
    
    // 扩展隐藏层维度
    const int hiddenDim1 = 256;  // 第一层扩展
    const int hiddenDim2 = 128;  // 第二层保持
    const int hiddenDim3 = 64;   // 第三层保持
    const int hiddenDim4 = 32;   // 新增层
    const int hiddenDim5 = 16;   // 新增层
    
    // 初始化权重矩阵（6层网络）
    network.resize(6);
    network[0].resize(inputDim * hiddenDim1);   // 130×256
    network[1].resize(hiddenDim1 * hiddenDim2); // 256×128
    network[2].resize(hiddenDim2 * hiddenDim3); // 128×64
    network[3].resize(hiddenDim3 * hiddenDim4); // 64×32
    network[4].resize(hiddenDim4 * hiddenDim5); // 32×16
    network[5].resize(hiddenDim5 * outputDim);  // 16×2
    
    // 初始化偏置
    biases.resize(6);
    biases[0].resize(hiddenDim1);
    biases[1].resize(hiddenDim2);
    biases[2].resize(hiddenDim3);
    biases[3].resize(hiddenDim4);
    biases[4].resize(hiddenDim5);
    biases[5].resize(outputDim);
    
    // Xavier初始化 - 为每层计算合适的范围
    std::mt19937 gen(std::random_device{}());
    
    // 逐层初始化权重（6层）
    float ranges[6] = {
        std::sqrt(6.0f / (inputDim + hiddenDim1)),
        std::sqrt(6.0f / (hiddenDim1 + hiddenDim2)),
        std::sqrt(6.0f / (hiddenDim2 + hiddenDim3)),
        std::sqrt(6.0f / (hiddenDim3 + hiddenDim4)),
        std::sqrt(6.0f / (hiddenDim4 + hiddenDim5)),
        std::sqrt(6.0f / (hiddenDim5 + outputDim))
    };
    
    for (int layer = 0; layer < 6; ++layer) {
        std::uniform_real_distribution<float> dist(-ranges[layer], ranges[layer]);
        for (auto& w : network[layer]) w = dist(gen);
        std::fill(biases[layer].begin(), biases[layer].end(), 0.0f);
    }
    
    // 初始化Adam优化器参数（扩展到6层）
    adamM.resize(6);
    adamV.resize(6);
    adamMB.resize(6);
    adamVB.resize(6);
    
    for (int i = 0; i < 6; ++i) {
        adamM[i].assign(network[i].size(), 0.0f);
        adamV[i].assign(network[i].size(), 0.0f);
        adamMB[i].assign(biases[i].size(), 0.0f);
        adamVB[i].assign(biases[i].size(), 0.0f);
    }
}

// 前向传播
std::vector<float> SLTrainer::BehaviorCloningAgent::forward(const std::vector<float>& state) {
    return forwardNetwork(state);
}

// 离散化预测输出
std::vector<float> SLTrainer::BehaviorCloningAgent::predict(const std::vector<float>& input) {
    std::vector<float> output = forwardNetwork(input);
    
    // 离散化输出：确保输出是满足操控游戏的离散值
    // 第一维（左右移动）：-1（左移），0（不动），1（右移）
    // 第二维（上下移动）：0（下降），1（上升/飞行）
    if (outputDim >= 1) {
        // 第一维离散化
        if (output[0] < -0.5f) {
            output[0] = -1.0f;
        } else if (output[0] > 0.5f) {
            output[0] = 1.0f;
        } else {
            output[0] = 0.0f;
        }
    }
    
    if (outputDim >= 2) {
        // 第二维离散化
        output[1] = (output[1] >= 0.5f) ? 1.0f : 0.0f;
    }
    
    return output;
}

// 神经网络前向传播 - 深度网络版本
std::vector<float> SLTrainer::BehaviorCloningAgent::forwardNetwork(const std::vector<float>& input) {
    // 扩展隐藏层维度
    const int hiddenDim1 = 256;
    const int hiddenDim2 = 128;
    const int hiddenDim3 = 64;
    const int hiddenDim4 = 32;
    const int hiddenDim5 = 16;
    
    // 第一层：130 -> 256 (ReLU)
    std::vector<float> hidden1(hiddenDim1);
    for (int i = 0; i < hiddenDim1; ++i) {
        hidden1[i] = biases[0][i];
        for (int j = 0; j < inputDim; ++j) {
            hidden1[i] += input[j] * network[0][j * hiddenDim1 + i];
        }
        hidden1[i] = relu(hidden1[i]);
    }
    
    // 第二层：256 -> 128 (ReLU)
    std::vector<float> hidden2(hiddenDim2);
    for (int i = 0; i < hiddenDim2; ++i) {
        hidden2[i] = biases[1][i];
        for (int j = 0; j < hiddenDim1; ++j) {
            hidden2[i] += hidden1[j] * network[1][j * hiddenDim2 + i];
        }
        hidden2[i] = relu(hidden2[i]);
    }
    
    // 第三层：128 -> 64 (ReLU)
    std::vector<float> hidden3(hiddenDim3);
    for (int i = 0; i < hiddenDim3; ++i) {
        hidden3[i] = biases[2][i];
        for (int j = 0; j < hiddenDim2; ++j) {
            hidden3[i] += hidden2[j] * network[2][j * hiddenDim3 + i];
        }
        hidden3[i] = relu(hidden3[i]);
    }
    
    // 第四层：64 -> 32 (ReLU)
    std::vector<float> hidden4(hiddenDim4);
    for (int i = 0; i < hiddenDim4; ++i) {
        hidden4[i] = biases[3][i];
        for (int j = 0; j < hiddenDim3; ++j) {
            hidden4[i] += hidden3[j] * network[3][j * hiddenDim4 + i];
        }
        hidden4[i] = relu(hidden4[i]);
    }
    
    // 第五层：32 -> 16 (ReLU)
    std::vector<float> hidden5(hiddenDim5);
    for (int i = 0; i < hiddenDim5; ++i) {
        hidden5[i] = biases[4][i];
        for (int j = 0; j < hiddenDim4; ++j) {
            hidden5[i] += hidden4[j] * network[4][j * hiddenDim5 + i];
        }
        hidden5[i] = relu(hidden5[i]);
    }
    
    // 第六层：16 -> 2 (线性输出)
    std::vector<float> output(outputDim);
    for (int i = 0; i < outputDim; ++i) {
        output[i] = biases[5][i];
        for (int j = 0; j < hiddenDim5; ++j) {
            output[i] += hidden5[j] * network[5][j * outputDim + i];
        }
    }
    
    return output;
}

// 训练模型
float SLTrainer::BehaviorCloningAgent::train(const std::vector<std::vector<float>>& states,
                                           const std::vector<std::vector<float>>& actions,
                                           float learningRate, int step) {
    // 计算批次梯度
    std::vector<std::vector<float>> biasGrads;
    auto weightGrads = computeGradients(states, actions, biasGrads);
    
    // Adam优化器更新
    adamUpdate(weightGrads, biasGrads, learningRate, step);
    
    // 返回平均损失
    return evaluate(states, actions);
}

// 评估模型
float SLTrainer::BehaviorCloningAgent::evaluate(const std::vector<std::vector<float>>& states,
                                              const std::vector<std::vector<float>>& actions) {
    float totalLoss = 0.0f;
    for (size_t i = 0; i < states.size(); ++i) {
        auto prediction = forwardNetwork(states[i]);
        totalLoss += computeLoss(prediction, actions[i]);
    }
    return totalLoss / states.size();
}

// 保存模型
void SLTrainer::BehaviorCloningAgent::save(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) return;
    
    // 保存网络权重和偏置
    for (const auto& layer : network) {
        size_t size = layer.size();
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
        file.write(reinterpret_cast<const char*>(layer.data()), size * sizeof(float));
    }
    
    for (const auto& bias : biases) {
        size_t size = bias.size();
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
        file.write(reinterpret_cast<const char*>(bias.data()), size * sizeof(float));
    }
}

// 加载模型
void SLTrainer::BehaviorCloningAgent::load(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return;
    
    // 加载网络权重和偏置
    for (auto& layer : network) {
        size_t size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        layer.resize(size);
        file.read(reinterpret_cast<char*>(layer.data()), size * sizeof(float));
    }
    
    for (auto& bias : biases) {
        size_t size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        bias.resize(size);
        file.read(reinterpret_cast<char*>(bias.data()), size * sizeof(float));
    }
}

// ReLU激活函数
float SLTrainer::BehaviorCloningAgent::relu(float x) {
    return std::max(0.0f, x);
}

// 计算损失函数（均方误差）
float SLTrainer::BehaviorCloningAgent::computeLoss(const std::vector<float>& predicted,
                                                   const std::vector<float>& target) {
    float loss = 0.0f;
    for (size_t i = 0; i < predicted.size(); ++i) {
        float diff = predicted[i] - target[i];
        loss += diff * diff;
    }
    return loss / predicted.size();
}

// 计算正则化损失（L2正则化）
float SLTrainer::BehaviorCloningAgent::computeRegularizationLoss(float lambda) {
    float regLoss = 0.0f;
    for (const auto& layer : network) {
        for (float weight : layer) {
            regLoss += weight * weight;
        }
    }
    return lambda * regLoss;
}

// 计算批次梯度 - 深度网络版本
std::vector<std::vector<float>> SLTrainer::BehaviorCloningAgent::computeGradients(
    const std::vector<std::vector<float>>& states,
    const std::vector<std::vector<float>>& actions,
    std::vector<std::vector<float>>& outBiasGrads) {
    
    // 确保输出梯度结构与当前网络匹配
    if (outBiasGrads.size() != 6) {
        outBiasGrads.resize(6);
    }
    outBiasGrads[0].assign(hiddenDim1, 0.0f);
    outBiasGrads[1].assign(hiddenDim2, 0.0f);
    outBiasGrads[2].assign(hiddenDim3, 0.0f);
    outBiasGrads[3].assign(hiddenDim4, 0.0f);
    outBiasGrads[4].assign(hiddenDim5, 0.0f);
    outBiasGrads[5].assign(outputDim, 0.0f);
    
    // 创建权重梯度结构
    std::vector<std::vector<float>> weightGrads(6);
    weightGrads[0].assign(inputDim * hiddenDim1, 0.0f);
    weightGrads[1].assign(hiddenDim1 * hiddenDim2, 0.0f);
    weightGrads[2].assign(hiddenDim2 * hiddenDim3, 0.0f);
    weightGrads[3].assign(hiddenDim3 * hiddenDim4, 0.0f);
    weightGrads[4].assign(hiddenDim4 * hiddenDim5, 0.0f);
    weightGrads[5].assign(hiddenDim5 * outputDim, 0.0f);
    
    // 计算梯度
    for (size_t s = 0; s < states.size(); ++s) {
        const auto& state = states[s];
        const auto& action = actions[s];
        
        // 前向传播 - 存储中间结果用于反向传播
        std::vector<float> hidden1(hiddenDim1);
        for (int i = 0; i < hiddenDim1; ++i) {
            hidden1[i] = biases[0][i];
            for (int j = 0; j < inputDim; ++j) {
                hidden1[i] += state[j] * network[0][j * hiddenDim1 + i];
            }
            hidden1[i] = relu(hidden1[i]);
        }
        
        std::vector<float> hidden2(hiddenDim2);
        for (int i = 0; i < hiddenDim2; ++i) {
            hidden2[i] = biases[1][i];
            for (int j = 0; j < hiddenDim1; ++j) {
                hidden2[i] += hidden1[j] * network[1][j * hiddenDim2 + i];
            }
            hidden2[i] = relu(hidden2[i]);
        }
        
        std::vector<float> hidden3(hiddenDim3);
        for (int i = 0; i < hiddenDim3; ++i) {
            hidden3[i] = biases[2][i];
            for (int j = 0; j < hiddenDim2; ++j) {
                hidden3[i] += hidden2[j] * network[2][j * hiddenDim3 + i];
            }
            hidden3[i] = relu(hidden3[i]);
        }
        
        std::vector<float> hidden4(hiddenDim4);
        for (int i = 0; i < hiddenDim4; ++i) {
            hidden4[i] = biases[3][i];
            for (int j = 0; j < hiddenDim3; ++j) {
                hidden4[i] += hidden3[j] * network[3][j * hiddenDim4 + i];
            }
            hidden4[i] = relu(hidden4[i]);
        }
        
        std::vector<float> hidden5(hiddenDim5);
        for (int i = 0; i < hiddenDim5; ++i) {
            hidden5[i] = biases[4][i];
            for (int j = 0; j < hiddenDim4; ++j) {
                hidden5[i] += hidden4[j] * network[4][j * hiddenDim5 + i];
            }
            hidden5[i] = relu(hidden5[i]);
        }
        
        std::vector<float> output(outputDim);
        for (int i = 0; i < outputDim; ++i) {
            output[i] = biases[5][i];
            for (int j = 0; j < hiddenDim5; ++j) {
                output[i] += hidden5[j] * network[5][j * outputDim + i];
            }
        }
        
        // 反向传播 - 从输出层开始
        std::vector<float> outputError(outputDim);
        for (int i = 0; i < outputDim; ++i) {
            outputError[i] = 2.0f * (output[i] - action[i]) / outputDim;
        }
        
        // 第六层梯度
        for (int i = 0; i < outputDim; ++i) {
            outBiasGrads[5][i] += outputError[i];
            for (int j = 0; j < hiddenDim5; ++j) {
                weightGrads[5][j * outputDim + i] += hidden5[j] * outputError[i];
            }
        }
        
        // 第五层误差和梯度
        std::vector<float> hidden5Error(hiddenDim5);
        for (int i = 0; i < hiddenDim5; ++i) {
            hidden5Error[i] = 0.0f;
            for (int j = 0; j < outputDim; ++j) {
                hidden5Error[i] += network[5][i * outputDim + j] * outputError[j];
            }
            hidden5Error[i] *= (hidden5[i] > 0.0f) ? 1.0f : 0.0f;
            outBiasGrads[4][i] += hidden5Error[i];
            for (int j = 0; j < hiddenDim4; ++j) {
                weightGrads[4][j * hiddenDim5 + i] += hidden4[j] * hidden5Error[i];
            }
        }
        
        // 第四层误差和梯度
        std::vector<float> hidden4Error(hiddenDim4);
        for (int i = 0; i < hiddenDim4; ++i) {
            hidden4Error[i] = 0.0f;
            for (int j = 0; j < hiddenDim5; ++j) {
                hidden4Error[i] += network[4][i * hiddenDim5 + j] * hidden5Error[j];
            }
            hidden4Error[i] *= (hidden4[i] > 0.0f) ? 1.0f : 0.0f;
            outBiasGrads[3][i] += hidden4Error[i];
            for (int j = 0; j < hiddenDim3; ++j) {
                weightGrads[3][j * hiddenDim4 + i] += hidden3[j] * hidden4Error[i];
            }
        }
        
        // 第三层误差和梯度
        std::vector<float> hidden3Error(hiddenDim3);
        for (int i = 0; i < hiddenDim3; ++i) {
            hidden3Error[i] = 0.0f;
            for (int j = 0; j < hiddenDim4; ++j) {
                hidden3Error[i] += network[3][i * hiddenDim4 + j] * hidden4Error[j];
            }
            hidden3Error[i] *= (hidden3[i] > 0.0f) ? 1.0f : 0.0f;
            outBiasGrads[2][i] += hidden3Error[i];
            for (int j = 0; j < hiddenDim2; ++j) {
                weightGrads[2][j * hiddenDim3 + i] += hidden2[j] * hidden3Error[i];
            }
        }
        
        // 第二层误差和梯度
        std::vector<float> hidden2Error(hiddenDim2);
        for (int i = 0; i < hiddenDim2; ++i) {
            hidden2Error[i] = 0.0f;
            for (int j = 0; j < hiddenDim3; ++j) {
                hidden2Error[i] += network[2][i * hiddenDim3 + j] * hidden3Error[j];
            }
            hidden2Error[i] *= (hidden2[i] > 0.0f) ? 1.0f : 0.0f;
            outBiasGrads[1][i] += hidden2Error[i];
            for (int j = 0; j < hiddenDim1; ++j) {
                weightGrads[1][j * hiddenDim2 + i] += hidden1[j] * hidden2Error[i];
            }
        }
        
        // 第一层误差和梯度
        std::vector<float> hidden1Error(hiddenDim1);
        for (int i = 0; i < hiddenDim1; ++i) {
            hidden1Error[i] = 0.0f;
            for (int j = 0; j < hiddenDim2; ++j) {
                hidden1Error[i] += network[1][i * hiddenDim2 + j] * hidden2Error[j];
            }
            hidden1Error[i] *= (hidden1[i] > 0.0f) ? 1.0f : 0.0f;
            outBiasGrads[0][i] += hidden1Error[i];
            for (int j = 0; j < inputDim; ++j) {
                weightGrads[0][j * hiddenDim1 + i] += state[j] * hidden1Error[i];
            }
        }
    }
    
    // 平均梯度
    float scale = 1.0f / states.size();
    for (auto& grad : weightGrads) {
        for (auto& g : grad) g *= scale;
    }
    for (auto& grad : outBiasGrads) {
        for (auto& g : grad) g *= scale;
    }
    
    return weightGrads;
}

// Adam优化器更新 - 支持6层深度网络
void SLTrainer::BehaviorCloningAgent::adamUpdate(
    const std::vector<std::vector<float>>& gradients,
    const std::vector<std::vector<float>>& biasGradients,
    float lr, int step) {
    
    float beta1 = 0.9f;
    float beta2 = 0.999f;
    float epsilon = 1e-8f;
    
    // 支持6层网络的参数更新
    for (size_t layer = 0; layer < 6; ++layer) {
        // 更新权重
        for (size_t i = 0; i < network[layer].size(); ++i) {
            adamM[layer][i] = beta1 * adamM[layer][i] + (1.0f - beta1) * gradients[layer][i];
            adamV[layer][i] = beta2 * adamV[layer][i] + (1.0f - beta2) * gradients[layer][i] * gradients[layer][i];
            
            float mCorrected = adamM[layer][i] / (1.0f - std::pow(beta1, step + 1));
            float vCorrected = adamV[layer][i] / (1.0f - std::pow(beta2, step + 1));
            
            network[layer][i] -= lr * mCorrected / (std::sqrt(vCorrected) + epsilon);
        }
        
        // 更新偏置
        for (size_t i = 0; i < biases[layer].size(); ++i) {
            adamMB[layer][i] = beta1 * adamMB[layer][i] + (1.0f - beta1) * biasGradients[layer][i];
            adamVB[layer][i] = beta2 * adamVB[layer][i] + (1.0f - beta2) * biasGradients[layer][i] * biasGradients[layer][i];
            
            float mCorrected = adamMB[layer][i] / (1.0f - std::pow(beta1, step + 1));
            float vCorrected = adamVB[layer][i] / (1.0f - std::pow(beta2, step + 1));
            
            biases[layer][i] -= lr * mCorrected / (std::sqrt(vCorrected) + epsilon);
        }
    }
}

// 获取模型参数
std::vector<float> SLTrainer::BehaviorCloningAgent::getParameters() const {
    std::vector<float> params;
    for (const auto& layer : network) {
        params.insert(params.end(), layer.begin(), layer.end());
    }
    for (const auto& bias : biases) {
        params.insert(params.end(), bias.begin(), bias.end());
    }
    return params;
}

// 设置模型参数
void SLTrainer::BehaviorCloningAgent::setParameters(const std::vector<float>& params) {
    size_t offset = 0;
    for (auto& layer : network) {
        std::copy(params.begin() + offset, params.begin() + offset + layer.size(), layer.begin());
        offset += layer.size();
    }
    for (auto& bias : biases) {
        std::copy(params.begin() + offset, params.begin() + offset + bias.size(), bias.begin());
        offset += bias.size();
    }
}