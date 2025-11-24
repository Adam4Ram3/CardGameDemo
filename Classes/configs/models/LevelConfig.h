#ifndef LEVEL_CONFIG_H
#define LEVEL_CONFIG_H

#include "configs/GameConsts.h"
#include "cocos2d.h"
#include <vector>

struct CardConfigData {
    // 【修正】使用新的枚举值 CFT_NONE 和 CST_NONE
    CardFaceType face = CardFaceType::CFT_NONE;
    CardSuitType suit = CardSuitType::CST_NONE;
    cocos2d::Vec2 position = cocos2d::Vec2::ZERO;
};

struct LevelConfig {
    std::vector<CardConfigData> playFieldCards;
    std::vector<CardConfigData> stackCards;
};

#endif // LEVEL_CONFIG_H