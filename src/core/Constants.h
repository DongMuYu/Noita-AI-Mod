// Constants.h

#ifndef CONSTANTS_H
#define CONSTANTS_H

/**
 * @file Constants.h
 * @brief 游戏核心常量定义
 * @details 包含游戏中使用的物理参数、尺寸定义和运动速度等全局常量
 * 所有游戏系统共享这些基础参数，修改会影响整体游戏体验和平衡性
 */

/**
 * @brief 瓦片尺寸（像素）
 * @details 游戏世界中单个瓦片的宽度和高度，用于地图渲染和碰撞检测
 * 选择15像素作为基础单位，平衡了细节表现和性能需求
 * 瓦片尺寸直接影响：
 * - 游戏世界物理尺度
 * - 碰撞检测精度
 * - 视觉元素大小比例
 */
constexpr int   TILE = 15;

/**
 * @brief 游戏世界宽度（瓦片数量）
 * @details 定义游戏世界水平方向的瓦片数量
 * 90瓦片 × 15像素/瓦片 = 1350像素的世界宽度
 * 该尺寸设计为：
 * - 提供足够的探索空间
 * - 在主流显示器上无需频繁滚动
 * - 保持AI路径计算的性能平衡
 */
constexpr int   W = 90;

/**
 * @brief 游戏世界高度（瓦片数量）
 * @details 定义游戏世界垂直方向的瓦片数量
 * 90瓦片 × 15像素/瓦片 = 1350像素的世界高度
 * 垂直空间设计考虑了：
 * - 跳跃高度与下落时间的平衡
 * - 垂直探索可能性
 * - 与水平空间的比例协调
 */
constexpr int   H = 90;

/**
 * @brief 重力加速度（像素/秒²）
 * @details 应用于所有实体的重力大小，影响下落速度和跳跃曲线
 * 1350像素/秒²的重力值经过多次测试，提供：
 * - 自然的下落感受
 * - 合理的跳跃高度（约1.8米现实比例）
 * - 适当的空中控制时间窗口
 * 计算公式：最终速度 = 初始速度 + 重力 × 时间
 */
constexpr float GRAVITY = 1350.f;

/**
 * @brief 跳跃初速度（像素/秒）
 * @details 玩家跳跃时的初始垂直速度，与重力共同决定跳跃高度
 * 500像素/秒的初速度配合1350重力，产生约：
 * - 0.74秒的跳跃上升时间
 * - 140像素的最大跳跃高度（约9.3个瓦片）
 * - 符合平台游戏的跳跃手感
 */
constexpr float JUMP_VELOCITY  = 400.f;

/**
 * @brief 移动速度（像素/秒）
 * @details 玩家水平移动的最大速度，决定角色的敏捷度
 * 150像素/秒的速度设计为：
 * - 约10个瓦片/秒的移动效率
 * - 与游戏世界尺寸相匹配的探索节奏
 * - 兼顾精确控制和快速移动的平衡
 * 与加速度和摩擦力配合产生平滑的移动体验
 */
constexpr float MOVE_SPEED = 160.f;

/**
 * @brief 寻路节点间距（像素）
 * @details A*寻路算法中节点之间的最小间距，影响路径精度和计算效率
 * 15像素的间距与瓦片尺寸匹配，确保路径点落在网格中心
 */
constexpr float NODE_SPACING = 15.0f;

/**
 * @brief AI模型文件路径
 * @details 当前使用的AI模型文件路径，便于统一管理和修改
 * 使用中间版本模型进行测试和开发
 */
constexpr const char* AI_MODEL_PATH = "d:/steam/steamapps/common/Noita/mods/NoitaCoreAI/aiDev/models/SL_models/intermediate_model_epoch_20.bin";

#endif // CONSTANTS_H