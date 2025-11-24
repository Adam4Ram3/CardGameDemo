/**
 * @file StackController.cpp
 * @brief 备用牌堆控制器实现 - 负责管理底牌堆和抽牌逻辑
 * 
 * @details 职责：
 * 1. **初始化牌堆**：将标记为 Stack 的卡牌分配到备用堆（Stock）和底牌堆（Active）
 * 2. **处理抽牌**：响应备用牌堆的点击事件，将新牌翻开并移动到底牌堆
 * 3. **维护状态**：记录当前底牌堆顶部的卡牌（TopCard），供匹配逻辑查询
 * 
 * @note 交互逻辑：
 * - 点击备用堆（Stock） -> 翻牌 -> 移动到底牌堆（Active） -> 成为新的 TopCard
 * - 记录 Undo 操作，支持回退翻牌动作
 */
#include "controllers/StackController.h"
#include "controllers/GameController.h"
#include "services/GameLogicService.h"
#include "views/CardView.h"
#include "views/GameView.h"

using namespace cocos2d;


/**
 * @brief 创建实例（静态工厂方法）
 * @return StackController* 自动释放的实例指针
 * @note 遵循 Cocos2d-x 的 create 模式
 */
StackController* StackController::create() {
    StackController* ret = new (std::nothrow) StackController();
    if (ret) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

/**
 * @brief 初始化控制器
 * @param model 游戏数据模型（共享）
 * @param undoMgr 回退管理器（共享）
 * @param mainController 主控制器引用（用于回调）
 * 
 * @details 
 * - 保存核心组件的引用
 * - 计算备用堆（Stock）和底牌堆（Active）的屏幕坐标
 * - 坐标基于屏幕中心计算，适配不同分辨率
 */
void StackController::init(std::shared_ptr<GameModel> model, std::shared_ptr<UndoManager> undoMgr, GameController* mainController) {
    _gameModel = model;
    _undoManager = undoMgr;
    _mainController = mainController;

    // 获取屏幕可见区域大小
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 计算备用牌堆位置（左侧）
    _stockPos = Vec2(visibleSize.width / 2 - 250, 290);

    // 计算底牌堆位置（右侧，用于放置翻开的牌）
    _activePos = Vec2(visibleSize.width / 2 + 150, 290);
}


/**
 * @brief 初始化视图层
 * @param gameView 游戏主视图
 * 
 * @details 执行流程：
 * 1. **筛选卡牌**：从 GameModel 中找出所有属于 Stack 的卡牌（位置为 ZERO 的牌）
 * 2. **分配位置**：
 *    - 最后一张牌 -> 翻开 -> 放入底牌堆（Active） -> 设为 TopCard
 *    - 其他牌 -> 覆盖 -> 放入备用堆（Stock）
 * 3. **创建视图**：为每张牌创建 CardView，并绑定点击回调
 * 4. **添加到场景**：将 CardView 添加到 GameView 中显示
 */
void StackController::initView(GameView* gameView) {
    if (!_gameModel) return;

    // 筛选出所有备用牌（在 LevelConfigLoader 中位置被设为 0,0 的牌）
    std::vector<std::shared_ptr<CardModel>> stackCards;
    for (auto& card : _gameModel->allCards) {
        if (card->getOriginPosition().equals(Vec2::ZERO)) {
            stackCards.push_back(card);
        }
    }

    int offsetIndex = 0;

    // 遍历并设置初始状态
    for (auto& card : stackCards) {

        // 判断是否是最后一张牌（作为初始底牌）
        bool isLastOne = (card == stackCards.back());
        if (isLastOne) {
            // 初始底牌：翻开，放在右侧 Active 位置
            GameLogicService::applyMove(card.get(), _activePos,100);
            GameLogicService::applyStateChange(card.get(), CardState::FACE_UP);
            _topStackCard = card;// 记录为当前底牌
        }
        else {

            float offsetX = offsetIndex * 70.0f;
            float offsetY = offsetIndex * 0.f;

            Vec2 finalPos = _stockPos + Vec2(offsetX, offsetY);
            // 备用牌：覆盖，放在左侧 Stock 位置
            GameLogicService::applyMove(card.get(),finalPos, offsetIndex);
            GameLogicService::applyStateChange(card.get(), CardState::FACE_DOWN);
            offsetIndex++;
        }

        // 创建视图对象
        CardView* cv = CardView::create(card.get());
        if (cv) {
            // 绑定点击回调：点击时调用 handleCardClick
            cv->setClickCallback([this](int id) {
                this->handleCardClick(id);
                });
            // 将卡牌视图添加到游戏场景中
            gameView->addCardView(cv);
        }
    }
}

/**
 * @brief 处理卡牌点击事件
 * @param cardId 被点击的卡牌ID
 * @return bool 是否成功处理了点击
 * 
 * @details 逻辑判断：
 * 1. 检查卡牌是否存在
 * 2. **抽牌逻辑**：
 *    - 如果点击的是备用堆（Stock）中的覆盖牌（FACE_DOWN）
 *    - 记录 Undo 操作（记录当前状态和之前的 TopCard）
 *    - 将牌翻开（FACE_UP）
 *    - 调用主控制器执行移动动画（移到底牌堆 Active）
 *    - 主控制器会自动更新 TopCard
 */
bool StackController::handleCardClick(int cardId) {
    auto card = _gameModel->getCardById(cardId);
    if (!card) return false;

    // 【修改】判定逻辑升级
    // 1. 是 Stack 区域的牌 (OriginPos == Zero)
    // 2. 是背面状态 (FACE_DOWN)
    // 3. (可选) 它是当前最上面的一张备用牌 (防止点到下面盖着的牌)
    //    但在 Cocos 的触摸机制里，上面的 View 会优先吞噬触摸，所以这里简化判断即可

    bool isStockCard = card->getOriginPosition().equals(Vec2::ZERO);
    bool isFaceDown = (card->getState() == CardState::FACE_DOWN);

    // 只有位于备用堆且覆盖的牌可以点击（抽牌操作）
    if (isStockCard && isFaceDown) {

        // 1. 记录撤销命令（关键：记录移动前的状态）
        // 参数：卡牌ID, 原始位置, 当前底牌ID, 原始状态, 原始Z序
        UndoCommand cmd(card->getId(), card->getPosition(), _topStackCard->getId(), card->getState(), card->getZIndex());
        _undoManager->pushCommand(cmd);

        // 2. 更新数据状态：翻开
        GameLogicService::applyStateChange(card.get(), CardState::FACE_UP);

        // 3. 委托主控制器执行移动逻辑（包含动画和更新 TopCard）
        if (_mainController) {
            _mainController->performMoveCard(card, _activePos);
        }
        return true;
    }
    return false;
}

/**
 * @brief 更新当前底牌堆顶部的卡牌引用
 * @param card 新的顶部卡牌
 * 
 * @note 此方法通常由 GameController 在执行移动后调用
 *       或者在 Undo 操作恢复状态时调用
 */
void StackController::setTopCard(std::shared_ptr<CardModel> card) {
    _topStackCard = card;
}