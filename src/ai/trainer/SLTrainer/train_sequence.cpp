#include "SequenceTrainer.h"
#include "TrainingManager.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <ctime>

int main() {
    std::cout << "Sequence Learning Training Module" << std::endl;
    
    // 序列训练配置（针对时序决策优化）
    SequenceTrainer::SequenceTrainingConfig config;
    config.epochs = 100;            // 更多训练轮数，序列学习需要更多时间
    config.batchSize = 32;        // 较小批次，序列学习对内存敏感
    config.learningRate = 0.001f;   // 较低学习率，保证训练稳定性
    config.validationSplit = 0.15f; // 减少验证集比例，保留更多训练数据
    config.earlyStoppingPatience = 20; // 更多耐心值，序列学习收敛较慢
    config.sequenceLength = 150;    // 150帧序列长度
    config.lstmHiddenSize1 = 256;   // 第一层LSTM隐藏层
    config.lstmHiddenSize2 = 128;   // 第二层LSTM隐藏层
    config.denseHiddenSize = 64;    // 全连接隐藏层
    config.dropoutRate = 0.2f;      // Dropout防止过拟合
    config.useLayerNorm = true;     // 使用层归一化
    
    // 创建序列训练器
    SequenceTrainer trainer(config);
    
    // 检查是否存在已有序列模型
    std::string sequenceModelPath = "d:/steam/steamapps/common/Noita/mods/NoitaCoreAI/aiDev/models/sequence_models/best_sequence_model.nn";
    if (std::filesystem::exists(sequenceModelPath)) {
        try {
            trainer.loadSequenceModel(sequenceModelPath);
            std::cout << "Loaded existing sequence model for continued training" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Warning: Failed to load existing sequence model, starting fresh training: " << e.what() << std::endl;
        }
    } else {
        std::cout << "No existing sequence model found, starting fresh training" << std::endl;
    }
    
    // 加载训练数据（分批处理避免内存溢出）
    std::vector<SimpleML::EpisodeData> trainingData;
    const int BATCH_SIZE = 10000; // 每批处理10000个样本
    int totalSamples = 0;
    
    // 从CSV文件读取数据
    std::ifstream file("d:/steam/steamapps/common/Noita/mods/NoitaCoreAI/aiDev/data/sequence_data/training_dataset.csv");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open training data file!" << std::endl;
        return 1;
    }
    
    std::string line;
    
    // 跳过表头
    std::getline(file, line);
    
    std::cout << "Starting to load training data in batches..." << std::endl;
    
    // 分批读取数据
    while (!file.eof()) {
        std::vector<std::vector<float>> batchData;
        
        // 读取一批数据
        for (int i = 0; i < BATCH_SIZE && std::getline(file, line); ++i) {
            std::stringstream ss(line);
            std::string value;
            std::vector<float> row;
            
            while (std::getline(ss, value, ',')) {
                try {
                    row.push_back(std::stof(value));
                } catch (const std::exception& e) {
                    std::cerr << "Warning: Invalid data format in line: " << line << std::endl;
                    continue;
                }
            }
            
            if (row.size() >= 132) {
                batchData.push_back(row);
            }
        }
        
        if (batchData.empty()) {
            continue;
        }
        
        // 为这批数据创建回合数据
        SimpleML::EpisodeData episode;
        for (const auto& row : batchData) {
            SimpleML::TrainingData sample;
            
            // 前130列是状态数据
            sample.state.resize(130);
            for (int i = 0; i < 130; ++i) {
                sample.state[i] = row[i];
            }
            
            // 接下来的2列是动作数据
            sample.action.resize(2);
            sample.action[0] = row[130]; // action_x
            sample.action[1] = row[131]; // use_energy
            
            sample.reward = 0.0f;
            sample.done = false;
            
            episode.states.push_back(sample);
        }
        
        if (!episode.states.empty()) {
            episode.states.back().done = true;
            trainingData.push_back(episode);
        }
        
        totalSamples += batchData.size();
        std::cout << "Processed batch: " << totalSamples << " samples loaded" << std::endl;
        
        // 释放这批数据的内存
        batchData.clear();
        batchData.shrink_to_fit();
    }
    
    file.close();
    
    std::cout << "Completed loading " << totalSamples << " training samples" << std::endl;
    
    if (trainingData.empty()) {
        std::cerr << "Error: No training data found!" << std::endl;
        return 1;
    }
    
    // 创建序列训练数据（分批处理避免内存溢出）
    std::vector<SequenceML::SequenceTrainingData> sequenceTrainingData;
    const int EPISODE_BATCH_SIZE = 5; // 每批处理5个episode
    
    std::cout << "Starting to create sequences from episodes in batches..." << std::endl;
    
    for (size_t i = 0; i < trainingData.size(); i += EPISODE_BATCH_SIZE) {
        size_t end = std::min(i + EPISODE_BATCH_SIZE, trainingData.size());
        std::vector<SimpleML::EpisodeData> episodeBatch(trainingData.begin() + i, trainingData.begin() + end);
        
        std::vector<SequenceML::SequenceTrainingData> batchSequences;
        trainer.createSequencesFromEpisodes(episodeBatch, batchSequences);
        
        // 将这批序列添加到总序列中
        sequenceTrainingData.insert(sequenceTrainingData.end(), batchSequences.begin(), batchSequences.end());
        
        std::cout << "Processed episodes " << (i+1) << "-" << end << ", total sequences: " << sequenceTrainingData.size() << std::endl;
        
        // 释放这批序列的内存
        batchSequences.clear();
        batchSequences.shrink_to_fit();
        episodeBatch.clear();
        episodeBatch.shrink_to_fit();
    }
    
    std::cout << "Completed creating " << sequenceTrainingData.size() << " sequence training samples" << std::endl;
    
    if (sequenceTrainingData.empty()) {
        std::cerr << "Error: No sequence training data created!" << std::endl;
        return 1;
    }
    
    // 创建序列回合数据（避免大规模内存复制）
    std::cout << "[DEBUG] Creating sequence episodes vector..." << std::endl;
    std::vector<SequenceML::SequenceEpisodeData> sequenceEpisodes;
    
    std::cout << "[DEBUG] Creating and reserving space for episode..." << std::endl;
    sequenceEpisodes.reserve(1);
    sequenceEpisodes.emplace_back();
    
    std::cout << "[DEBUG] Moving sequences to episode (" << sequenceTrainingData.size() << " sequences)..." << std::endl;
    sequenceEpisodes[0].sequences = std::move(sequenceTrainingData);
    std::cout << "[DEBUG] Sequence move completed" << std::endl;
    
    std::cout << "[DEBUG] Sequence episodes vector created with " << sequenceEpisodes.size() << " episodes" << std::endl;
    
    // 开始序列训练
    std::cout << "Starting sequence training..." << std::endl;
    std::cout << "[DEBUG] About to call trainFromSequences..." << std::endl;
    trainer.trainFromSequences(sequenceEpisodes);
    std::cout << "[DEBUG] trainFromSequences call completed" << std::endl;
    
    // 获取训练统计信息
    auto stats = trainer.getSequenceTrainingStats();
    std::cout << "Training completed!" << std::endl;
    std::cout << "Final training loss: " << stats.trainingLoss << std::endl;
    std::cout << "Final validation loss: " << stats.validationLoss << std::endl;
    std::cout << "Best epoch: " << stats.bestEpoch << std::endl;
    std::cout << "Best validation loss: " << stats.bestValidationLoss << std::endl;
    
    // 保存最终序列模型
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_time);
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &tm);
    
    std::string newModelPath = "d:/steam/steamapps/common/Noita/mods/NoitaCoreAI/aiDev/models/sequence_models/sequence_model_" + std::string(timestamp) + ".nn";
    trainer.saveSequenceModel(newModelPath);
    std::cout << "Sequence model saved to: " << newModelPath << std::endl;
    
    // 同时保存为默认序列模型
    trainer.saveSequenceModel(sequenceModelPath);
    std::cout << "Sequence model also saved to: " << sequenceModelPath << std::endl;
    
    return 0;
}