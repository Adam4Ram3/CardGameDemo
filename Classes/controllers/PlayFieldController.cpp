
#include "controllers/PlayFieldController.h"
#include "controllers/GameController.h"
#include "controllers/StackController.h"
#include "services/GameLogicService.h"
#include "views/CardView.h"
#include "views/GameView.h" 

using namespace cocos2d;

PlayFieldController* PlayFieldController::create() {
    PlayFieldController* ret = new (std::nothrow) PlayFieldController();
    if (ret) { ret->autorelease(); return ret; }
    CC_SAFE_DELETE(ret); return nullptr;
}

void PlayFieldController::init(std::shared_ptr<GameModel> model, std::shared_ptr<UndoManager> undoMgr, GameController* mainController) {
    _gameModel = model;
    _undoManager = undoMgr;
    _mainController = mainController;
}

void PlayFieldController::initView(GameView* gameView) {
    if (!_gameModel) return;

    // 【布局修正】主牌区整体上移偏移量
    // 原始 Y=600 会挡住堆牌区(高度580)，所以我们把所有主牌向上提 250 像素
    const float PLAYFIELD_OFFSET_Y = 250.0f;

    for (auto& card : _gameModel->allCards) {
        // 筛选出 PlayField 的牌 (OriginPos != 0,0)
        if (!card->getOriginPosition().equals(Vec2::ZERO)) {

            // 1. 修正 Model 数据位置 (防止视觉和逻辑不一致)
            Vec2 oldPos = card->getPosition();
            Vec2 newPos = Vec2(oldPos.x, oldPos.y + PLAYFIELD_OFFSET_Y);

            // 使用 Service 修改数据 (遵循架构)
            // 注意：这里不需要记入 Undo，这是初始化布局
            GameLogicService::applyMove(card.get(), newPos, card->getZIndex());

            // 2. 创建视图
            CardView* cv = CardView::create(card.get());
            if (cv) {
                cv->setClickCallback([this](int id) {
                    this->handleCardClick(id);
                    });
                gameView->addCardView(cv);
            }
        }
    }
}

// ... handleCardClick 保持不变 ...
bool PlayFieldController::handleCardClick(int cardId) {
    auto card = _gameModel->getCardById(cardId);
    if (!card) return false;

    auto stackCtrl = _mainController->getStackController();
    auto topCard = stackCtrl->getTopCard();

    if (!topCard) return false;

    if (GameLogicService::canMatch(topCard.get(), card.get())) {
        UndoCommand cmd(card->getId(), card->getPosition(), topCard->getId(), card->getState(), card->getZIndex());
        _undoManager->pushCommand(cmd);

        Vec2 targetPos = topCard->getPosition();
        _mainController->performMoveCard(card, targetPos);

        CCLOG("Action: PlayField Match Success");
        return true;
    }

    return false;
}