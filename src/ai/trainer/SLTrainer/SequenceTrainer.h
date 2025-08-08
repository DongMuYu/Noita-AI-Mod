#pragma once

#include <vector>
#include <memory>
#include <random>
#include <limits>
#include <string>
#include <deque>
#include "SLTrainer.h"  // 包含EpisodeData定义

namespace SequenceML {
    /**
     * @brief 序列训练数据结构体
     * 存储序列训练样本的数据，包括状态序列、动作序列和目标动作
     */
    struct SequenceTrainingData {
        std::vector<std::vector<float>> stateSequence;   ///< 状态序列 (150帧×130特征)
        std::vector<std::vector<float>> actionSequence;  ///< 动作序列 (150帧×2动作)
        std::vector<float> targetAction;                   ///< 下一帧目标动作 (2维)
        int sequenceLength;                               ///< 序列实际长度
        
        SequenceTrainingData() : sequenceLength(150) {}
    };
    
    /**
     * @brief 序列回合数据结构体
     * 存储一个完整回合的所有序列训练数据
     */
    struct SequenceEpisodeData {
        std::vector<SequenceTrainingData> sequences;  ///< 序列训练数据
    };
}

/**
 * @brief 序列学习训练器类
 * 实现基于LSTM的序列模型训练，用于学习时序决策模式
 */
class SequenceTrainer {
public:
    /**
     * @brief 序列训练配置结构体
     * 包含序列训练过程中的所有超参数配置
     */
    struct SequenceTrainingConfig {
        int batchSize = 32;                ///< 批次大小
        int epochs = 1000;                 ///< 训练轮数
        float learningRate = 0.001f;       ///< 学习率
        float validationSplit = 0.2f;      ///< 验证集比例
        float earlyStoppingPatience = 15;    ///< 早停耐心值
        int sequenceLength = 150;          ///< 序列长度
        int lstmHiddenSize1 = 256;         ///< 第一层LSTM隐藏层大小
        int lstmHiddenSize2 = 128;         ///< 第二层LSTM隐藏层大小
        int denseHiddenSize = 64;          ///< 全连接隐藏层大小
        float dropoutRate = 0.2f;          ///< Dropout比率
        bool useLayerNorm = true;          ///< 是否使用层归一化
        
        SequenceTrainingConfig() = default;
    };

public:
    /**
     * @brief 构造函数
     * @param config 序列训练配置参数
     */
    SequenceTrainer(const SequenceTrainingConfig& config);
    
    /**
     * @brief 析构函数
     */
    virtual ~SequenceTrainer();
    
    /**
     * @brief 从序列数据训练模型
     * @param episodes 包含多个回合的序列数据
     */
    void trainFromSequences(const std::vector<SequenceML::SequenceEpisodeData>& episodes);
    
    /**
     * @brief 单步序列训练
     * @param batch 序列训练批次
     * @param step 当前训练步数
     * @return 当前批次的损失值
     */
    float trainSequenceStep(const std::vector<SequenceML::SequenceTrainingData>& batch, int step);
    
    /**
     * @brief 保存序列模型
     * @param filename 模型文件名
     */
    void saveSequenceModel(const std::string& filename);
    
    /**
     * @brief 加载序列模型
     * @param filename 模型文件名
     */
    void loadSequenceModel(const std::string& filename);
    
    /**
     * @brief 评估序列模型
     * @param testEpisodes 测试序列数据
     * @return 模型在测试集上的性能指标
     */
    float evaluateSequence(const std::vector<SequenceML::SequenceEpisodeData>& testEpisodes);
    
    /**
     * @brief 序列预测
     * @param stateSequence 状态序列输入
     * @return 预测的动作
     */
    std::vector<float> predictSequence(const std::vector<std::vector<float>>& stateSequence);
    
    /**
     * @brief 从回合数据创建序列
     * @param episodes 原始回合数据
     * @param sequences 输出的序列数据
     */
    void createSequencesFromEpisodes(const std::vector<SimpleML::EpisodeData>& episodes,
                                   std::vector<SequenceML::SequenceTrainingData>& sequences);
    
    /**
     * @brief 设置序列训练配置
     * @param config 新的配置
     */
    void setSequenceConfig(const SequenceTrainingConfig& config);
    
    /**
     * @brief 获取当前序列训练配置
     * @return 当前配置
     */
    SequenceTrainingConfig getSequenceConfig() const;
    
    /**
     * @brief 序列训练统计信息
     */
    struct SequenceTrainingStats {
        float trainingLoss;           ///< 训练损失
        float validationLoss;         ///< 验证损失
        float sequenceAccuracy;       ///< 序列准确率
        int epochsCompleted;          ///< 已完成轮数
        int bestEpoch;                ///< 最佳轮数
        float bestValidationLoss;     ///< 最佳验证损失
        float actionAccuracy;         ///< 动作预测准确率
        float temporalConsistency;    ///< 时序一致性
        
        SequenceTrainingStats() : 
            trainingLoss(0.0f), validationLoss(0.0f), sequenceAccuracy(0.0f),
            epochsCompleted(0), bestEpoch(0), bestValidationLoss(std::numeric_limits<float>::max()),
            actionAccuracy(0.0f), temporalConsistency(0.0f) {}
    };
    
    /**
     * @brief 获取序列训练统计信息
     * @return 训练统计信息
     */
    SequenceTrainingStats getSequenceTrainingStats() const;

private:
    /**
     * @brief LSTM序列模型类
     * 实现LSTM序列神经网络
     */
    class LSTMSequenceModel {
    public:
        /**
         * @brief 构造函数
         * @param config 序列训练配置
         */
        LSTMSequenceModel(const SequenceTrainingConfig& config);
        
        /**
         * @brief 析构函数
         */
        ~LSTMSequenceModel();
        
        /**
         * @brief 前向传播
         * @param sequences 输入序列批次
         * @return 预测动作
         */
        std::vector<std::vector<float>> forward(const std::vector<std::vector<std::vector<float>>>& sequences);
        
        /**
         * @brief 训练步骤
         * @param sequences 输入序列
         * @param targets 目标动作
         * @param learningRate 学习率
         * @param step 训练步数
         * @return 损失值
         */
        float trainStep(const std::vector<std::vector<std::vector<float>>>& sequences,
                       const std::vector<std::vector<float>>& targets,
                       float learningRate, int step);
        
        /**
         * @brief 评估模型
         * @param sequences 输入序列
         * @param targets 目标动作
         * @return 损失值
         */
        float evaluate(const std::vector<std::vector<std::vector<float>>>& sequences,
                      const std::vector<std::vector<float>>& targets);
        
        /**
         * @brief 保存模型
         * @param filename 文件名
         */
        void save(const std::string& filename);
        
        /**
         * @brief 加载模型
         * @param filename 文件名
         */
        void load(const std::string& filename);
        
        /**
         * @brief 获取模型参数
         * @return 参数向量
         */
        std::vector<float> getParameters() const;
        
        /**
         * @brief 设置模型参数
         * @param params 参数向量
         */
        void setParameters(const std::vector<float>& params);
        
    private:
        SequenceTrainingConfig config;
        
        // LSTM权重和偏置
        std::vector<std::vector<float>> lstm1Weights;      ///< 第一层LSTM权重
        std::vector<std::vector<float>> lstm1Biases;     ///< 第一层LSTM偏置
        std::vector<std::vector<float>> lstm2Weights;      ///< 第二层LSTM权重
        std::vector<std::vector<float>> lstm2Biases;     ///< 第二层LSTM偏置
        
        // 全连接层权重和偏置
        std::vector<std::vector<float>> denseWeights;      ///< 全连接层权重
        std::vector<float> denseBiases;       ///< 全连接层偏置
        
        // 优化器状态
        std::vector<std::vector<float>> adamM1, adamV1;    ///< 第一层LSTM优化器状态
        std::vector<std::vector<float>> adamM2, adamV2;    ///< 第二层LSTM优化器状态
        std::vector<std::vector<float>> adamMD, adamVD;    ///< 全连接层优化器状态
        
        std::mt19937 rng;                                ///< 随机数生成器
        
        /**
         * @brief 初始化LSTM权重
         */
        void initializeLSTMWeights();
        
        /**
         * @brief LSTM前向传播
         * @param input 输入序列
         * @param hiddenState 隐藏状态
         * @param cellState 细胞状态
         * @return 输出
         */
        std::vector<float> lstmForward(const std::vector<float>& input,
                                     std::vector<float>& hiddenState,
                                     std::vector<float>& cellState,
                                     const std::vector<std::vector<float>>& weights,
                                     const std::vector<float>& biases);
        
        /**
         * @brief 计算损失
         * @param predictions 预测值
         * @param targets 目标值
         * @return 损失
         */
        float computeLoss(const std::vector<std::vector<float>>& predictions,
                         const std::vector<std::vector<float>>& targets);
        
        /**
         * @brief Xavier初始化
         * @param weights 权重向量
         * @param fanIn 输入维度
         * @param fanOut 输出维度
         */
        void xavierInitialize(std::vector<float>& weights, int fanIn, int fanOut);
    };

private:
    SequenceTrainingConfig config;                          ///< 序列训练配置
    std::unique_ptr<LSTMSequenceModel> sequenceModel;        ///< 序列模型实例
    SequenceTrainingStats stats;                             ///< 序列训练统计信息
    
    /**
     * @brief 预处理序列状态数据
     * @param data 序列训练数据
     * @return 预处理后的状态序列
     */
    std::vector<std::vector<std::vector<float>>> preprocessSequenceStates(
        const std::vector<SequenceML::SequenceTrainingData>& data);
    
    /**
     * @brief 预处理序列目标数据
     * @param data 序列训练数据
     * @return 预处理后的目标动作
     */
    std::vector<std::vector<float>> preprocessSequenceTargets(
        const std::vector<SequenceML::SequenceTrainingData>& data);
    
    /**
     * @brief 分割序列数据集
     * @param data 原始序列数据
     * @param trainSet 训练集
     * @param valSet 验证集
     */
    void splitSequenceDataset(const std::vector<SequenceML::SequenceTrainingData>& data,
                            std::vector<SequenceML::SequenceTrainingData>& trainSet,
                            std::vector<SequenceML::SequenceTrainingData>& valSet);
    
    /**
     * @brief 分割大数据集（分批处理）
     * @param data 原始序列数据
     * @param trainSet 训练集
     * @param valSet 验证集
     * @param batchSize 批次大小
     */
    void splitLargeDataset(const std::vector<SequenceML::SequenceTrainingData>& data,
                          std::vector<SequenceML::SequenceTrainingData>& trainSet,
                          std::vector<SequenceML::SequenceTrainingData>& valSet,
                          size_t batchSize);
    
    /**
     * @brief 计算序列准确率
     * @param predictions 预测值
     * @param targets 目标值
     * @return 准确率
     */
    float computeSequenceAccuracy(const std::vector<std::vector<float>>& predictions,
                                const std::vector<std::vector<float>>& targets);
    
    /**
     * @brief 计算时序一致性
     * @param predictions 预测序列
     * @return 时序一致性得分
     */
    float computeTemporalConsistency(const std::vector<std::vector<float>>& predictions);
};