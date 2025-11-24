#ifndef UNDO_MODEL_H
#define UNDO_MODEL_H

#include "cocos2d.h"
#include "configs/GameConsts.h"

/**
 * @brief 回退命令结构体 (UndoCommand)
 * 职责：记录一次操作发生前的所有状态数据，用于恢复（回退）
 */
struct UndoCommand {
    int cardId;                 // 被操作的卡牌ID
    cocos2d::Vec2 fromPos;      // 移动前的位置
    int prevTopCardId;          // 操作前，堆牌区顶部的卡牌ID (用于恢复 StackController 的状态)
    CardState prevState;        // 操作前的状态 (比如在备用堆是背面的)
    int prevZIndex;             // 操作前的层级

    // 默认构造函数
    UndoCommand() 
        : cardId(-1), fromPos(cocos2d::Vec2::ZERO), prevTopCardId(-1), prevState(CardState::FACE_DOWN), prevZIndex(0) {}

    // 带参构造函数 (方便快速赋值)
    UndoCommand(int id, cocos2d::Vec2 pos, int topId, CardState state, int z)
        : cardId(id), fromPos(pos), prevTopCardId(topId), prevState(state), prevZIndex(z) {
    }
};

#endif // UNDO_MODEL_H