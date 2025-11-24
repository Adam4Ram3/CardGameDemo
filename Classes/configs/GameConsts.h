#ifndef GAME_CONSTS_H
#define GAME_CONSTS_H

// 花色类型
enum class CardSuitType
{
    CST_NONE = -1,
    CST_CLUBS = 0,      // 梅花
    CST_DIAMONDS = 1,   // 方块
    CST_HEARTS = 2,     // 红桃
    CST_SPADES = 3,     // 黑桃
    CST_NUM_CARD_SUIT_TYPES
};

// 点数类型 (0-based)
// Ace=0, Two=1, ..., Ten=9, Jack=10, Queen=11, King=12
enum class CardFaceType
{
    CFT_NONE = -1,
    CFT_ACE = 0,
    CFT_TWO = 1,
    CFT_THREE = 2,
    CFT_FOUR = 3,
    CFT_FIVE = 4,
    CFT_SIX = 5,
    CFT_SEVEN = 6,
    CFT_EIGHT = 7,
    CFT_NINE = 8,
    CFT_TEN = 9,
    CFT_JACK = 10,
    CFT_QUEEN = 11,
    CFT_KING = 12,
    CFT_NUM_CARD_FACE_TYPES
};

// 卡牌状态
enum class CardState {
    FACE_DOWN,  // 背面
    FACE_UP,    // 正面
    REMOVED     // 已移除
};

#endif // GAME_CONSTS_H