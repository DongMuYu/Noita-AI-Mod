#include "DataCollector.h"
#include <iomanip>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <SFML/Window/Keyboard.hpp>
#include "../../entity/Player.h"
#include "../../core/Map.h"
#include "../pathfinding/RayCasting.h"


// 构造函数，初始化数据收集器
DataCollector::DataCollector() : 
    currentEpisode(nullptr),
    recordingEnabled(true),
    episodeLimit(10000),
    nextEpisodeId(1)
{
}

// 析构函数，清理当前episode数据
DataCollector::~DataCollector() {
    if (currentEpisode) {
        delete currentEpisode;
    }
}

// 开始新的一局游戏记录
void DataCollector::startEpisode() {
    if (!recordingEnabled) {
        std::cout << "[DEBUG] Data collection disabled, skipping episode start" << std::endl;
        return;
    }
    
    if (currentEpisode) {
        std::cout << "[DEBUG] Warning: Previous episode not properly ended, cleaning up" << std::endl;
        delete currentEpisode;
    }
    
    currentEpisode = new EpisodeData();
    currentEpisode->episodeId = nextEpisodeId++;
    currentEpisode->startTime = std::chrono::steady_clock::now();
    currentEpisode->success = false;
    currentEpisode->steps = 0;
    currentEpisode->frames.clear();
    
    std::cout << "[DEBUG] Episode " << currentEpisode->episodeId << " started";
    std::cout << std::endl;
}


// 获取当前帧数据
DataCollector::TrainingData DataCollector::getCurrentFrameData(Player& player, 
                                                           Map& map, 
                                                           RayCasting& rayCaster) {
    DataCollector::TrainingData frame;
    
    // 收集玩家和环境的当前状态
    DataCollector::AIState state;

    state.position = player.getPosition();
    state.velocity = player.getVelocity();
    state.energy = player.getCurrentEnergy() / player.getMaxEnergy();
    state.target = map.getTargetPosition();
    
    // 获取射线检测结果 - 检测周围障碍物 - 共60条射线的信息
    sf::Vector2f playerCenter = player.getPosition() + sf::Vector2f(player.getWidth() / 2, player.getHeight() / 2);
    auto rayHits = rayCaster.castRays(playerCenter, map.getLevelData());
    state.rayDistances.clear();
    state.rayHits.clear();
    for (const auto& hit : rayHits) {
        state.rayDistances.push_back(hit.distance);
        state.rayHits.push_back(hit.hit ? 1.0f : 0.0f);
    }
    
    // 计算到目标的距离和角度 - 用于导航
    sf::Vector2f diff = state.target - state.position;
    state.distanceToTarget = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    state.angleToTarget = std::atan2(diff.y, diff.x);
    state.isGrounded = player.isOnGround();
    
    // 获取玩家真实的键盘输入作为训练标签 - 监督学习
    DataCollector::Action action;
    
    // 检测真实的按键输入
    bool leftPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    bool rightPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
    bool flyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    
    // 设置真实的动作标签 - 将键盘输入转换为动作
    if (leftPressed && !rightPressed) {
        action.moveX = -1.0f;  // 左移
    } else if (rightPressed && !leftPressed) {
        action.moveX = 1.0f;   // 右移
    } else {
        action.moveX = 0.0f;   // 不动
    }
    
    action.useEnergy = flyPressed && player.getCurrentEnergy() > 0 ? 1.0f : 0.0f;  // 飞行状态：1=飞，0=不飞
    
    frame.state = state;
    frame.action = action;
    frame.terminal = false;
    
    return frame;
}

// 记录当前帧数据
void DataCollector::recordCurrentFrame(const DataCollector::TrainingData& frame) {
    if (!recordingEnabled || !currentEpisode) {
        if (!recordingEnabled) {
            std::cout << "[DEBUG] Recording disabled, skipping frame" << std::endl;
        } else {
            std::cout << "[DEBUG] Warning: No active episode for frame recording" << std::endl;
        }
        return;
    }
    
    currentEpisode->frames.push_back(frame);
    currentEpisode->steps++;
    
    if (currentEpisode->steps % 100 == 0) {
        std::cout << "[DEBUG] Recorded frame " << currentEpisode->steps 
                << " in episode " << currentEpisode->episodeId << std::endl;
    }
}

// 结束当前局游戏记录
void DataCollector::endEpisode(bool success, float gameDuration, float averageFPS) {
    if (!recordingEnabled || !currentEpisode) {
        std::cout << "[DEBUG] Warning: Cannot end episode - recording disabled or no active episode" << std::endl;
        return;
    }
    
    currentEpisode->endTime = std::chrono::steady_clock::now();
    currentEpisode->success = success;
    currentEpisode->gameDuration = gameDuration;
    currentEpisode->averageFPS = averageFPS;
    
    // 标记最后一帧为终止状态 - 用于训练时识别episode结束
    if (!currentEpisode->frames.empty()) {
        currentEpisode->frames.back().terminal = true;
    }
    
    episodes.push_back(*currentEpisode);
    
    std::cout << "[DEBUG] Episode " << currentEpisode->episodeId << " ended" << std::endl;
    std::cout << "[DEBUG] Success: " << (success ? "true" : "false") 
            << ", Steps: " << currentEpisode->steps; 
    
    // 限制存储的episode数量 - 防止内存溢出
    if (episodes.size() > static_cast<size_t>(episodeLimit)) {
        std::cout << "[DEBUG] Episode limit reached, removing oldest episode" << std::endl;
        episodes.erase(episodes.begin());
    }
    
    // 清理当前episode - 准备下一局游戏
    delete currentEpisode;
    currentEpisode = nullptr;
    
    std::cout << "[DEBUG] Total episodes stored: " << episodes.size() << std::endl;
}

// 保存所有局数据到文件
void DataCollector::saveEpisodeData(const std::string& filename) {
    // 构建新的文件路径，将数据保存到sequence_data子文件夹
    std::string newFilename = "D:\\steam\\steamapps\\common\\Noita\\mods\\NoitaCoreAI\\aiDev\\data\\sequence_data\\" + std::filesystem::path(filename).filename().string();
    std::cout << "[DEBUG] Saving episode data to: " << newFilename << std::endl;
    
    // 确保目录存在 - 创建必要的文件夹
    std::filesystem::create_directories(std::filesystem::path(newFilename).parent_path());
    
    // 使用追加模式打开文件，避免覆盖原有数据 - 增量保存
    std::ofstream file(newFilename, std::ios::app);
    if (!file.is_open()) {
          std::cerr << "[ERROR] Failed to open file: " << newFilename << std::endl;
          std::cerr << "[ERROR] Current working directory: " << std::filesystem::current_path() << std::endl;
          std::cerr << "[ERROR] Please check if directory exists and has write permissions" << std::endl;
          return;
      }
    
    // 获取已保存的游戏局数（用于跳过已保存的数据） - 避免重复保存
    int startEpisodeId = 0;
    std::ifstream checkFile(newFilename);
    if (checkFile.is_open()) {
        std::string line;
        while (std::getline(checkFile, line)) {
            if (line.find("Episode: ") != std::string::npos) {
                try {
                    int episodeId = std::stoi(line.substr(9));
                    startEpisodeId = std::max(startEpisodeId, episodeId + 1);
                } catch (...) {}
            }
        }
        checkFile.close();
    }
    
    std::cout << "[DEBUG] Starting from episode ID: " << startEpisodeId << std::endl;
    
    // 只保存新增的游戏局 - 提高保存效率
    int newEpisodes = 0;
    for (const auto& episode : episodes) {
        if (episode.episodeId >= startEpisodeId) {
            file << "Episode: " << episode.episodeId << "\n";
            file << "Success: " << (episode.success ? "1" : "0") << "\n";
            file << "Steps: " << episode.steps << "\n";
            
            // 计算持续时间
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                episode.endTime - episode.startTime);
            file << "Duration: " << duration.count() << "\n";
            file << "GameDuration: " << episode.gameDuration << "\n";
            file << "AverageFPS: " << episode.averageFPS << "\n";
            
            file << "Frames: " << episode.frames.size() << "\n";
            
            // 保存每帧数据
            for (const auto& frame : episode.frames) {
                file << "F:";
                
                // 状态特征
                const auto& s = frame.state;
                file << s.position.x << "," << s.position.y << ","
                    << s.velocity.x << "," << s.velocity.y << ","
                    << s.energy << "," << s.distanceToTarget << ","
                    << s.angleToTarget << ",";
                
                // 射线距离（取前8个）
                for (size_t i = 0; i < 8 && i < s.rayDistances.size(); ++i) {
                    file << s.rayDistances[i] << (i < 7 ? "," : "");
                }
                
                file << ";";
                
                // 动作
                file << frame.action.moveX << "," << frame.action.useEnergy << ","
                    << (frame.action.useEnergy ? "1" : "0") << ";";
                
                // 终止状态
                file << (frame.terminal ? "1" : "0") << "\n";
            }
            
            file << "\n";
            newEpisodes++;
        }
    }
    
    file.close();
    
    std::cout << "[DEBUG] Appended " << newEpisodes << " new episodes to file" << std::endl;
}

// 从文件加载数据
void DataCollector::loadEpisodeData(const std::string& filename) {
    // 构建新的文件路径，从sequence_data子文件夹加载数据
    std::string newFilename = "D:\\steam\\steamapps\\common\\Noita\\mods\\NoitaCoreAI\\aiDev\\data\\sequence_data\\" + std::filesystem::path(filename).filename().string();
    std::ifstream file(newFilename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << newFilename << std::endl;
        return;
    }
    
    std::string line;
    if (!std::getline(file, line)) {
        std::cout << "DataCollector: File is empty, starting fresh" << std::endl;
        file.close();
        return;
    }
    
    if (line.find("F:") != std::string::npos) {
        std::cout << "DataCollector: Detected old format file (frame data only), skipping load" << std::endl;
        std::cout << "DataCollector: Old format data will be preserved but not loaded" << std::endl;
        file.close();
        return;
    }
    
    file.clear();
    file.seekg(0);
    
    bool isNewFormat = false;
    while (std::getline(file, line)) {
        if (line.find("Episode:") != std::string::npos) {
            isNewFormat = true;
            break;
        }
    }
    
    if (!isNewFormat) {
        std::cout << "DataCollector: No compatible format found, skipping load" << std::endl;
        file.close();
        return;
    }
    
    file.clear();
    file.seekg(0);
    
    std::cout << "[DEBUG] Loading data from: " << newFilename << std::endl;
    
    int loadedEpisodes = 0;
    int skippedEpisodes = 0;
    
    int maxExistingId = 0;
    if (!episodes.empty()) {
        maxExistingId = episodes.back().episodeId;
    }
    
    file.clear();
    file.seekg(0);
    
    try {
        while (std::getline(file, line)) {
            if (line.find("Episode:") != std::string::npos) {
                EpisodeData episode;
                
                try {
                    int episodeId = std::stoi(line.substr(8));
                    episode.episodeId = episodeId;
                    
                    if (episodeId <= maxExistingId) {
                        skippedEpisodes++;
                        
                        while (std::getline(file, line)) {
                            if (line.empty()) break;
                        }
                        continue;
                    }
                    
                    if (!std::getline(file, line)) break;
                    episode.success = (line.substr(8) == "1");
                    
                    if (!std::getline(file, line)) break;
                    episode.steps = std::stoi(line.substr(6));
                    
                    if (!std::getline(file, line)) break;
                    episode.gameDuration = std::stof(line.substr(13));
                    
                    if (!std::getline(file, line)) break;
                    episode.averageFPS = std::stof(line.substr(11));
                    
                    if (!std::getline(file, line)) break;
                    int frameCount = std::stoi(line.substr(7));
                    
                    for (int i = 0; i < frameCount && std::getline(file, line); ++i) {
                    }
                    
                    std::getline(file, line);
                    
                    episodes.push_back(episode);
                    loadedEpisodes++;
                    
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing episode " << loadedEpisodes << ": " << e.what() << std::endl;
                    continue;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading data: " << e.what() << std::endl;
    }
    
    file.close();
    
    if (!episodes.empty()) {
        nextEpisodeId = episodes.back().episodeId + 1;
    }
    
    std::cout << "DataCollector: Loaded " << loadedEpisodes << " new episodes, skipped " << skippedEpisodes << " existing episodes" << std::endl;
    std::cout << "DataCollector: Total episodes now: " << episodes.size() << std::endl;
}

// 导出训练数据集为CSV格式
void DataCollector::exportTrainingDataset(const std::string& filename) {
    // 构建新的文件路径，将数据导出到sequence_data子文件夹
    std::string newFilename = "D:\\steam\\steamapps\\common\\Noita\\mods\\NoitaCoreAI\\aiDev\\data\\sequence_data\\" + std::filesystem::path(filename).filename().string();
    std::cout << "[DEBUG] Exporting training dataset to: " << newFilename << std::endl;
    
    std::filesystem::create_directories(std::filesystem::path(newFilename).parent_path());
    
    std::filesystem::path filePath(newFilename);
    bool fileExists = std::filesystem::exists(filePath);
    
    // 打开文件，存在则追加，不存在则创建
    std::ofstream file(filePath, fileExists ? std::ios::app : std::ios::out);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open file: " << newFilename << std::endl;
        std::cerr << "[ERROR] Current working directory: " << std::filesystem::current_path() << std::endl;
        std::cerr << "[ERROR] Please check if directory exists and has write permissions" << std::endl;
        return;
    }
    
    // 只有创建新文件时才写入表头
    if (!fileExists) {
        file << "pos_x,pos_y,vel_x,vel_y,energy,target_x,target_y,dist_target,angle_target,is_grounded";
        
        for (int i = 0; i < 60; ++i) {
            file << ",ray_dist_" << i;
        }
        
        for (int i = 0; i < 60; ++i) {
            file << ",ray_hit_" << i;
        }
        file << ",action_x,use_energy" << std::endl;
    }
    
    for (const auto& episode : episodes) {
        for (const auto& frame : episode.frames) {
            const auto& s = frame.state;
            
            file << s.position.x << "," << s.position.y << ","
                << s.velocity.x << "," << s.velocity.y << ","
                << s.energy << ","
                << s.target.x << "," << s.target.y << ","
                << s.distanceToTarget << "," << s.angleToTarget << ","
                << (s.isGrounded ? 1 : 0);
            
            for (int i = 0; i < 60; ++i) {
                if (i < s.rayDistances.size()) {
                    file << "," << s.rayDistances[i];
                } else {
                    file << ",0.0";
                }
            }
            
            for (int i = 0; i < 60; ++i) {
                if (i < s.rayHits.size()) {
                    file << "," << s.rayHits[i];
                } else {
                    file << ",0.0";
                }
            }
            
            file << "," << frame.action.moveX << "," << (frame.action.useEnergy ? 1 : 0) << "\n";
        }
    }
    
    file.close();
    std::cout << "[DEBUG] Exported " << episodes.size() << " episodes" << std::endl;
}

// 获取总局数
int DataCollector::getTotalEpisodes() const {
    return episodes.size();
}

// 获取成功局数
int DataCollector::getSuccessfulEpisodes() const {
    int count = 0;
    for (const auto& episode : episodes) {
        if (episode.success) count++;
    }
    return count;
}

// 获取平均步数
float DataCollector::getAverageSteps() const {
    if (episodes.empty()) return 0.0f;
    
    int totalSteps = 0;
    for (const auto& episode : episodes) {
        totalSteps += episode.steps;
    }
    return static_cast<float>(totalSteps) / episodes.size();
}


// 获取成功率
float DataCollector::getSuccessRate() const {
    if (episodes.empty()) return 0.0f;
    return static_cast<float>(getSuccessfulEpisodes()) / episodes.size();
}


// 清除所有数据
void DataCollector::clearAllData() {

    if (currentEpisode) {
        delete currentEpisode;
        currentEpisode = nullptr;
    }
    

    episodes.clear();
    

    nextEpisodeId = 0;
    
    std::cout << "DataCollector: All data cleared. Ready for new training session." << std::endl;
}

// 设置是否启用记录
void DataCollector::setRecordingEnabled(bool enabled) {
    recordingEnabled = enabled;
    std::cout << "DataCollector: Recording " << (enabled ? "enabled" : "disabled") << std::endl;
}

// 检查是否启用记录
bool DataCollector::isRecordingEnabled() const {
    return recordingEnabled;
}

// 设置最大存储局数限制
void DataCollector::setEpisodeLimit(int limit) {
    episodeLimit = limit;
    std::cout << "DataCollector: Episode limit set to " << limit << " (0 = unlimited)" << std::endl;
}

// 获取所有训练数据
std::vector<DataCollector::TrainingData> DataCollector::getTrainingData() const {
    std::vector<DataCollector::TrainingData> trainingData;
    
    for (const auto& episode : episodes) {
        for (const auto& frame : episode.frames) {
            trainingData.push_back(frame);
        }
    }
    
    return trainingData;
}

// 清除训练数据
void DataCollector::clearTrainingData() {
    clearAllData();
}