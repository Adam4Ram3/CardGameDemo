#ifndef GAME_MODEL_H
#define GAME_MODEL_H

#include "models/CardModel.h"
#include <vector>
#include <memory> // 使用智能指针管理 Model 生命周期

class GameModel {
public:
    // **运行时卡牌池**
    // 集合中维护当前关卡所有处于游戏中的卡牌实例，外部可以通过 shared_ptr 获取引用并操作状态。
    std::vector<std::shared_ptr<CardModel>> allCards;

    // **按 ID 查找卡牌**
    // @param id 要查找的卡牌唯一标识
    // @return 如果找到返回该卡牌的 shared_ptr，否则返回 nullptr
    std::shared_ptr<CardModel> getCardById(int id) {
        for (auto& card : allCards) {
            if (card->getId() == id) return card;
        }
        return nullptr;
    }

    // **注册新卡牌**
    // 将生成的 CardModel 实例添加到主数据集合中，供所有系统共享使用。
    void addCard(std::shared_ptr<CardModel> card) {
        allCards.push_back(card);
    }

    // **清空当前关卡数据**
    // 通常在重新加载关卡或退出游戏时调用，确保没有悬挂引用。
    void clear() {
        allCards.clear();
    }
};

#endif // GAME_MODEL_H