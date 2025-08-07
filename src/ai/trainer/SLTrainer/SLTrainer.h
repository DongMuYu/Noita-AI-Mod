// sltrainer.h
// 监督学习训练器头文件
// 用于实现基于行为克隆的监督学习训练功能

#ifndef SL_TRAINER_H
#define SL_TRAINER_H

#pragma once

#include <vector>
#include <memory>
#include <random>
#include <limits>
#include <string>

namespace SimpleML {
    /**
     * @brief 训练数据结构体
     * 存储单个训练样本的数据，包括状态、动作、奖励和终止标志
     */
    struct TrainingData {
        std::vector<float> state;   ///< 环境状态向量
        std::vector<float> action;  ///< 执行的动作向量
        float reward;               ///< 获得的奖励值
        bool done;                  ///< 是否为终止状态
        
        TrainingData() : reward(0.0f), done(false) {}
    };
    
    /**
     * @brief 回合数据结构体
     * 存储一个完整回合的所有训练数据
     */
    struct EpisodeData {
        std::vector<TrainingData> states;  ///< 回合中的所有状态序列
    };
}

/**
 * @brief 监督学习训练器类
 * 实现基于行为克隆的监督学习训练，用于训练AI代理模仿专家行为
 */
class SLTrainer {
public:
    /**
     * @brief 训练配置结构体
     * 包含训练过程中的所有超参数配置
     */
    struct TrainingConfig {
        int batchSize = 64;                ///< 批次大小，每批次训练的样本数量
        int epochs = 1000;                 ///< 训练轮数，整个数据集的训练次数
        float learningRate = 0.001f;       ///< 学习率，控制权重更新的步长
        float validationSplit = 0.2f;      ///< 验证集比例，用于划分训练和验证数据
        float earlyStoppingPatience = 10;  ///< 早停耐心值，验证损失不改善的最大轮数
        bool useDropout = false;           ///< 是否使用Dropout正则化
        float dropoutRate = 0.1f;          ///< Dropout比率，随机忽略神经元的概率
        
        TrainingConfig() = default;
    };

public:
    /**
     * @brief 构造函数
     * @param config 训练配置参数
     */
    SLTrainer(const TrainingConfig& config);
    
    /**
     * @brief 析构函数
     */
    virtual ~SLTrainer();

    /**
     * @brief 从回合数据训练模型
     * @param episodes 包含多个回合训练数据的向量
     */
    void trainFromData(const std::vector<SimpleML::EpisodeData>& episodes);
    
    /**
     * @brief 单步训练
     * @param batch 训练批次数据
     * @param step 当前训练步数
     * @return 当前批次的损失值
     */
    float trainStep(const std::vector<SimpleML::TrainingData>& batch, int step);
    
    /**
     * @brief 使用指定学习率的单步训练
     * @param batch 训练批次数据
     * @param learningRate 学习率
     * @param step 当前训练步数
     * @return 当前批次的损失值
     */
    float trainStepWithLearningRate(const std::vector<SimpleML::TrainingData>& batch, 
                                   float learningRate, int step);
    
    /**
     * @brief 保存训练好的模型
     * @param filename 模型文件名
     */
    void saveModel(const std::string& filename);
    
    /**
     * @brief 加载预训练模型
     * @param filename 模型文件名
     */
    void loadModel(const std::string& filename);
    
    /**
     * @brief 评估模型性能
     * @param testEpisodes 测试回合数据
     * @return 模型在测试集上的准确率
     */
    float evaluate(const std::vector<SimpleML::EpisodeData>& testEpisodes);
    
    /**
     * @brief 使用模型进行预测
     * @param state 输入状态
     * @return 预测的动作向量
     */
    std::vector<float> predict(const std::vector<float>& state);
    
    /**
     * @brief 从回合数据分割数据集
     * @param episodes 原始回合数据
     * @param trainSet 输出的训练集
     * @param valSet 输出的验证集
     */
    void splitDataset(const std::vector<SimpleML::EpisodeData>& episodes,
                       std::vector<SimpleML::TrainingData>& trainSet,
                       std::vector<SimpleML::TrainingData>& valSet);
    
    /**
     * @brief 从训练数据分割数据集
     * @param data 原始训练数据
     * @param trainSet 输出的训练集
     * @param valSet 输出的验证集
     */
    void splitDatasetFromTrainingData(const std::vector<SimpleML::TrainingData>& data,
                                    std::vector<SimpleML::TrainingData>& trainSet,
                                    std::vector<SimpleML::TrainingData>& valSet);
    
    /**
     * @brief 设置训练配置
     * @param config 新的训练配置
     */
    void setConfig(const TrainingConfig& config);
    
    /**
     * @brief 获取当前训练配置
     * @return 当前训练配置
     */
    TrainingConfig getConfig() const;
    
    /**
     * @brief 训练统计信息结构体
     * 记录训练过程中的各种统计指标
     */
    struct TrainingStats {
        float trainingLoss;        ///< 训练损失
        float validationLoss;      ///< 验证损失
        float trainingAccuracy;    ///< 训练准确率
        float validationAccuracy;  ///< 验证准确率
        int epochsCompleted;       ///< 已完成的训练轮数
        int bestEpoch;             ///< 最佳验证损失对应的轮数
        float bestValidationLoss;  ///< 最佳验证损失值
        
        TrainingStats() : 
            trainingLoss(0.0f), validationLoss(0.0f),
            trainingAccuracy(0.0f), validationAccuracy(0.0f),
            epochsCompleted(0), bestEpoch(0), bestValidationLoss(std::numeric_limits<float>::max()) {}
    };
    
    /**
     * @brief 获取训练统计信息
     * @return 训练统计信息
     */
    TrainingStats getTrainingStats() const;
    
    /**
     * @brief 重置训练统计信息
     */
    void resetTrainingStats();
    
    /**
     * @brief 特征重要性结构体
     * 用于存储特征重要性分析结果
     */
    struct FeatureImportance {
        std::string name;       ///< 特征名称
        float importance;       ///< 重要性分数
        float correlation;      ///< 相关性系数
    };
    
    /**
     * @brief 分析特征重要性
     * @param episodes 训练数据
     * @return 特征重要性分析结果
     */
    std::vector<FeatureImportance> analyzeFeatureImportance(
        const std::vector<SimpleML::EpisodeData>& episodes);

private:
    /**
     * @brief 行为克隆代理类
     * 实现具体的神经网络模型和训练逻辑
     */
    class BehaviorCloningAgent {
    public:
        /**
         * @brief 构造函数
         * @param config 训练配置
         */
        BehaviorCloningAgent(const TrainingConfig& config);
        
        /**
         * @brief 析构函数
         */
        ~BehaviorCloningAgent();
        
        /**
         * @brief 前向传播
         * @param state 输入状态
         * @return 预测的动作
         */
        std::vector<float> forward(const std::vector<float>& state);
        
        /**
         * @brief 训练模型
         * @param states 输入状态
         * @param actions 目标动作
         * @param learningRate 学习率
         * @param step 训练步数
         * @return 训练损失
         */
        float train(const std::vector<std::vector<float>>& states,
                   const std::vector<std::vector<float>>& actions,
                   float learningRate, int step);
        
        /**
         * @brief 评估模型
         * @param states 输入状态
         * @param actions 目标动作
         * @return 评估损失
         */
        float evaluate(const std::vector<std::vector<float>>& states,
                      const std::vector<std::vector<float>>& actions);
        
        /**
         * @brief 离散化预测输出
         * @param input 输入向量
         * @return 离散化后的输出向量
         */
        std::vector<float> predict(const std::vector<float>& input);
        
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
         * @return 模型参数向量
         */
        std::vector<float> getParameters() const;
        
        /**
         * @brief 设置模型参数
         * @param params 参数向量
         */
        void setParameters(const std::vector<float>& params);
        
    private:
        TrainingConfig config;                    ///< 训练配置
        const int inputDim = 130;                 ///< 输入维度
        const int hiddenDim1 = 256;               ///< 第一层隐藏层维度（扩展）
        const int hiddenDim2 = 128;               ///< 第二层隐藏层维度
        const int hiddenDim3 = 64;                ///< 第三层隐藏层维度
        const int hiddenDim4 = 32;                ///< 第四层隐藏层（新增）
        const int hiddenDim5 = 16;                ///< 第五层隐藏层（新增）
        const int outputDim = 2;                  ///< 输出维度（左右移动和上下移动）
        
        std::vector<std::vector<float>> network;  ///< 神经网络权重矩阵
        std::vector<std::vector<float>> biases;   ///< 神经网络偏置向量
        std::vector<std::vector<float>> biasGradients;  ///< 偏置梯度
        std::vector<std::vector<float>> adamM;    ///< Adam优化器的一阶矩估计
        std::vector<std::vector<float>> adamV;    ///< Adam优化器的二阶矩估计
        std::vector<std::vector<float>> adamMB;   ///< Adam优化器的偏置一阶矩估计
        std::vector<std::vector<float>> adamVB;   ///< Adam优化器的偏置二阶矩估计
        std::mt19937 rng;                        ///< 随机数生成器
        
        /**
         * @brief 初始化神经网络
         */
        void initializeNetwork();
        
        /**
         * @brief 初始化权重
         * @param weights 权重向量
         * @param in_dim 输入维度
         */
        void initializeWeights(std::vector<float>& weights, int in_dim);
        
        /**
         * @brief ReLU激活函数
         * @param x 输入值
         * @return 激活后的值
         */
        float relu(float x);
        
        /**
         * @brief 神经网络前向传播
         * @param input 输入向量
         * @return 输出向量
         */
        std::vector<float> forwardNetwork(const std::vector<float>& input);
        
        /**
         * @brief 输入归一化
         * @param input 输入向量
         * @return 归一化后的向量
         */
        std::vector<float> normalizeInput(const std::vector<float>& input);
        
        /**
         * @brief 计算损失函数
         * @param predicted 预测值
         * @param target 目标值
         * @return 损失值
         */
        float computeLoss(const std::vector<float>& predicted,
                         const std::vector<float>& target);
                         
        /**
         * @brief 计算正则化损失（L2正则化）
         * @param lambda 正则化系数
         * @return 正则化损失值
         */
        float computeRegularizationLoss(float lambda);
        
        /**
         * @brief 计算单个样本的梯度
         * @param state 输入状态
         * @param action 目标动作
         * @param outBiasGrads 输出的偏置梯度
         * @return 权重梯度
         */
        std::vector<std::vector<float>> computeGradientsSingle(
            const std::vector<float>& state,
            const std::vector<float>& action,
            std::vector<std::vector<float>>& outBiasGrads);
        
        /**
         * @brief 计算批次梯度
         * @param states 输入状态
         * @param actions 目标动作
         * @param outBiasGrads 输出的偏置梯度
         * @return 权重梯度
         */
        std::vector<std::vector<float>> computeGradients(
            const std::vector<std::vector<float>>& states,
            const std::vector<std::vector<float>>& actions,
            std::vector<std::vector<float>>& outBiasGrads);
            
        /**
         * @brief 计算梯度（重载版本）
         * @param state 输入状态
         * @param action 目标动作
         * @return 梯度
         */
        std::vector<std::vector<float>> computeGradients(
            const std::vector<float>& state,
            const std::vector<float>& action);
            
        /**
         * @brief Adam优化器更新
         * @param gradients 权重梯度
         * @param biasGradients 偏置梯度
         * @param lr 学习率
         * @param step 训练步数
         */
        void adamUpdate(const std::vector<std::vector<float>>& gradients,
                       const std::vector<std::vector<float>>& biasGradients,
                       float lr, int step);
    };

private:
    TrainingConfig config;                                   ///< 训练配置
    std::unique_ptr<BehaviorCloningAgent> agent;            ///< 行为克隆代理实例
    TrainingStats stats;                                    ///< 训练统计信息
    
    /**
     * @brief 预处理状态数据
     * @param data 训练数据
     * @return 预处理后的状态矩阵
     */
    std::vector<std::vector<float>> preprocessStates(const std::vector<SimpleML::TrainingData>& data);
    
    /**
     * @brief 预处理动作数据
     * @param data 训练数据
     * @return 预处理后的动作矩阵
     */
    std::vector<std::vector<float>> preprocessActions(const std::vector<SimpleML::TrainingData>& data);
    
    /**
     * @brief 数据增强
     * @param data 原始训练数据
     * @return 增强后的训练数据
     */
    std::vector<SimpleML::TrainingData> augmentData(const std::vector<SimpleML::TrainingData>& data);
    
    /**
     * @brief 计算准确率
     * @param predictions 预测值
     * @param targets 目标值
     * @return 准确率
     */
    float computeAccuracy(const std::vector<std::vector<float>>& predictions,
                       const std::vector<std::vector<float>>& targets);
    
    /**
     * @brief 计算均方误差
     * @param predictions 预测值
     * @param targets 目标值
     * @return 均方误差
     */
    float computeMSE(const std::vector<std::vector<float>>& predictions,
                    const std::vector<std::vector<float>>& targets);
};

#endif // SL_TRAINER_H