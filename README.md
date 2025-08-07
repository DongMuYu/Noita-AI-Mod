# NoitaCoreAI

一个为Noita游戏开发的AI系统，包含监督学习训练功能和智能决策模块。(还未完工，正在进行中)

## 项目概述

NoitaCoreAI是一个基于C++的AI项目，专为Noita游戏设计。该项目实现了以下核心功能：

- **游戏核心系统**：包含游戏循环、模拟生成地下环境、物理引擎、碰撞检测、渲染系统
- **AI控制器**：智能决策系统，支持监督学习训练
- **数据收集与处理**：自动收集游戏数据并生成训练数据集
- **模型训练**：支持行为克隆的监督学习训练
- **优化机制**：包含早停、中间模型保存等训练优化功能

## 系统要求

- **操作系统**：Windows 10或更高版本
- **编译器**：MSVC (Visual Studio 2019或更高版本)
- **CMake**：3.16或更高版本
- **依赖库**：
  - SFML (图形渲染)
  - PyTorch C++ API (深度学习)(还未使用)
  - 其他第三方库 (见CMake配置)

## 安装说明

### 1. 克隆项目

```bash
git clone <repository-url>
cd NoitaCoreAI/aiDev
```

### 2. 安装依赖

确保已安装以下依赖：

- **Visual Studio 2019/2022**：包含C++开发工具
- **CMake**：从官网下载并安装
- **PyTorch**：安装PyTorch C++库

### 3. 构建项目

#### 构建主程序

```bash
build_main.bat
```

#### 构建训练器

```bash
build_trainer.bat
```

#### 运行训练器

```bash
run_trainer.bat
```

## 使用方法

### 运行游戏

```bash
# 运行主游戏程序
bin\aiDev.exe
```

### 训练AI模型

```bash
# 运行训练器
run_trainer.bat
```

训练器将自动：
1. 检查训练数据文件
2. 加载现有模型（如果存在）
3. 使用优化后的参数进行训练
4. 定期保存中间模型
5. 保存最终模型

## 项目结构

```
aiDev/
├── bin/                    # 编译输出的可执行文件
│   ├── aiDev.exe          # 主游戏程序
│   └── sl_trainer.exe     # 监督学习训练器
├── build_training/         # 训练器构建目录
├── data/                   # 数据文件
│   ├── collected_data.bin # 原始收集的游戏数据
│   ├── training_dataset.csv     # 导出的训练数据集
│   └── training_dataset_reduced.csv # 优化后的训练数据集
├── models/                 # 训练模型
│   └── SL_models/          # 监督学习模型
├── src/                    # 源代码
│   ├── ai/                 # AI相关模块
│   │   ├── controller/     # AI控制器
│   │   ├── decision/       # 决策系统
│   │   ├── pathfinding/    # 路径寻找
│   │   └── trainer/        # 训练器
│   ├── core/               # 核心游戏系统
│   ├── entity/             # 游戏实体
│   ├── physics/            # 物理引擎
│   ├── scene/              # 场景管理
│   └── util/               # 工具类
├── scripts/                # 脚本文件
├── logs/                   # 日志文件
└── docs/                   # 文档
```

## 数据处理流程

### 1. 数据收集

游戏运行时会自动收集数据并保存到`data/collected_data.bin`。

### 2. 数据导出

使用DataCollector将二进制数据导出为CSV格式：

```cpp
// 数据会自动导出到 data/training_dataset.csv
```

### 3. 数据优化

运行数据优化脚本减少冗余数据：

```bash
# 需要Python环境
python reduce_zero_actions.py
```

这将生成优化后的`training_dataset_reduced.csv`文件。

### 4. 模型训练

使用优化后的数据集训练模型：

```bash
run_trainer.bat
```

## 训练优化特性

### 自动优化功能

- **早停机制**：当验证损失不再改善时自动停止训练
- **中间模型保存**：每20个epoch保存一次中间模型
- **动态学习率**：优化的学习率设置
- **批量处理优化**：增大的批次大小提高训练效率

### 训练参数

```cpp
config.epochs = 50;                    // 训练轮数
config.batchSize = 256;                // 批次大小
config.learningRate = 0.002f;          // 学习率
config.validationSplit = 0.2f;         // 验证集比例
config.earlyStoppingPatience = 10;    // 早停耐心值
```

## 文件说明

### 核心文件

- **src/core/Game.cpp**：游戏主循环，包含游戏初始化、输入处理、物理更新、碰撞检测等
- **src/ai/trainer/train_sl.cpp**：监督学习训练主程序
- **src/ai/trainer/SLTrainer/SLTrainer.cpp**：训练器核心实现

### 配置文件

- **build_trainer.bat**：训练器构建脚本
- **run_trainer.bat**：训练器运行脚本
- **scripts/config.json**：项目配置文件

### 数据处理

- **reduce_zero_actions.py**：数据优化脚本，减少连续零动作序列
- **src/core/DataCollector.cpp**：数据收集和导出功能





