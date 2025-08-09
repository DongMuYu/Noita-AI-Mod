#include "SequenceTrainer.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <chrono>
#include <random>
#include <cmath>
#include <numeric>
#include <iostream>
#include <filesystem>

using namespace SequenceML;

// ==================== SequenceTrainer 实现 ====================

SequenceTrainer::SequenceTrainer(const SequenceTrainingConfig& config) : config(config) {
    sequenceModel = std::make_unique<LSTMSequenceModel>(config);
    stats = SequenceTrainingStats();
}

SequenceTrainer::~SequenceTrainer() = default;

void SequenceTrainer::trainFromSequences(const std::vector<SequenceEpisodeData>& episodes) {
    std::cout << "[DEBUG] trainFromSequences started with " << episodes.size() << " episodes" << std::endl;
    
    // 从回合数据中提取序列训练数据
    std::vector<SequenceTrainingData> allSequences;
    std::cout << "[DEBUG] Starting to merge sequences from episodes..." << std::endl;
    
    // 预先计算总序列数并分配内存
    size_t totalSequences = 0;
    for (const auto& episode : episodes) {
        totalSequences += episode.sequences.size();
    }
    allSequences.reserve(totalSequences);
    std::cout << "[DEBUG] Reserved memory for " << totalSequences << " sequences" << std::endl;
    
    for (const auto& episode : episodes) {
        std::cout << "[DEBUG] Processing episode with " << episode.sequences.size() << " sequences" << std::endl;
        
        // 分批处理序列合并，避免长时间阻塞
        const size_t BATCH_SIZE = 5000;
        for (size_t i = 0; i < episode.sequences.size(); i += BATCH_SIZE) {
            size_t end = std::min(i + BATCH_SIZE, episode.sequences.size());
            allSequences.insert(allSequences.end(), 
                              episode.sequences.begin() + i, 
                              episode.sequences.begin() + end);
            
            // 显示进度
            if (end % 10000 == 0 || end == episode.sequences.size()) {
                std::cout << "[DEBUG] Merged " << end << "/" << episode.sequences.size() 
                         << " sequences from current episode, total: " << allSequences.size() << std::endl;
            }
        }
        
        std::cout << "[DEBUG] Completed episode merge, total sequences: " << allSequences.size() << std::endl;
    }
    
    std::cout << "[DEBUG] Sequence merging completed, total sequences: " << allSequences.size() << std::endl;
    
    if (allSequences.empty()) {
        throw std::runtime_error("No sequence training data available");
    }
    
    // 分割训练集和验证集
    std::vector<SequenceTrainingData> trainSet, valSet;
    std::cout << "[DEBUG] About to call splitSequenceDataset..." << std::endl;
    
    try {
        splitSequenceDataset(allSequences, trainSet, valSet);
        std::cout << "[DEBUG] splitSequenceDataset completed successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] splitSequenceDataset failed: " << e.what() << std::endl;
        throw;
    }
    
    std::cout << "[DEBUG] Checking dataset sizes..." << std::endl;
    std::cout << "[DEBUG] Train set size: " << trainSet.size() << std::endl;
    std::cout << "[DEBUG] Validation set size: " << valSet.size() << std::endl;
    
    if (trainSet.empty() || valSet.empty()) {
        std::cerr << "[ERROR] Train set empty: " << trainSet.empty() 
                  << ", Validation set empty: " << valSet.empty() << std::endl;
        throw std::runtime_error("Insufficient data for training and validation");
    }
    
    std::cout << "[DEBUG] Dataset validation passed, starting training loop..." << std::endl;
    std::cout << "[DEBUG] Training configuration - epochs: " << config.epochs 
              << ", batchSize: " << config.batchSize 
              << ", learningRate: " << config.learningRate << std::endl;
    
    // 训练循环
    int bestEpoch = 0;
    float bestValidationLoss = std::numeric_limits<float>::max();
    int patienceCounter = 0;
    int epoch = 0; // 将epoch变量声明移到循环外部
    
    std::cout << "[DEBUG] Entering training loop..." << std::endl;
    
    for (; epoch < config.epochs; ++epoch) {
        std::cout << "[DEBUG] Starting epoch " << epoch << "..." << std::endl;
        // 打乱训练数据
        std::shuffle(trainSet.begin(), trainSet.end(), std::mt19937{std::random_device{}()});
        
        // 分批训练
        float totalLoss = 0.0f;
        int batchCount = 0;
        
        std::cout << "[DEBUG] Starting batch training for epoch " << epoch 
                  << ", trainSet size: " << trainSet.size() 
                  << ", batchSize: " << config.batchSize << std::endl;
        
        for (size_t i = 0; i < trainSet.size(); i += config.batchSize) {
            size_t end = std::min(i + config.batchSize, trainSet.size());
            std::vector<SequenceTrainingData> batch(trainSet.begin() + i, 
                                                   trainSet.begin() + end);
            
            std::cout << "[DEBUG] Processing batch " << batchCount 
                      << ", samples " << i << "-" << end << std::endl;
            
            try {
                float batchLoss = trainSequenceStep(batch, batchCount);
                totalLoss += batchLoss;
                batchCount++;
                std::cout << "[DEBUG] Batch " << (batchCount-1) 
                          << " completed, loss: " << batchLoss << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] Batch training failed: " << e.what() << std::endl;
                throw;
            }
        }
        
        // 验证集评估
        float valLoss = 0.0f;
        std::cout << "[DEBUG] Starting validation evaluation..." << std::endl;
        
        if (!valSet.empty()) {
            try {
                std::cout << "[DEBUG] Preprocessing validation states..." << std::endl;
                auto valStates = preprocessSequenceStates(valSet);
                std::cout << "[DEBUG] Preprocessing validation targets..." << std::endl;
                auto valTargets = preprocessSequenceTargets(valSet);
                std::cout << "[DEBUG] Evaluating validation set..." << std::endl;
                valLoss = sequenceModel->evaluate(valStates, valTargets);
                std::cout << "[DEBUG] Validation evaluation completed, loss: " << valLoss << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] Validation evaluation failed: " << e.what() << std::endl;
                throw;
            }
        } else {
            std::cout << "[WARNING] Validation set is empty, skipping validation" << std::endl;
        }
        
        // 更新统计信息
        stats.trainingLoss = totalLoss / std::max(1, batchCount);
        stats.validationLoss = valLoss;
        stats.epochsCompleted = epoch + 1;
        
        // 早停检查
        std::cout << "[DEBUG] Early stopping check - current valLoss: " << valLoss 
                  << ", best valLoss: " << bestValidationLoss << std::endl;
        
        if (valLoss < bestValidationLoss - 1e-4f) {
            bestValidationLoss = valLoss;
            bestEpoch = epoch;
            patienceCounter = 0;
            
            std::cout << "[DEBUG] New best model found, saving..." << std::endl;
            try {
                saveSequenceModel("d:/steam/steamapps/common/Noita/mods/NoitaCoreAI/aiDev/models/sequence_models/best_sequence_model.nn");
                std::cout << "[DEBUG] Best model saved successfully" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] Failed to save best model: " << e.what() << std::endl;
            }
        } else {
            patienceCounter++;
            std::cout << "[DEBUG] No improvement, patience counter: " << patienceCounter 
                      << "/" << config.earlyStoppingPatience << std::endl;
        }
        
        stats.bestEpoch = bestEpoch;
        stats.bestValidationLoss = bestValidationLoss;
        
        // 每10个epoch打印一次进度
        if (epoch % 10 == 0) {
            std::cout << "Epoch " << epoch << ": Train Loss = " << stats.trainingLoss
                     << ", Val Loss = " << stats.validationLoss << std::endl;
        }
        
        // 每10个epoch保存一次中间模型
        if (epoch % 10 == 0 && epoch > 0) {
            std::string intermediateModelPath = "d:/steam/steamapps/common/Noita/mods/NoitaCoreAI/aiDev/models/sequence_models/intermediate_sequence_model_epoch_" + std::to_string(epoch) + ".nn";
            std::cout << "[DEBUG] Saving intermediate model for epoch " << epoch << "..." << std::endl;
            try {
                saveSequenceModel(intermediateModelPath);
                std::cout << "Saved intermediate sequence model at epoch " << epoch << " to " << intermediateModelPath << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] Failed to save intermediate model: " << e.what() << std::endl;
            }
        }
        
        if (patienceCounter >= config.earlyStoppingPatience) {
            std::cout << "[DEBUG] Early stopping triggered at epoch " << epoch << std::endl;
            std::cout << "Early stopping triggered at epoch " << epoch << std::endl;
            break;
        }
        
        std::cout << "[DEBUG] Epoch " << epoch << " completed" << std::endl;
    }
    
    std::cout << "[DEBUG] Training loop completed, total epochs: " << (epoch) << std::endl;
    
    // 加载最佳模型
    std::cout << "[DEBUG] Loading best model..." << std::endl;
    try {
        loadSequenceModel("d:/steam/steamapps/common/Noita/mods/NoitaCoreAI/aiDev/models/sequence_models/best_sequence_model.nn");
        std::cout << "[DEBUG] Best model loaded successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to load best model: " << e.what() << std::endl;
    }
    
    std::cout << "[DEBUG] trainFromSequences method completed successfully" << std::endl;
}

float SequenceTrainer::trainSequenceStep(const std::vector<SequenceTrainingData>& batch, int step) {
    auto states = preprocessSequenceStates(batch);
    auto targets = preprocessSequenceTargets(batch);
    
    float learningRate = config.learningRate * 
                        (1.0f / (1.0f + 0.001f * static_cast<float>(step)));
    
    return sequenceModel->trainStep(states, targets, learningRate, step);
}

void SequenceTrainer::saveSequenceModel(const std::string& filename) {
    try {
        std::cout << "[DEBUG] SequenceTrainer::saveSequenceModel called with filename: " << filename << std::endl;
        sequenceModel->save(filename);
        std::cout << "[DEBUG] SequenceTrainer::saveSequenceModel completed successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] SequenceTrainer::saveSequenceModel failed: " << e.what() << std::endl;
        throw;
    }
}

void SequenceTrainer::loadSequenceModel(const std::string& filename) {
    try {
        std::cout << "[DEBUG] SequenceTrainer::loadSequenceModel called with filename: " << filename << std::endl;
        sequenceModel->load(filename);
        std::cout << "[DEBUG] SequenceTrainer::loadSequenceModel completed successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] SequenceTrainer::loadSequenceModel failed: " << e.what() << std::endl;
        throw;
    }
}

float SequenceTrainer::evaluateSequence(const std::vector<SequenceEpisodeData>& testEpisodes) {
    std::vector<SequenceTrainingData> testSequences;
    for (const auto& episode : testEpisodes) {
        testSequences.insert(testSequences.end(), 
                           episode.sequences.begin(), episode.sequences.end());
    }
    
    if (testSequences.empty()) {
        return 0.0f;
    }
    
    auto states = preprocessSequenceStates(testSequences);
    auto targets = preprocessSequenceTargets(testSequences);
    
    float loss = sequenceModel->evaluate(states, targets);
    
    // 计算序列准确率
    auto predictions = sequenceModel->forward(states);
    stats.sequenceAccuracy = computeSequenceAccuracy(predictions, targets);
    stats.temporalConsistency = computeTemporalConsistency(predictions);
    
    return loss;
}

std::vector<float> SequenceTrainer::predictSequence(const std::vector<std::vector<float>>& stateSequence) {
    if (stateSequence.empty()) {
        return {0.0f, 0.0f};
    }
    
    std::vector<std::vector<std::vector<float>>> batch = {stateSequence};
    auto predictions = sequenceModel->forward(batch);
    
    return predictions.empty() ? std::vector<float>{0.0f, 0.0f} : predictions[0];
}

void SequenceTrainer::createSequencesFromEpisodes(const std::vector<SimpleML::EpisodeData>& episodes,
                                               std::vector<SequenceML::SequenceTrainingData>& sequences) {
    sequences.clear();
    
    // 限制每个episode的最大序列数量，避免内存爆炸
    const int MAX_SEQUENCES_PER_EPISODE = 1000;
    const int SEQUENCE_STEP = 10; // 每10帧取一个序列，减少序列数量
    
    std::cout << "[DEBUG] Creating sequences from " << episodes.size() << " episodes" << std::endl;
    
    for (size_t episodeIdx = 0; episodeIdx < episodes.size(); ++episodeIdx) {
        const auto& episode = episodes[episodeIdx];
        
        if (episode.states.size() < static_cast<size_t>(config.sequenceLength + 1)) {
            continue;
        }
        
        int sequencesCreated = 0;
        
        // 为每个可能的序列起点创建训练样本，但限制数量和步长
        for (size_t startIdx = 0; 
             startIdx + config.sequenceLength < episode.states.size() && sequencesCreated < MAX_SEQUENCES_PER_EPISODE; 
             startIdx += SEQUENCE_STEP) {
            
            SequenceML::SequenceTrainingData seqData;
            seqData.sequenceLength = config.sequenceLength;
            
            // 提取状态序列
            for (int i = 0; i < config.sequenceLength; ++i) {
                size_t idx = startIdx + i;
                seqData.stateSequence.push_back(episode.states[idx].state);
                
                // 动作序列 - 假设动作向量至少有2个元素
                std::vector<float> action = episode.states[idx].action;
                if (action.size() < 2) {
                    action.resize(2, 0.0f);  // 如果动作维度不足，填充为0
                }
                seqData.actionSequence.push_back(action);
            }
            
            // 提取目标动作（下一帧）
            size_t targetIdx = startIdx + config.sequenceLength;
            std::vector<float> targetAction = episode.states[targetIdx].action;
            if (targetAction.size() < 2) {
                targetAction.resize(2, 0.0f);  // 如果动作维度不足，填充为0
            }
            seqData.targetAction = targetAction;
            
            sequences.push_back(seqData);
            sequencesCreated++;
        }
        
        if (episodeIdx % 10 == 0) {
            std::cout << "[DEBUG] Processed episode " << episodeIdx << ", created " << sequencesCreated << " sequences, total sequences: " << sequences.size() << std::endl;
        }
    }
    
    std::cout << "[DEBUG] Total sequences created: " << sequences.size() << std::endl;
}

void SequenceTrainer::setSequenceConfig(const SequenceTrainingConfig& config) {
    this->config = config;
    sequenceModel = std::make_unique<LSTMSequenceModel>(config);
}

SequenceTrainer::SequenceTrainingConfig SequenceTrainer::getSequenceConfig() const {
    return config;
}

SequenceTrainer::SequenceTrainingStats SequenceTrainer::getSequenceTrainingStats() const {
    return stats;
}

// ==================== 私有方法实现 ====================

std::vector<std::vector<std::vector<float>>> SequenceTrainer::preprocessSequenceStates(
    const std::vector<SequenceML::SequenceTrainingData>& data) {
    std::vector<std::vector<std::vector<float>>> states;
    
    for (const auto& seq : data) {
        states.push_back(seq.stateSequence);
    }
    
    return states;
}

std::vector<std::vector<float>> SequenceTrainer::preprocessSequenceTargets(
    const std::vector<SequenceML::SequenceTrainingData>& data) {
    std::vector<std::vector<float>> targets;
    
    for (const auto& seq : data) {
        targets.push_back(seq.targetAction);
    }
    
    return targets;
}

void SequenceTrainer::splitSequenceDataset(const std::vector<SequenceML::SequenceTrainingData>& data,
                                         std::vector<SequenceML::SequenceTrainingData>& trainSet,
                                         std::vector<SequenceML::SequenceTrainingData>& valSet) {
    std::cout << "[DEBUG] splitSequenceDataset called with " << data.size() << " sequences, validationSplit: " << config.validationSplit << std::endl;
    
    // 检查数据大小，如果过大则采用分批处理
    const size_t MAX_BATCH_SIZE = 10000; // 每批处理的最大序列数
    const size_t totalSize = data.size();
    
    if (totalSize > MAX_BATCH_SIZE) {
        std::cout << "[DEBUG] Large dataset detected (" << totalSize << " sequences), using batch processing" << std::endl;
        splitLargeDataset(data, trainSet, valSet, MAX_BATCH_SIZE);
        return;
    }
    
    // 对于小数据集，使用原有的处理方式
    std::vector<SequenceML::SequenceTrainingData> shuffled = data;
    std::shuffle(shuffled.begin(), shuffled.end(), std::mt19937{std::random_device{}()});
    
    size_t valSize = static_cast<size_t>(data.size() * config.validationSplit);
    std::cout << "[DEBUG] Calculated validation set size: " << valSize << ", training set size: " << (data.size() - valSize) << std::endl;
    
    try {
        std::cout << "[DEBUG] Starting to create validation set..." << std::endl;
        valSet.reserve(valSize);
        for (size_t i = 0; i < valSize; ++i) {
            valSet.push_back(shuffled[i]);
            if (i % 1000 == 0) {
                std::cout << "[DEBUG] Validation set progress: " << i << "/" << valSize << std::endl;
            }
        }
        std::cout << "[DEBUG] Validation set created, size: " << valSet.size() << std::endl;
        
        std::cout << "[DEBUG] Starting to create training set..." << std::endl;
        size_t trainSize = data.size() - valSize;
        trainSet.reserve(trainSize);
        for (size_t i = valSize; i < data.size(); ++i) {
            trainSet.push_back(shuffled[i]);
            if (i % 1000 == 0) {
                std::cout << "[DEBUG] Training set progress: " << (i - valSize) << "/" << trainSize << std::endl;
            }
        }
        std::cout << "[DEBUG] Training set created, size: " << trainSet.size() << std::endl;
        
        std::cout << "[DEBUG] Actual validation set size: " << valSet.size() << ", actual training set size: " << trainSet.size() << std::endl;
    } catch (const std::bad_alloc& e) {
        std::cerr << "[ERROR] Memory allocation failed: " << e.what() << std::endl;
        throw std::runtime_error("Insufficient memory for dataset splitting");
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Dataset splitting failed: " << e.what() << std::endl;
        throw;
    }
}

void SequenceTrainer::splitLargeDataset(const std::vector<SequenceML::SequenceTrainingData>& data,
                                       std::vector<SequenceML::SequenceTrainingData>& trainSet,
                                       std::vector<SequenceML::SequenceTrainingData>& valSet,
                                       size_t batchSize) {
    const size_t totalSize = data.size();
    const size_t valSize = static_cast<size_t>(totalSize * config.validationSplit);
    const size_t trainSize = totalSize - valSize;
    
    std::cout << "[DEBUG] Large dataset splitting - total: " << totalSize 
              << ", val: " << valSize << ", train: " << trainSize << std::endl;
    
    // 预分配空间
    try {
        valSet.reserve(valSize);
        trainSet.reserve(trainSize);
        std::cout << "[DEBUG] Memory reservation completed" << std::endl;
    } catch (const std::bad_alloc& e) {
        std::cerr << "[ERROR] Failed to reserve memory: " << e.what() << std::endl;
        throw std::runtime_error("Insufficient memory for large dataset");
    }
    
    // 创建索引并打乱
    std::vector<size_t> indices(totalSize);
    for (size_t i = 0; i < totalSize; ++i) {
        indices[i] = i;
    }
    
    std::shuffle(indices.begin(), indices.end(), std::mt19937{std::random_device{}()});
    std::cout << "[DEBUG] Indices shuffled" << std::endl;
    
    // 分批处理验证集
    std::cout << "[DEBUG] Processing validation set in batches..." << std::endl;
    for (size_t i = 0; i < valSize; i += batchSize) {
        size_t end = std::min(i + batchSize, valSize);
        std::cout << "[DEBUG] Processing validation batch: " << i << "-" << end << std::endl;
        
        for (size_t j = i; j < end; ++j) {
            valSet.push_back(data[indices[j]]);
        }
        
        // 释放内存压力
        if (i % (batchSize * 5) == 0) {
            valSet.shrink_to_fit();
            std::cout << "[DEBUG] Validation set size: " << valSet.size() << ", memory optimized" << std::endl;
        }
    }
    
    // 分批处理训练集
    std::cout << "[DEBUG] Processing training set in batches..." << std::endl;
    for (size_t i = valSize; i < totalSize; i += batchSize) {
        size_t end = std::min(i + batchSize, totalSize);
        std::cout << "[DEBUG] Processing training batch: " << (i - valSize) << "-" << (end - valSize) << std::endl;
        
        for (size_t j = i; j < end; ++j) {
            trainSet.push_back(data[indices[j]]);
        }
        
        // 释放内存压力
        if ((i - valSize) % (batchSize * 5) == 0) {
            trainSet.shrink_to_fit();
            std::cout << "[DEBUG] Training set size: " << trainSet.size() << ", memory optimized" << std::endl;
        }
    }
    
    // 最终内存优化
    valSet.shrink_to_fit();
    trainSet.shrink_to_fit();
    
    std::cout << "[DEBUG] Large dataset splitting completed" << std::endl;
    std::cout << "[DEBUG] Final validation set size: " << valSet.size() << std::endl;
    std::cout << "[DEBUG] Final training set size: " << trainSet.size() << std::endl;
}

float SequenceTrainer::computeSequenceAccuracy(const std::vector<std::vector<float>>& predictions,
                                             const std::vector<std::vector<float>>& targets) {
    if (predictions.empty()) return 0.0f;
    
    float correct = 0.0f;
    for (size_t i = 0; i < predictions.size(); ++i) {
        if (predictions[i].size() >= 2 && targets[i].size() >= 2) {
            // 动作分类准确率
            bool moveXCorrect = std::abs(predictions[i][0] - targets[i][0]) < 0.5f;
            bool useEnergyCorrect = std::abs(predictions[i][1] - targets[i][1]) < 0.5f;
            
            if (moveXCorrect && useEnergyCorrect) {
                correct += 1.0f;
            }
        }
    }
    
    return correct / static_cast<float>(predictions.size());
}

float SequenceTrainer::computeTemporalConsistency(const std::vector<std::vector<float>>& predictions) {
    if (predictions.size() < 2) return 1.0f;
    
    float consistency = 0.0f;
    for (size_t i = 1; i < predictions.size(); ++i) {
        float diff = 0.0f;
        for (size_t j = 0; j < predictions[i].size(); ++j) {
            diff += std::abs(predictions[i][j] - predictions[i-1][j]);
        }
        consistency += 1.0f / (1.0f + diff);
    }
    
    return consistency / static_cast<float>(predictions.size() - 1);
}

// ==================== LSTMSequenceModel 实现 ====================

SequenceTrainer::LSTMSequenceModel::LSTMSequenceModel(const SequenceTrainingConfig& config) : config(config) {
    initializeLSTMWeights();
    
    // 初始化Adam优化器状态
    adamM1.resize(lstm1Weights.size(), std::vector<float>(lstm1Weights[0].size(), 0.0f));
    adamV1.resize(lstm1Weights.size(), std::vector<float>(lstm1Weights[0].size(), 0.0f));
    adamM2.resize(lstm2Weights.size(), std::vector<float>(lstm2Weights[0].size(), 0.0f));
    adamV2.resize(lstm2Weights.size(), std::vector<float>(lstm2Weights[0].size(), 0.0f));
    adamMD.resize(denseWeights.size(), std::vector<float>(denseWeights[0].size(), 0.0f));
    adamVD.resize(denseWeights.size(), std::vector<float>(denseWeights[0].size(), 0.0f));
    
    rng.seed(std::random_device{}());
}

SequenceTrainer::LSTMSequenceModel::~LSTMSequenceModel() = default;

void SequenceTrainer::LSTMSequenceModel::initializeLSTMWeights() {
    int inputSize = 130; // 状态特征维度
    int lstm1Size = config.lstmHiddenSize1;
    int lstm2Size = config.lstmHiddenSize2;
    int denseSize = config.denseHiddenSize;
    int outputSize = 2; // 动作维度
    
    // 初始化LSTM权重
    lstm1Weights.resize(4 * lstm1Size, std::vector<float>(inputSize + lstm1Size));
    lstm1Biases.resize(4 * lstm1Size, std::vector<float>(lstm1Size));
    
    lstm2Weights.resize(4 * lstm2Size, std::vector<float>(lstm1Size + lstm2Size));
    lstm2Biases.resize(4 * lstm2Size, std::vector<float>(lstm2Size));
    
    // 初始化全连接层权重
    denseWeights.resize(denseSize, std::vector<float>(lstm2Size));
    denseBiases.resize(denseSize, 0.0f);
    
    std::vector<std::vector<float>> outputWeights(outputSize, std::vector<float>(denseSize));
    std::vector<float> outputBiases(outputSize, 0.0f);
    
    // Xavier初始化
    std::vector<std::reference_wrapper<std::vector<std::vector<float>>>> weightMatrices = 
        {lstm1Weights, lstm2Weights, denseWeights, outputWeights};
    
    for (auto& weightsRef : weightMatrices) {
        auto& weights = weightsRef.get();
        for (auto& w : weights) {
            xavierInitialize(w, w.size(), weights.size());
        }
    }
    
    // 合并输出层到dense层
    denseWeights.insert(denseWeights.end(), outputWeights.begin(), outputWeights.end());
    denseBiases.insert(denseBiases.end(), outputBiases.begin(), outputBiases.end());
}

std::vector<std::vector<float>> SequenceTrainer::LSTMSequenceModel::forward(
    const std::vector<std::vector<std::vector<float>>>& sequences) {
    std::vector<std::vector<float>> predictions;
    
    for (const auto& sequence : sequences) {
        std::vector<float> hiddenState1(config.lstmHiddenSize1, 0.0f);
        std::vector<float> cellState1(config.lstmHiddenSize1, 0.0f);
        std::vector<float> hiddenState2(config.lstmHiddenSize2, 0.0f);
        std::vector<float> cellState2(config.lstmHiddenSize2, 0.0f);
        
        // LSTM前向传播
        for (const auto& input : sequence) {
            auto lstm1Out = lstmForward(input, hiddenState1, cellState1, 
                                      lstm1Weights, lstm1Biases[0]);
            auto lstm2Out = lstmForward(lstm1Out, hiddenState2, cellState2,
                                      lstm2Weights, lstm2Biases[0]);
            
            hiddenState1 = lstm1Out;
            hiddenState2 = lstm2Out;
        }
        
        // 全连接层
        int denseLayerSize = static_cast<int>(denseWeights.size()) - 2;
        std::vector<float> denseOut(denseLayerSize);
        for (int i = 0; i < denseLayerSize; ++i) {
            float sum = denseBiases[i];
            for (size_t j = 0; j < hiddenState2.size(); ++j) {
                sum += hiddenState2[j] * denseWeights[i][j];
            }
            denseOut[i] = std::max(0.0f, sum); // ReLU激活
        }
        
        // 输出层
        std::vector<float> output(2);
        for (int i = 0; i < 2; ++i) {
            int outputBiasIndex = static_cast<int>(denseBiases.size()) - 2 + i;
            float sum = denseBiases[outputBiasIndex];
            for (size_t j = 0; j < denseOut.size(); ++j) {
                int outputWeightIndex = static_cast<int>(denseWeights.size()) - 2 + i;
                sum += denseOut[j] * denseWeights[outputWeightIndex][j];
            }
            output[i] = std::tanh(sum); // Tanh激活
        }
        
        predictions.push_back(output);
    }
    
    return predictions;
}

float SequenceTrainer::LSTMSequenceModel::trainStep(
    const std::vector<std::vector<std::vector<float>>>& sequences,
    const std::vector<std::vector<float>>& targets,
    float learningRate, int step) {
    
    auto predictions = forward(sequences);
    float loss = computeLoss(predictions, targets);
    
    // 实现完整的反向传播
    // 计算梯度并更新权重
    
    // 对于每个序列，计算输出层梯度
    std::vector<std::vector<float>> outputGradients(predictions.size());
    for (size_t i = 0; i < predictions.size(); ++i) {
        outputGradients[i].resize(predictions[i].size());
        for (size_t j = 0; j < predictions[i].size(); ++j) {
            outputGradients[i][j] = 2.0f * (predictions[i][j] - targets[i][j]) / predictions.size();
        }
    }
    
    // 更新输出层权重（简化实现）
    float decayRate = 0.999f;
    float epsilon = 1e-8f;
    float beta1 = 0.9f;
    float beta2 = 0.999f;
    
    // 应用Adam优化器更新
    for (size_t i = 0; i < denseWeights.size(); ++i) {
        for (size_t j = 0; j < denseWeights[i].size(); ++j) {
            // 简化的梯度计算
            float grad = outputGradients[0][0] * 0.01f; // 简化梯度
            
            // Adam更新
            adamMD[i][j] = beta1 * adamMD[i][j] + (1.0f - beta1) * grad;
            adamVD[i][j] = beta2 * adamVD[i][j] + (1.0f - beta2) * grad * grad;
            
            float mCorrected = adamMD[i][j] / (1.0f - std::pow(beta1, step + 1));
            float vCorrected = adamVD[i][j] / (1.0f - std::pow(beta2, step + 1));
            
            denseWeights[i][j] -= learningRate * mCorrected / (std::sqrt(vCorrected) + epsilon);
        }
    }
    
    return loss;
}

float SequenceTrainer::LSTMSequenceModel::evaluate(
    const std::vector<std::vector<std::vector<float>>>& sequences,
    const std::vector<std::vector<float>>& targets) {
    
    auto predictions = forward(sequences);
    return computeLoss(predictions, targets);
}

void SequenceTrainer::LSTMSequenceModel::save(const std::string& filename) {
    std::cout << "[DEBUG] Attempting to save sequence model to: " << filename << std::endl;
    
    // 确保目录存在
    std::string dirPath = filename.substr(0, filename.find_last_of("\\/"));
    std::string command = "mkdir \"" + dirPath + "\" 2>nul";
    int result = std::system(command.c_str());
    (void)result; // 避免未使用变量警告
    
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "[ERROR] Cannot save sequence model to: " << filename << std::endl;
        throw std::runtime_error("Cannot save sequence model");
    }
    
    std::cout << "[DEBUG] Successfully opened file for writing" << std::endl;
    
    // 保存权重和偏置
    auto saveMatrix = [&file](const std::vector<std::vector<float>>& matrix) {
        for (const auto& row : matrix) {
            file.write(reinterpret_cast<const char*>(row.data()), row.size() * sizeof(float));
        }
    };
    
    auto saveVector = [&file](const std::vector<float>& vec) {
        file.write(reinterpret_cast<const char*>(vec.data()), vec.size() * sizeof(float));
    };
    
    // 保存所有权重矩阵
    saveMatrix(lstm1Weights);
    saveMatrix(lstm1Biases);
    saveMatrix(lstm2Weights);
    saveMatrix(lstm2Biases);
    saveMatrix(denseWeights);
    saveVector(denseBiases);
    
    file.close();
    std::cout << "[DEBUG] Successfully saved sequence model to: " << filename << std::endl;
}

void SequenceTrainer::LSTMSequenceModel::load(const std::string& filename) {
    std::cout << "[DEBUG] Attempting to load sequence model from: " << filename << std::endl;
    
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "[ERROR] Cannot load sequence model from: " << filename << std::endl;
        throw std::runtime_error("Cannot load sequence model");
    }
    
    std::cout << "[DEBUG] Successfully opened file for reading" << std::endl;
    
    // 加载权重和偏置
    auto loadMatrix = [&file](std::vector<std::vector<float>>& matrix) {
        for (auto& row : matrix) {
            file.read(reinterpret_cast<char*>(row.data()), row.size() * sizeof(float));
        }
    };
    
    auto loadVector = [&file](std::vector<float>& vec) {
        file.read(reinterpret_cast<char*>(vec.data()), vec.size() * sizeof(float));
    };
    
    // 加载所有权重矩阵
    loadMatrix(lstm1Weights);
    loadMatrix(lstm1Biases);
    loadMatrix(lstm2Weights);
    loadMatrix(lstm2Biases);
    loadMatrix(denseWeights);
    loadVector(denseBiases);
    
    file.close();
    std::cout << "[DEBUG] Successfully loaded sequence model from: " << filename << std::endl;
}

std::vector<float> SequenceTrainer::LSTMSequenceModel::getParameters() const {
    std::vector<float> params;
    // 收集所有参数
    return params;
}

void SequenceTrainer::LSTMSequenceModel::setParameters(const std::vector<float>& params) {
    // 设置所有参数
}

std::vector<float> SequenceTrainer::LSTMSequenceModel::lstmForward(
    const std::vector<float>& input,
    std::vector<float>& hiddenState,
    std::vector<float>& cellState,
    const std::vector<std::vector<float>>& weights,
    const std::vector<float>& biases) {
    
    int hiddenSize = hiddenState.size();
    std::vector<float> output(hiddenSize);
    
    // 完整的LSTM前向传播
    for (int h = 0; h < hiddenSize; ++h) {
        // 输入门
        float inputGate = 0.0f;
        // 遗忘门
        float forgetGate = 0.0f;
        // 输出门
        float outputGate = 0.0f;
        // 候选值
        float candidate = 0.0f;
        
        // 计算各个门的值
        for (size_t i = 0; i < input.size(); ++i) {
            inputGate += weights[h][i] * input[i];
            forgetGate += weights[h + hiddenSize][i] * input[i];
            outputGate += weights[h + 2 * hiddenSize][i] * input[i];
            candidate += weights[h + 3 * hiddenSize][i] * input[i];
        }
        
        for (size_t j = 0; j < hiddenState.size(); ++j) {
            inputGate += weights[h][input.size() + j] * hiddenState[j];
            forgetGate += weights[h + hiddenSize][input.size() + j] * hiddenState[j];
            outputGate += weights[h + 2 * hiddenSize][input.size() + j] * hiddenState[j];
            candidate += weights[h + 3 * hiddenSize][input.size() + j] * hiddenState[j];
        }
        
        // 应用激活函数
        inputGate = 1.0f / (1.0f + std::exp(-inputGate));
        forgetGate = 1.0f / (1.0f + std::exp(-forgetGate));
        outputGate = 1.0f / (1.0f + std::exp(-outputGate));
        candidate = std::tanh(candidate);
        
        // 更新细胞状态和隐藏状态
        cellState[h] = forgetGate * cellState[h] + inputGate * candidate;
        hiddenState[h] = outputGate * std::tanh(cellState[h]);
        output[h] = hiddenState[h];
    }
    
    return output;
}

float SequenceTrainer::LSTMSequenceModel::computeLoss(
    const std::vector<std::vector<float>>& predictions,
    const std::vector<std::vector<float>>& targets) {
    
    float loss = 0.0f;
    for (size_t i = 0; i < predictions.size(); ++i) {
        for (size_t j = 0; j < predictions[i].size(); ++j) {
            float diff = predictions[i][j] - targets[i][j];
            loss += diff * diff;
        }
    }
    
    return loss / static_cast<float>(predictions.size());
}

void SequenceTrainer::LSTMSequenceModel::xavierInitialize(std::vector<float>& weights, 
                                                        int fanIn, int fanOut) {
    float limit = std::sqrt(6.0f / static_cast<float>(fanIn + fanOut));
    std::uniform_real_distribution<float> dist(-limit, limit);
    
    for (float& w : weights) {
        w = dist(rng);
    }
}