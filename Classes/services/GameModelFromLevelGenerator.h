#ifndef GAME_MODEL_FROM_LEVEL_GENERATOR_H
#define GAME_MODEL_FROM_LEVEL_GENERATOR_H

#include "configs/models/LevelConfig.h"
#include "models/GameModel.h"
#include <memory>

/**
 * @brief 模型生成服务
 * 职责：将静态的关卡配置转换为运行时的动态游戏数据
 * 特性：无状态服务 (Stateless Service)
 */
class GameModelFromLevelGenerator {
public:
    /**
     * 根据配置生成游戏模型
     * @param config 静态配置
     * @return 包含所有初始数据的 GameModel 指针
     */
    static std::shared_ptr<GameModel> generateGameModel(const LevelConfig& config);
};

#endif // GAME_MODEL_FROM_LEVEL_GENERATOR_H