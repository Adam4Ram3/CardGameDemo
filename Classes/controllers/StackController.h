#ifndef STACK_CONTROLLER_H
#define STACK_CONTROLLER_H

#include "cocos2d.h"
#include "models/GameModel.h"
#include "managers/UndoManager.h"
#include <memory>
#include <vector>

// 【关键修复】前向声明
class GameView;
class GameController;

class StackController : public cocos2d::Ref {
public:
    static StackController* create();

    void init(std::shared_ptr<GameModel> model, std::shared_ptr<UndoManager> undoMgr, GameController* mainController);

    // 这里用到了 GameView*，所以上面必须有 class GameView;
    void initView(GameView* gameView);

    bool handleCardClick(int cardId);
    std::shared_ptr<CardModel> getTopCard() const { return _topStackCard; }
    void setTopCard(std::shared_ptr<CardModel> card);

private:
    std::shared_ptr<GameModel> _gameModel;
    std::shared_ptr<UndoManager> _undoManager;
    GameController* _mainController;

    std::shared_ptr<CardModel> _topStackCard;
    cocos2d::Vec2 _stockPos;
    cocos2d::Vec2 _activePos;
};

#endif // STACK_CONTROLLER_H