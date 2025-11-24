#ifndef PLAY_FIELD_CONTROLLER_H
#define PLAY_FIELD_CONTROLLER_H

#include "cocos2d.h"
#include "models/GameModel.h"
#include "managers/UndoManager.h"
#include <memory>

// 【关键修复】前向声明
class GameView;
class GameController;

class PlayFieldController : public cocos2d::Ref {
public:
    static PlayFieldController* create();

    void init(std::shared_ptr<GameModel> model, std::shared_ptr<UndoManager> undoMgr, GameController* mainController);

    // 这里用到了 GameView*，所以需要前向声明
    void initView(GameView* gameView);

    bool handleCardClick(int cardId);

private:
    std::shared_ptr<GameModel> _gameModel;
    std::shared_ptr<UndoManager> _undoManager;
    GameController* _mainController;
};

#endif // PLAY_FIELD_CONTROLLER_H