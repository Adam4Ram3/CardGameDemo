#ifndef CARD_MODEL_H
#define CARD_MODEL_H

#include "configs/GameConsts.h"
#include "cocos2d.h"

class CardModel {
public:
    // 【修正】这里也要改用 CST_NONE / CFT_NONE
    CardModel()
        : _id(-1)
        , _face(CardFaceType::CFT_NONE)
        , _suit(CardSuitType::CST_NONE)
        , _state(CardState::FACE_DOWN)
        , _zIndex(0)
    {
    }

    void init(int id, CardFaceType face, CardSuitType suit, const cocos2d::Vec2& pos) {
        _id = id;
        _face = face;
        _suit = suit;
        _position = pos;
        _originPosition = pos;
        _state = CardState::FACE_DOWN;
    }

    int getId() const { return _id; }
    CardFaceType getFace() const { return _face; }
    CardSuitType getSuit() const { return _suit; }
    const cocos2d::Vec2& getPosition() const { return _position; }
    const cocos2d::Vec2& getOriginPosition() const { return _originPosition; }
    CardState getState() const { return _state; }
    int getZIndex() const { return _zIndex; }

    void setPosition(const cocos2d::Vec2& pos) { _position = pos; }
    void setState(CardState state) { _state = state; }
    void setZIndex(int z) { _zIndex = z; }

private:
    int _id;
    CardFaceType _face;
    CardSuitType _suit;
    cocos2d::Vec2 _position;
    cocos2d::Vec2 _originPosition;
    CardState _state;
    int _zIndex;
};

#endif // CARD_MODEL_H