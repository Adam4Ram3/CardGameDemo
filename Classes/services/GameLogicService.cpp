// Classes/services/GameLogicService.cpp

#include "services/GameLogicService.h"
#include <cmath>

bool GameLogicService::canMatch(const CardModel* handCard, const CardModel* fieldCard) {
    if (!handCard || !fieldCard) return false;

    // 获取两个牌的点数 (int)
    int faceA = (int)handCard->getFace();
    int faceB = (int)fieldCard->getFace();

    // 规则 1: 常规匹配
    // 点数相差 1 (例如: 3和4，9和10)
    int diff = std::abs(faceA - faceB);
    if (diff == 1) {
        return true;
    }

    // 规则 2: K 和 A 循环匹配 (新增逻辑)
    // 在我们现在的 0-based 定义中：Ace是0，King是12。
    // 所以只要判断一张是 Ace(0) 且另一张是 King(12) 即可。
    if ((faceA == (int)CardFaceType::CFT_ACE && faceB == (int)CardFaceType::CFT_KING) ||
        (faceA == (int)CardFaceType::CFT_KING && faceB == (int)CardFaceType::CFT_ACE)) {
        return true;
    }

    return false;
}

void GameLogicService::applyMove(CardModel* card, const cocos2d::Vec2& targetPos, int newZIndex) {
    if (card) {
        card->setPosition(targetPos);
        card->setZIndex(newZIndex);
    }
}

void GameLogicService::applyStateChange(CardModel* card, CardState newState) {
    if (card) {
        card->setState(newState);
    }
}