#include "TrainingManager.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>

TrainingManager::TrainingManager(const TrainingConfig& config) : config(config) {
    traditionalTrainer = std::make_unique<SLTrainer>(config.traditionalConfig);
    sequenceTrainer = std::make_unique<SequenceTrainer>(config.sequenceConfig);
    
    // 确保模型目录存在
    createDirectory(config.modelDirectory);
}

TrainingManager::~TrainingManager() = default;

TrainingManager::TrainingResult TrainingManager::startTraining(const std::vector<SimpleML::EpisodeData>& episodes) {
    TrainingResult result;
    auto startTime = std::chrono::steady_clock::now();
    
    try {
        // 验证数据
        if (!validateData(episodes)) {
            result.errorMessage = "Invalid training data";
            return result;
        }
        
        // 根据模式选择训练方式
        switch (config.mode) {
            case TrainingMode::TRADITIONAL:
                result = trainTraditional(episodes);
                break;
            case TrainingMode::SEQUENCE:
                result = trainSequence(episodes);
                break;
            case TrainingMode::HYBRID:
                result = trainHybrid(episodes);
                break;
        }
        
    } catch (const std::exception& e) {
        result.errorMessage = e.what();
        return result;
    }
    
    auto endTime = std::chrono::steady_clock::now();
    result.trainingTimeSeconds = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count());
    
    return result;
}

TrainingManager::TrainingResult TrainingManager::startSequenceTraining(const std::vector<SequenceML::SequenceEpisodeData>& episodes) {
    TrainingResult result;
    auto startTime = std::chrono::steady_clock::now();
    
    try {
        if (episodes.empty()) {
            result.errorMessage = "No sequence training data provided";
            return result;
        }
        
        sequenceTrainer->trainFromSequences(episodes);
        
        // 获取训练统计
        auto stats = sequenceTrainer->getSequenceTrainingStats();
        result.sequenceAccuracy = stats.sequenceAccuracy;
        result.success = true;
        
    } catch (const std::exception& e) {
        result.errorMessage = e.what();
        return result;
    }
    
    auto endTime = std::chrono::steady_clock::now();
    result.trainingTimeSeconds = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count());
    
    return result;
}

void TrainingManager::saveAllModels(const std::string& prefix) {
    // 保存传统模型
    std::string traditionalModelPath = config.modelDirectory + "/" + prefix + "_traditional_model.bin";
    traditionalTrainer->saveModel(traditionalModelPath);
    
    // 保存序列模型
    std::string sequenceModelPath = config.modelDirectory + "/" + prefix + "_sequence_model.bin";
    sequenceTrainer->saveSequenceModel(sequenceModelPath);
    
    std::cout << "All models saved with prefix: " << prefix << std::endl;
}

void TrainingManager::loadAllModels(const std::string& prefix) {
    // 加载传统模型
    std::string traditionalModelPath = config.modelDirectory + "/" + prefix + "_traditional_model.bin";
    if (std::filesystem::exists(traditionalModelPath)) {
        traditionalTrainer->loadModel(traditionalModelPath);
    }
    
    // 加载序列模型
    std::string sequenceModelPath = config.modelDirectory + "/" + prefix + "_sequence_model.bin";
    if (std::filesystem::exists(sequenceModelPath)) {
        sequenceTrainer->loadSequenceModel(sequenceModelPath);
    }
    
    std::cout << "All models loaded with prefix: " << prefix << std::endl;
}

void TrainingManager::setTrainingConfig(const TrainingConfig& config) {
    this->config = config;
    traditionalTrainer = std::make_unique<SLTrainer>(config.traditionalConfig);
    sequenceTrainer = std::make_unique<SequenceTrainer>(config.sequenceConfig);
}

TrainingManager::TrainingConfig TrainingManager::getTrainingConfig() const {
    return config;
}

// ==================== 私有方法实现 ====================

std::vector<SimpleML::TrainingData> TrainingManager::prepareTraditionalData(const std::vector<SimpleML::EpisodeData>& episodes) {
    std::vector<SimpleML::TrainingData> trainingData;
    
    for (const auto& episode : episodes) {
        for (const auto& sample : episode.states) {
            trainingData.push_back(sample);
        }
    }
    
    return trainingData;
}

std::vector<SequenceML::SequenceEpisodeData> TrainingManager::prepareSequenceData(const std::vector<SimpleML::EpisodeData>& episodes) {
    std::vector<SequenceML::SequenceTrainingData> sequences;
    
    // 为每个回合创建序列
    for (const auto& episode : episodes) {
        SequenceML::SequenceEpisodeData sequenceEpisode;
        sequenceTrainer->createSequencesFromEpisodes({episode}, sequences);
        sequenceEpisode.sequences = sequences;
    }
    
    return {SequenceML::SequenceEpisodeData{sequences}};
}

bool TrainingManager::validateData(const std::vector<SimpleML::EpisodeData>& episodes) {
    if (episodes.empty()) {
        return false;
    }
    
    for (const auto& episode : episodes) {
        if (episode.states.empty()) {
            return false;
        }
        
        for (const auto& sample : episode.states) {
            if (sample.state.empty() || sample.action.empty()) {
                return false;
            }
        }
    }
    
    return true;
}

void TrainingManager::createDirectory(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }
}

TrainingManager::TrainingResult TrainingManager::trainTraditional(const std::vector<SimpleML::EpisodeData>& episodes) {
    TrainingResult result;
    
    auto trainingData = prepareTraditionalData(episodes);
    if (trainingData.empty()) {
        result.errorMessage = "No valid traditional training data";
        return result;
    }
    
    // 创建单个回合用于训练
    SimpleML::EpisodeData episode;
    episode.states = trainingData;
    
    traditionalTrainer->trainFromData({episode});
    
    // 计算实际准确率
    std::vector<SimpleML::EpisodeData> testEpisodes = {episode};
    result.traditionalAccuracy = traditionalTrainer->evaluate(testEpisodes);
    result.success = true;
    
    return result;
}

TrainingManager::TrainingResult TrainingManager::trainSequence(const std::vector<SimpleML::EpisodeData>& episodes) {
    TrainingResult result;
    
    auto sequenceEpisodes = prepareSequenceData(episodes);
    if (sequenceEpisodes.empty() || sequenceEpisodes[0].sequences.empty()) {
        result.errorMessage = "No valid sequence training data";
        return result;
    }
    
    sequenceTrainer->trainFromSequences(sequenceEpisodes);
    
    auto stats = sequenceTrainer->getSequenceTrainingStats();
    result.sequenceAccuracy = stats.sequenceAccuracy;
    result.success = true;
    
    return result;
}

TrainingManager::TrainingResult TrainingManager::trainHybrid(const std::vector<SimpleML::EpisodeData>& episodes) {
    TrainingResult result;
    
    std::cout << "Starting hybrid training..." << std::endl;
    
    // 先训练传统模型
    std::cout << "Phase 1: Training traditional model..." << std::endl;
    auto traditionalResult = trainTraditional(episodes);
    if (!traditionalResult.success) {
        result.errorMessage = "Traditional training failed: " + traditionalResult.errorMessage;
        return result;
    }
    
    // 再训练序列模型
    std::cout << "Phase 2: Training sequence model..." << std::endl;
    auto sequenceResult = trainSequence(episodes);
    if (!sequenceResult.success) {
        result.errorMessage = "Sequence training failed: " + sequenceResult.errorMessage;
        return result;
    }
    
    result.traditionalAccuracy = traditionalResult.traditionalAccuracy;
    result.sequenceAccuracy = sequenceResult.sequenceAccuracy;
    result.hybridAccuracy = (traditionalResult.traditionalAccuracy + sequenceResult.sequenceAccuracy) / 2.0f;
    result.success = true;
    
    std::cout << "Hybrid training completed successfully!" << std::endl;
    std::cout << "Traditional accuracy: " << result.traditionalAccuracy << std::endl;
    std::cout << "Sequence accuracy: " << result.sequenceAccuracy << std::endl;
    std::cout << "Hybrid accuracy: " << result.hybridAccuracy << std::endl;
    
    return result;
}