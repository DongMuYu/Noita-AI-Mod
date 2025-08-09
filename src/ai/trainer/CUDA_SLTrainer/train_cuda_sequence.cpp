#include "CUDASequenceTrainer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>

// 独立定义EpisodeData结构体，不依赖SLTrainer
namespace SimpleML {
    /**
     * @brief 训练数据结构体
     * 存储单帧训练样本的数据
     */
    struct TrainingData {
        std::vector<float> state;   ///< 状态特征 (130维)
        std::vector<float> action;  ///< 动作数据 (2维)
        float reward;               ///< 奖励值
        bool done;                  ///< 是否结束
    };
    
    /**
     * @brief 回合数据结构体
     * 存储一个完整回合的所有训练数据
     */
    struct EpisodeData {
        std::vector<TrainingData> states;  ///< 状态序列
    };
}

/**
 * @brief 从CSV文件加载训练数据
 * @param filename CSV文件路径
 * @return 加载的回合数据
 */
std::vector<SimpleML::EpisodeData> loadTrainingDataFromCSV(const std::string& filename) {
    std::vector<SimpleML::EpisodeData> episodes;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return episodes;
    }
    
    std::string line;
    SimpleML::EpisodeData currentEpisode;
    
    // 跳过标题行
    if (!std::getline(file, line)) {
        std::cerr << "Error: Empty CSV file" << std::endl;
        return episodes;
    }
    
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;
        std::vector<std::string> values;
        
        // 分割CSV行
        while (std::getline(ss, value, ',')) {
            values.push_back(value);
        }
        
        if (values.size() < 134) {  // 130状态 + 2动作 + 1奖励 + 1结束标记
            std::cerr << "Warning: Invalid data format in line: " << line << std::endl;
            continue;
        }
        
        // 解析训练数据
        SimpleML::TrainingData data;
        
        // 解析状态特征 (130维)
        for (int i = 0; i < 130; ++i) {
            try {
                data.state.push_back(std::stof(values[i]));
            } catch (const std::exception& e) {
                data.state.push_back(0.0f);
            }
        }
        
        // 解析动作数据 (2维)
        try {
            data.action.push_back(std::stof(values[130]));
            data.action.push_back(std::stof(values[131]));
        } catch (const std::exception& e) {
            data.action = {0.0f, 0.0f};
        }
        
        // 解析奖励
        try {
            data.reward = std::stof(values[132]);
        } catch (const std::exception& e) {
            data.reward = 0.0f;
        }
        
        // 解析结束标记
        try {
            data.done = (std::stoi(values[133]) == 1);
        } catch (const std::exception& e) {
            data.done = false;
        }
        
        currentEpisode.states.push_back(data);
        
        // 如果当前回合结束，保存并开始新回合
        if (data.done) {
            if (!currentEpisode.states.empty()) {
                episodes.push_back(currentEpisode);
                currentEpisode.states.clear();
            }
        }
    }
    
    // 保存最后一个未结束的回合
    if (!currentEpisode.states.empty()) {
        episodes.push_back(currentEpisode);
    }
    
    file.close();
    
    std::cout << "Loaded " << episodes.size() << " episodes from " << filename << std::endl;
    
    return episodes;
}

/**
 * @brief 主函数 - CUDA序列训练器
 */
int main() {
    try {
        std::cout << "=== CUDA Sequence Trainer ===" << std::endl;
        
        // 配置CUDA序列训练器
        CUDASequenceTrainer::CUDASequenceTrainingConfig config;
        config.batchSize = 32;
        config.epochs = 1000;
        config.learningRate = 0.001f;
        config.validationSplit = 0.2f;
        config.earlyStoppingPatience = 15;
        config.sequenceLength = 150;
        config.lstmHiddenSize1 = 256;
        config.lstmHiddenSize2 = 128;
        config.denseHiddenSize = 64;
        config.dropoutRate = 0.2f;
        config.useLayerNorm = true;
        config.gpuDeviceId = 0;
        config.useTensorCores = true;
        config.memoryPoolSize = 1024 * 1024 * 1024;  // 1GB
        
        std::cout << "CUDA Sequence Training Configuration:" << std::endl;
        std::cout << "  Batch Size: " << config.batchSize << std::endl;
        std::cout << "  Epochs: " << config.epochs << std::endl;
        std::cout << "  Learning Rate: " << config.learningRate << std::endl;
        std::cout << "  Sequence Length: " << config.sequenceLength << std::endl;
        std::cout << "  LSTM Hidden Sizes: " << config.lstmHiddenSize1 << ", " << config.lstmHiddenSize2 << std::endl;
        std::cout << "  GPU Device ID: " << config.gpuDeviceId << std::endl;
        std::cout << "  Use Tensor Cores: " << (config.useTensorCores ? "Yes" : "No") << std::endl;
        std::cout << std::endl;
        
        // 创建CUDA序列训练器
        CUDASequenceTrainer trainer(config);
        
        // 检查CUDA状态
        if (!trainer.checkCUDAStatus()) {
            std::cerr << "Error: CUDA initialization failed" << std::endl;
            return 1;
        }
        
        // 显示GPU信息
        std::cout << "GPU Information:" << std::endl;
        std::cout << "  " << trainer.getGPUInfo() << std::endl;
        std::cout << std::endl;
        
        // 加载训练数据
        std::string dataFile = "training_data.csv";
        std::cout << "Loading training data from: " << dataFile << std::endl;
        
        auto episodes = loadTrainingDataFromCSV(dataFile);
        if (episodes.empty()) {
            std::cerr << "Error: No training data loaded" << std::endl;
            return 1;
        }
        
        // 将回合数据转换为序列数据
        std::vector<SequenceML::SequenceTrainingData> sequences;
        trainer.createSequencesFromEpisodes(episodes, sequences);
        
        if (sequences.empty()) {
            std::cerr << "Error: No sequences created from episodes" << std::endl;
            return 1;
        }
        
        // 创建序列回合数据
        std::vector<SequenceML::SequenceEpisodeData> sequenceEpisodes;
        SequenceML::SequenceEpisodeData seqEpisode;
        seqEpisode.sequences = sequences;
        sequenceEpisodes.push_back(seqEpisode);
        
        std::cout << "Created " << sequences.size() << " sequences for training" << std::endl;
        std::cout << std::endl;
        
        // 开始训练
        std::cout << "Starting CUDA sequence training..." << std::endl;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        trainer.trainFromSequences(sequenceEpisodes);
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
        
        std::cout << std::endl;
        std::cout << "Training completed in " << duration.count() << " seconds" << std::endl;
        
        // 获取训练统计信息
        auto stats = trainer.getCUDASequenceTrainingStats();
        std::cout << "\n=== Training Statistics ===" << std::endl;
        std::cout << "Epochs Completed: " << stats.epochsCompleted << std::endl;
        std::cout << "Best Epoch: " << stats.bestEpoch << std::endl;
        std::cout << "Training Loss: " << std::fixed << std::setprecision(4) << stats.trainingLoss << std::endl;
        std::cout << "Validation Loss: " << stats.validationLoss << std::endl;
        std::cout << "Best Validation Loss: " << stats.bestValidationLoss << std::endl;
        std::cout << "Action Accuracy: " << stats.actionAccuracy * 100.0f << "%" << std::endl;
        std::cout << "Temporal Consistency: " << stats.temporalConsistency << std::endl;
        std::cout << "GPU Memory Usage: " << stats.gpuMemoryUsage << " MB" << std::endl;
        std::cout << "Training Speedup: " << stats.trainingSpeedup << "x" << std::endl;
        
        // 保存最终模型
        std::string modelFile = "cuda_sequence_model_final.bin";
        trainer.saveSequenceModel(modelFile);
        
        std::cout << "\nModel saved to: " << modelFile << std::endl;
        std::cout << "CUDA sequence training completed successfully!" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}