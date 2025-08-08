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
    std::cout << "Training Manager - Hybrid Learning System" << std::endl;
    std::cout << "=========================================" << std::endl;
    
    // 创建训练管理器配置
    TrainingManager::TrainingConfig config;
    
    // 传统训练配置
    config.traditionalConfig.epochs = 50;
    config.traditionalConfig.batchSize = 256;
    config.traditionalConfig.learningRate = 0.002f;
    config.traditionalConfig.validationSplit = 0.2f;
    config.traditionalConfig.earlyStoppingPatience = 10;
    
    // 序列训练配置
    config.sequenceConfig.epochs = 100;
    config.sequenceConfig.batchSize = 32;
    config.sequenceConfig.learningRate = 0.001f;
    config.sequenceConfig.validationSplit = 0.15f;
    config.sequenceConfig.earlyStoppingPatience = 20;
    config.sequenceConfig.sequenceLength = 150;
    config.sequenceConfig.lstmHiddenSize1 = 256;
    config.sequenceConfig.lstmHiddenSize2 = 128;
    config.sequenceConfig.denseHiddenSize = 64;
    config.sequenceConfig.dropoutRate = 0.2f;
    config.sequenceConfig.useLayerNorm = true;
    
    // 设置模式和目录
    config.mode = TrainingManager::TrainingMode::HYBRID;
    config.dataDirectory = "d:/steam/steamapps/common/Noita/mods/NoitaCoreAI/aiDev/data";
    config.modelDirectory = "d:/steam/steamapps/common/Noita/mods/NoitaCoreAI/aiDev/models/SL_models";
    config.enableDataAugmentation = true;
    config.enableCrossValidation = false;
    
    // 创建训练管理器
    TrainingManager manager(config);
    
    // 加载训练数据
    std::vector<SimpleML::EpisodeData> trainingData;
    
    // 从CSV文件读取数据
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
    
    // 创建回合数据
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
        
        sample.reward = 0.0f;
        sample.done = false;
        
        episode.states.push_back(sample);
    }
    
    if (!episode.states.empty()) {
        episode.states.back().done = true;
        trainingData.push_back(episode);
    }
    
    std::cout << "Created " << trainingData.size() << " episode with " << episode.states.size() << " samples" << std::endl;
    
    // 显示训练模式选择
    std::cout << "\nAvailable training modes:" << std::endl;
    std::cout << "1. Traditional (single frame)" << std::endl;
    std::cout << "2. Sequence (150-frame sequences)" << std::endl;
    std::cout << "3. Hybrid (both traditional and sequence)" << std::endl;
    std::cout << "Selected mode: ";
    
    switch (config.mode) {
        case TrainingManager::TrainingMode::TRADITIONAL:
            std::cout << "Traditional" << std::endl;
            break;
        case TrainingManager::TrainingMode::SEQUENCE:
            std::cout << "Sequence" << std::endl;
            break;
        case TrainingManager::TrainingMode::HYBRID:
            std::cout << "Hybrid" << std::endl;
            break;
    }
    
    // 开始训练
    std::cout << "\nStarting training..." << std::endl;
    auto result = manager.startTraining(trainingData);
    
    if (!result.success) {
        std::cerr << "Training failed: " << result.errorMessage << std::endl;
        return 1;
    }
    
    // 显示训练结果
    std::cout << "\nTraining completed successfully!" << std::endl;
    std::cout << "Training time: " << result.trainingTimeSeconds << " seconds" << std::endl;
    
    if (config.mode == TrainingManager::TrainingMode::TRADITIONAL || 
        config.mode == TrainingManager::TrainingMode::HYBRID) {
        std::cout << "Traditional accuracy: " << result.traditionalAccuracy << std::endl;
    }
    
    if (config.mode == TrainingManager::TrainingMode::SEQUENCE || 
        config.mode == TrainingManager::TrainingMode::HYBRID) {
        std::cout << "Sequence accuracy: " << result.sequenceAccuracy << std::endl;
    }
    
    if (config.mode == TrainingManager::TrainingMode::HYBRID) {
        std::cout << "Hybrid accuracy: " << result.hybridAccuracy << std::endl;
    }
    
    // 保存模型
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_time);
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &tm);
    
    std::string modelPrefix = "hybrid_model_" + std::string(timestamp);
    manager.saveAllModels(modelPrefix);
    
    std::cout << "\nModels saved successfully!" << std::endl;
    std::cout << "Traditional model: " << config.modelDirectory << "/" << modelPrefix << "_traditional_model.bin" << std::endl;
    std::cout << "Sequence model: " << config.modelDirectory << "/" << modelPrefix << "_sequence_model.bin" << std::endl;
    
    return 0;
}