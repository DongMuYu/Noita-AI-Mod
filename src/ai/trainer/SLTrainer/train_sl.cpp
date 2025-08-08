#include "SLTrainer.h"
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
    std::cout << "Supervised Learning Training Module" << std::endl;
    
    // 优化后的训练配置（改进参数以提高训练效率）
    SLTrainer::TrainingConfig config;
    config.epochs = 50;            // 减少训练轮数，配合早停机制
    config.batchSize = 256;        // 增大批次大小以提高训练效率
    config.learningRate = 0.002f;  // 提高学习率加快收敛
    config.validationSplit = 0.2f; // 验证集比例保持不变
    config.earlyStoppingPatience = 10; // 减少耐心值，更快触发早停
    
    // 创建训练器
    SLTrainer trainer(config);
    
    // 检查是否存在已有模型，如果有则加载继续训练
    std::string modelPath = "d:/steam/steamapps/common/Noita/mods/NoitaCoreAI/aiDev/models/SL_models/trained_model.bin";
    if (std::filesystem::exists(modelPath)) {
        try {
            trainer.loadModel(modelPath);
            std::cout << "Loaded existing model for continued training" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Warning: Failed to load existing model, starting fresh training: " << e.what() << std::endl;
        }
    } else {
        std::cout << "No existing model found, starting fresh training" << std::endl;
    }
    
    // 加载训练数据
    std::vector<SimpleML::EpisodeData> trainingData;
    
    // 从CSV文件读取数据（使用清洗后的数据）
    std::ifstream file("d:/steam/steamapps/common/Noita/mods/NoitaCoreAI/aiDev/data/training_dataset_reduced.csv");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open training data file!" << std::endl;
        return 1;
    }
    
    std::string line;
    std::vector<std::vector<float>> allData;
    
    // 跳过表头
    std::getline(file, line);
    
    // 读取所有数据行
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;
        std::vector<float> row;
        
        while (std::getline(ss, value, ',')) {
            row.push_back(std::stof(value));
        }
        
        if (row.size() >= 132) {
            allData.push_back(row);
        }
    }
    file.close();
    
    std::cout << "Loaded " << allData.size() << " training samples" << std::endl;
    
    if (allData.empty()) {
        std::cerr << "Error: No training data found!" << std::endl;
        return 1;
    }
    
    // 创建单个episode包含所有数据（CSV已经是连续的游戏帧数据）
    SimpleML::EpisodeData episode;
    
    for (const auto& row : allData) {
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
        
        // 设置默认奖励和结束标志
        sample.reward = 0.0f;
        sample.done = false;
        
        episode.states.push_back(sample);
    }
    
    // 设置最后一帧为episode结束
    if (!episode.states.empty()) {
        episode.states.back().done = true;
        trainingData.push_back(episode);
    }
    
    std::cout << "Created " << trainingData.size() << " episode with " << episode.states.size() << " samples" << std::endl;
    
    // 开始训练
    std::cout << "Starting training..." << std::endl;
    trainer.trainFromData(trainingData);
    
    // 保存最终模型（使用清洗数据训练的模型）
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_time);
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &tm);
    
    std::string newModelPath = "d:/steam/steamapps/common/Noita/mods/NoitaCoreAI/aiDev/models/SL_models/trained_model_reduced_" + std::string(timestamp) + ".bin";
    trainer.saveModel(newModelPath);
    std::cout << "Model (trained with reduced data) saved to: " << newModelPath << std::endl;
    
    return 0;
}