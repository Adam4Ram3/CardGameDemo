#ifndef GAME_VIEW_H
#define GAME_VIEW_H

#include "cocos2d.h"
#include "views/CardView.h" // 只有 CardView 是必须包含的
#include <vector>

// 【注意】绝对不要在这里 include GameController.h
// 也不需要前向声明 GameController，因为 .h 里没用到它

class GameView : public cocos2d::Layer {
public:
    static cocos2d::Scene* createScene();
    CREATE_FUNC(GameView);

    virtual bool init();
    void addCardView(CardView* cardView);
};

#endif // GAME_VIEW_H