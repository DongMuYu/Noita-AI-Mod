#pragma once

#include <vector>
#include <memory>
#include <random>
#include <limits>
#include <string>
#include <deque>
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <cudnn.h>

// 独立定义EpisodeData结构体，不依赖SLTrainer
namespace CUDA_SLTrainer {
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
 * @brief CUDA序列学习训练器类
 * 实现基于CUDA加速的LSTM序列模型训练，用于学习时序决策模式
 */
class CUDASequenceTrainer {
public:
    /**
     * @brief CUDA序列训练配置结构体
     * 包含序列训练过程中的所有超参数配置
     */
    struct CUDASequenceTrainingConfig {
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
        int gpuDeviceId = 0;               ///< GPU设备ID
        bool useTensorCores = true;        ///< 是否使用Tensor Cores
        int memoryPoolSize = 1024 * 1024 * 1024; ///< 内存池大小(1GB)
        
        CUDASequenceTrainingConfig() = default;
    };

public:
    /**
     * @brief 构造函数
     * @param config CUDA序列训练配置参数
     */
    CUDASequenceTrainer(const CUDASequenceTrainingConfig& config);
    
    /**
     * @brief 析构函数
     */
    virtual ~CUDASequenceTrainer();
    
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
     * @brief 设置CUDA序列训练配置
     * @param config 新的配置
     */
    void setCUDASequenceConfig(const CUDASequenceTrainingConfig& config);
    
    /**
     * @brief 获取当前CUDA序列训练配置
     * @return 当前配置
     */
    CUDASequenceTrainingConfig getCUDASequenceConfig() const;
    
    /**
     * @brief CUDA序列训练统计信息
     */
    struct CUDASequenceTrainingStats {
        float trainingLoss;           ///< 训练损失
        float validationLoss;         ///< 验证损失
        float sequenceAccuracy;       ///< 序列准确率
        int epochsCompleted;          ///< 已完成轮数
        int bestEpoch;                ///< 最佳轮数
        float bestValidationLoss;     ///< 最佳验证损失
        float actionAccuracy;         ///< 动作预测准确率
        float temporalConsistency;    ///< 时序一致性
        float gpuMemoryUsage;         ///< GPU内存使用量(MB)
        float trainingSpeedup;        ///< 相比CPU的训练加速比
        
        CUDASequenceTrainingStats() : 
            trainingLoss(0.0f), validationLoss(0.0f), sequenceAccuracy(0.0f),
            epochsCompleted(0), bestEpoch(0), bestValidationLoss(std::numeric_limits<float>::max()),
            actionAccuracy(0.0f), temporalConsistency(0.0f), gpuMemoryUsage(0.0f), trainingSpeedup(1.0f) {}
    };
    
    /**
     * @brief 获取CUDA序列训练统计信息
     * @return 训练统计信息
     */
    CUDASequenceTrainingStats getCUDASequenceTrainingStats() const;
    
    /**
     * @brief 获取GPU信息
     * @return GPU设备信息字符串
     */
    std::string getGPUInfo() const;
    
    /**
     * @brief 检查CUDA状态
     * @return CUDA状态是否正常
     */
    bool checkCUDAStatus() const;

private:
    /**
     * @brief CUDA LSTM序列模型类
     * 实现基于CUDA加速的LSTM序列神经网络
     */
    class CUDALSTMSequenceModel {
    public:
        /**
         * @brief 构造函数
         * @param config CUDA序列训练配置
         */
        CUDALSTMSequenceModel(const CUDASequenceTrainingConfig& config);
        
        /**
         * @brief 析构函数
         */
        ~CUDALSTMSequenceModel();
        
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
        
        /**
         * @brief 获取GPU内存使用情况
         * @return 内存使用量(MB)
         */
        float getGPUMemoryUsage() const;
        
    private:
        CUDASequenceTrainingConfig config;
        
        // CUDA设备句柄
        cudaStream_t stream;
        cublasHandle_t cublasHandle;
        cudnnHandle_t cudnnHandle;
        
        // CUDA内存指针
        float* d_lstm1Weights;      ///< 第一层LSTM权重(GPU)
        float* d_lstm1Biases;     ///< 第一层LSTM偏置(GPU)
        float* d_lstm2Weights;      ///< 第二层LSTM权重(GPU)
        float* d_lstm2Biases;     ///< 第二层LSTM偏置(GPU)
        float* d_denseWeights;      ///< 全连接层权重(GPU)
        float* d_denseBiases;       ///< 全连接层偏置(GPU)
        
        // LSTM状态(GPU)
        float* d_hiddenState1;     ///< 第一层LSTM隐藏状态
        float* d_cellState1;       ///< 第一层LSTM细胞状态
        float* d_hiddenState2;     ///< 第二层LSTM隐藏状态
        float* d_cellState2;       ///< 第二层LSTM细胞状态
        
        // 临时缓冲区
        float* d_inputBuffer;      ///< 输入缓冲区
        float* d_outputBuffer;     ///< 输出缓冲区
        float* d_targetBuffer;     ///< 目标缓冲区
        float* d_gradientBuffer;   ///< 梯度缓冲区
        
        // cuDNN描述符
        cudnnTensorDescriptor_t inputDesc;
        cudnnTensorDescriptor_t outputDesc;
        cudnnTensorDescriptor_t hiddenDesc;
        cudnnRNNDescriptor_t rnnDesc;
        
        std::mt19937 rng;                                ///< 随机数生成器
        
        /**
         * @brief 初始化CUDA设备
         */
        void initializeCUDA();
        
        /**
         * @brief 初始化CUDA内存
         */
        void initializeCUDAMemory();
        
        /**
         * @brief 初始化cuDNN
         */
        void initializeCuDNN();
        
        /**
         * @brief 释放CUDA资源
         */
        void releaseCUDAResources();
        
        /**
         * @brief CUDA LSTM前向传播
         * @param input 输入序列
         * @return 输出
         */
        std::vector<float> cudaLSTMForward(const std::vector<std::vector<float>>& input);
        
        /**
         * @brief CUDA LSTM反向传播
         * @param input 输入序列
         * @param targets 目标值
         * @param learningRate 学习率
         * @return 损失值
         */
        float cudaLSTMBackward(const std::vector<std::vector<float>>& input,
                              const std::vector<float>& targets,
                              float learningRate);
        
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
        
        /**
         * @brief 检查CUDA错误
         * @param error CUDA错误代码
         * @param message 错误消息
         */
        void checkCUDAError(cudaError_t error, const std::string& message) const;
        
        /**
         * @brief 检查cuBLAS错误
         * @param error cuBLAS错误代码
         * @param message 错误消息
         */
        void checkCUBLASError(cublasStatus_t error, const std::string& message) const;
        
        /**
         * @brief 检查cuDNN错误
         * @param error cuDNN错误代码
         * @param message 错误消息
         */
        void checkCUDNNError(cudnnStatus_t error, const std::string& message) const;
    };

private:
    CUDASequenceTrainingConfig config;                          ///< CUDA序列训练配置
    std::unique_ptr<CUDALSTMSequenceModel> cudaSequenceModel;    ///< CUDA序列模型实例
    CUDASequenceTrainingStats stats;                             ///< CUDA序列训练统计信息
    
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
    
    /**
     * @brief 更新GPU内存使用统计
     */
    void updateGPUMemoryStats();
};