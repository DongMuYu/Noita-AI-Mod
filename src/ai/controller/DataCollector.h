#pragma once

#include "AIController.h"
#include "../pathfinding/RayCasting.h"
#include <vector>
#include <string>
#include <chrono>
#include <fstream>


// 数据收集器类 - 用于收集和管理AI训练数据
class DataCollector {
public:

    struct AIState {
        sf::Vector2f position;
        sf::Vector2f velocity;
        sf::Vector2f target;
        std::vector<float> rayDistances;
        std::vector<int> rayHits;
        float energy;
        float distanceToTarget;
        float angleToTarget;
        bool isGrounded;
    };

    struct Action {
        int moveX;
        int useEnergy;
    };

    struct TrainingData {
        AIState state;
        Action action;
        bool terminal;
    };
    
    // 序列训练数据结构
    struct SequenceTrainingData {
        std::vector<TrainingData> sequence;  // 150帧序列
        TrainingData target;                 // 目标动作
        int sequenceLength = 150;
    };

    // 单局游戏数据记录结构体
    struct EpisodeData {
        int episodeId;
        std::chrono::steady_clock::time_point startTime;
        std::chrono::steady_clock::time_point endTime;
        bool success;
        int steps;
        float gameDuration;
        float averageFPS;
        std::vector<TrainingData> frames;

    };

public:

    // 构造函数
    DataCollector();
    
    // 析构函数
    ~DataCollector();

    // 开始新的一局游戏记录
    void startEpisode();
    
    // 获取当前帧数据
    DataCollector::TrainingData getCurrentFrameData(Player& player, 
                                                   Map& map, 
                                                   RayCasting& rayCaster);
    
    // 记录当前帧数据
    void recordCurrentFrame(const DataCollector::TrainingData& frame);
    
    // 结束当前局游戏记录
    void endEpisode(bool success, float gameDuration = 0.0f, float averageFPS = 0.0f);
    
    // 保存所有局数据到文件
    void saveEpisodeData(const std::string& filename);
    
    // 从文件加载数据
    void loadEpisodeData(const std::string& filename);
    
    // 导出训练数据集为CSV格式
    void exportTrainingDataset(const std::string& filename);
    
    // 获取总局数
    int getTotalEpisodes() const;
    
    // 获取成功局数
    int getSuccessfulEpisodes() const;
    
    // 获取平均步数
    float getAverageSteps() const;
    
    // 获取成功率
    float getSuccessRate() const;
    
    // 清除所有数据
    void clearAllData();
    
    // 设置是否启用记录
    void setRecordingEnabled(bool enabled);
    
    // 检查是否启用记录
    bool isRecordingEnabled() const;
    
    // 设置最大存储局数限制
    void setEpisodeLimit(int limit);
    
    // 获取所有训练数据
    std::vector<DataCollector::TrainingData> getTrainingData() const;
    
    // 清除训练数据
    void clearTrainingData();

private:
    std::vector<EpisodeData> episodes;
    EpisodeData* currentEpisode;
    bool recordingEnabled;
    int episodeLimit;
    int nextEpisodeId;
    

    void saveEpisodeToFile(const EpisodeData& episode, const std::string& filename);
    

    EpisodeData loadEpisodeFromFile(const std::string& filename);
};