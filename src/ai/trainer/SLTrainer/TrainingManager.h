#pragma once

#include "SLTrainer.h"
#include "SequenceTrainer.h"
#include <memory>
#include <string>

/**
 * @brief 训练管理器类
 * 统一管理传统监督学习和序列学习的训练流程
 */
class TrainingManager {
public:
    /**
     * @brief 训练模式枚举
     */
    enum class TrainingMode {
        TRADITIONAL,    ///< 传统监督学习
        SEQUENCE,       ///< 序列学习
        HYBRID          ///< 混合训练
    };
    
    /**
     * @brief 训练配置结构体
     */
    struct TrainingConfig {
        TrainingMode mode = TrainingMode::HYBRID;
        SLTrainer::TrainingConfig traditionalConfig;
        SequenceTrainer::SequenceTrainingConfig sequenceConfig;
        std::string dataDirectory = "training_data";
        std::string modelDirectory = "models";
        bool enableDataAugmentation = true;
        bool enableCrossValidation = false;
        int crossValidationFolds = 5;
        
        TrainingConfig() = default;
    };
    
    /**
     * @brief 训练结果结构体
     */
    struct TrainingResult {
        bool success = false;
        float traditionalAccuracy = 0.0f;
        float sequenceAccuracy = 0.0f;
        float hybridAccuracy = 0.0f;
        std::string errorMessage;
        int trainingTimeSeconds = 0;
        
        TrainingResult() = default;
    };

public:
    /**
     * @brief 构造函数
     * @param config 训练配置
     */
    TrainingManager(const TrainingConfig& config);
    
    /**
     * @brief 析构函数
     */
    ~TrainingManager();
    
    /**
     * @brief 开始训练
     * @param episodes 训练数据回合
     * @return 训练结果
     */
    TrainingResult startTraining(const std::vector<SimpleML::EpisodeData>& episodes);
    
    /**
     * @brief 开始序列训练
     * @param episodes 序列训练数据回合
     * @return 训练结果
     */
    TrainingResult startSequenceTraining(const std::vector<SequenceML::SequenceEpisodeData>& episodes);
    
    /**
     * @brief 保存所有模型
     * @param prefix 模型前缀
     */
    void saveAllModels(const std::string& prefix);
    
    /**
     * @brief 加载所有模型
     * @param prefix 模型前缀
     */
    void loadAllModels(const std::string& prefix);
    
    /**
     * @brief 设置训练配置
     * @param config 新的配置
     */
    void setTrainingConfig(const TrainingConfig& config);
    
    /**
     * @brief 获取训练配置
     * @return 当前配置
     */
    TrainingConfig getTrainingConfig() const;
    
    /**
     * @brief 获取传统训练器
     * @return 传统训练器指针
     */
    SLTrainer* getTraditionalTrainer() const { return traditionalTrainer.get(); }
    
    /**
     * @brief 获取序列训练器
     * @return 序列训练器指针
     */
    SequenceTrainer* getSequenceTrainer() const { return sequenceTrainer.get(); }

private:
    TrainingConfig config;
    std::unique_ptr<SLTrainer> traditionalTrainer;
    std::unique_ptr<SequenceTrainer> sequenceTrainer;
    
    /**
     * @brief 准备训练数据
     * @param episodes 原始数据
     * @return 预处理后的数据
     */
    std::vector<SimpleML::TrainingData> prepareTraditionalData(const std::vector<SimpleML::EpisodeData>& episodes);
    
    /**
     * @brief 准备序列训练数据
     * @param episodes 原始数据
     * @return 序列训练数据
     */
    std::vector<SequenceML::SequenceEpisodeData> prepareSequenceData(const std::vector<SimpleML::EpisodeData>& episodes);
    
    /**
     * @brief 验证数据完整性
     * @param episodes 数据回合
     * @return 验证结果
     */
    bool validateData(const std::vector<SimpleML::EpisodeData>& episodes);
    
    /**
     * @brief 创建目录
     * @param path 目录路径
     */
    void createDirectory(const std::string& path);
    
    /**
     * @brief 传统训练
     * @param episodes 训练数据
     * @return 训练结果
     */
    TrainingResult trainTraditional(const std::vector<SimpleML::EpisodeData>& episodes);
    
    /**
     * @brief 序列训练
     * @param episodes 训练数据
     * @return 训练结果
     */
    TrainingResult trainSequence(const std::vector<SimpleML::EpisodeData>& episodes);
    
    /**
     * @brief 混合训练
     * @param episodes 训练数据
     * @return 训练结果
     */
    TrainingResult trainHybrid(const std::vector<SimpleML::EpisodeData>& episodes);
};