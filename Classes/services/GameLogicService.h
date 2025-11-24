// Classes/services/GameLogicService.h

#ifndef GAME_LOGIC_SERVICE_H
#define GAME_LOGIC_SERVICE_H

#include "models/CardModel.h"

class GameLogicService {
public:
    // [只读逻辑] 判断是否匹配
    static bool canMatch(const CardModel* handCard, const CardModel* fieldCard);

    // [写逻辑] 执行卡牌移动的数据变更
    // 纯粹的数据操作：修改位置、修改层级
    static void applyMove(CardModel* card, const cocos2d::Vec2& targetPos, int newZIndex);

    // [写逻辑] 执行卡牌状态变更
    // 纯粹的数据操作：翻面
    static void applyStateChange(CardModel* card, CardState newState);
};

#endif // GAME_LOGIC_SERVICE_H