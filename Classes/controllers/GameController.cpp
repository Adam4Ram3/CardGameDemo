/**
 * @file GameController.cpp
 * @brief 游戏控制器实现 - MVC架构中的核心控制层
 * 
 * 职责：
 * 1. 协调整个游戏流程的初始化与运行
 * 2. 管理子控制器（StackController, PlayFieldController）的生命周期
 * 3. 处理卡牌移动与撤销操作的业务逻辑
 * 4. 连接数据层（GameModel）与视图层（GameView）
 * 
 * 依赖关系：
 * - 持有：GameModel（数据）、GameView（视图）、UndoManager（回退管理器）
 * - 管理：StackController（备用牌堆控制器）、PlayFieldController（主牌区控制器）
 */

#include "controllers/GameController.h"
#include "configs/loaders/LevelConfigLoader.h"
#include "services/GameModelFromLevelGenerator.h"
#include "services/GameLogicService.h"

#include "controllers/StackController.h"
#include "controllers/PlayFieldController.h"
#include "views/GameView.h"
#include "managers/UndoManager.h"
#include "views/CardView.h" 


using namespace cocos2d;

// **常量定义**
// 关卡配置文件路径规则：levels/level_<id>.json
const std::string kLevelPathPrefix = "levels/level_";
const std::string kLevelPathSuffix = ".json";


/**
 * @brief 构造函数
 * @details 初始化所有成员为空指针，避免使用未初始化的指针
 */
GameController::GameController()
    : _gameModel(nullptr)
    , _gameView(nullptr)
    , _stackController(nullptr)
    , _playFieldController(nullptr)
{
}

/**
 * @brief 析构函数
 * @details 释放子控制器的引用计数，防止内存泄漏
 * @note 使用 CC_SAFE_RELEASE 宏确保在释放前检查指针有效性
 */
GameController::~GameController() {
    CC_SAFE_RELEASE(_stackController);
    CC_SAFE_RELEASE(_playFieldController);
    CCLOG("GameController released");
}

/**
 * @brief 游戏启动入口（静态工厂方法）
 * @param levelId 关卡ID，用于加载对应的关卡配置文件
 * 
 * @details 执行流程：
 * 1. 创建 GameController 实例并加入自动释放池
 * 2. 手动 retain 一次，防止初始化过程中被提前释放
 * 3. 调用 _initWithLevel 完成实际初始化
 * 
 * @note 采用两阶段构造模式（构造 + init）是 Cocos2d-x 的标准做法
 */
void GameController::startGame(int levelId) {
    // 使用 new (std::nothrow) 避免分配失败时抛出异常
    GameController* controller = new (std::nothrow) GameController();
    if (controller) {
        controller->autorelease();
        controller->retain(); // 保持引用，直到初始化完成
        controller->_initWithLevel(levelId);
    }
}


/**
 * @brief 关卡初始化私有方法（核心初始化逻辑）
 * @param levelId 关卡ID
 * 
 * @details 执行流程（严格按照 MVC 架构）：
 * 1. **加载静态配置** - 从 JSON 文件读取关卡布局数据（Config 层）
 * 2. **生成运行时数据** - 将配置转换为 GameModel（Service 层）
 * 3. **初始化管理器** - 创建 UndoManager 处理撤销逻辑（Manager 层）
 * 4. **初始化子控制器** - 创建并配置 StackController 和 PlayFieldController
 * 5. **构建视图层** - 创建 Scene 和 GameView，将视图组装到场景中
 * 6. **视图与数据绑定** - 调用子控制器的 initView 方法生成卡牌精灵
 * 7. **切换场景** - 将游戏场景设为当前运行场景
 * 
 * @warning 任何一步失败都应该中断并记录日志，避免产生未定义行为
 */
void GameController::_initWithLevel(int levelId) {
    // ========== 步骤1: 加载关卡配置 ==========
    // 拼接配置文件路径，例如 "levels/level_1.json"
    std::string path = StringUtils::format("%s%d%s", kLevelPathPrefix.c_str(), levelId, kLevelPathSuffix.c_str());
    LevelConfig config = LevelConfigLoader::loadLevelConfig(path);

    // 配置校验：如果没有任何卡牌数据，则中断初始化
    if (config.playFieldCards.empty() && config.stackCards.empty()) {
        CCLOG("Error: Level config empty");
        return;
    }

    // ========== 步骤2: 生成游戏数据模型 ==========
    // 调用 Service 层将静态配置转换为运行时数据
    _gameModel = GameModelFromLevelGenerator::generateGameModel(config);

    // ========== 步骤3: 初始化回退管理器 ==========
    // 使用 shared_ptr 管理 UndoManager 生命周期（Manager 层禁止单例）
    _undoManager = std::make_shared<UndoManager>();

    // ========== 步骤4: 初始化备用牌堆控制器 ==========
    // 负责管理抽牌区域的业务逻辑
    _stackController = StackController::create();
    _stackController->retain();
    _stackController->init(_gameModel, _undoManager, this);

    // ========== 步骤5: 初始化主牌区控制器 ==========
    // 负责管理桌面卡牌的点击与匹配逻辑
    _playFieldController = PlayFieldController::create();
    _playFieldController->retain();
    _playFieldController->init(_gameModel, _undoManager, this);

    // ========== 步骤6: 创建场景与主视图 ==========
    Scene* scene = Scene::create(); //Cocos2d-x 场景容器
    _gameView = GameView::create(); //游戏主视图（UI布局管理器）

    if (_gameView) {
        scene->addChild(_gameView);
        //将 Controller绑定到View的UserObject，便于回调中访问
        _gameView->setUserObject(this);

        // ========== 步骤7: 子控制器初始化各自的视图 ==========
        // 每个子控制器负责创建并管理自己区域的卡牌精灵
        if (_stackController) _stackController->initView(_gameView);
        if (_playFieldController) _playFieldController->initView(_gameView);
    }

    // ========== 步骤8: 切换到游戏场景 ==========
    // 如果已有场景在运行，则替换；否则直接运行
    if (Director::getInstance()->getRunningScene()) {
        Director::getInstance()->replaceScene(scene);
    }
    else {
        Director::getInstance()->runWithScene(scene);
    }

    // ========== 步骤9: 释放临时引用 ==========
    // 对应 startGame 中的 retain，平衡引用计数
    this->release();
}


/**
 * @brief 执行卡牌移动操作（核心业务逻辑）
 * @param card 要移动的卡牌数据模型
 * @param targetPos 目标位置（通常是底牌堆顶部）
 * 
 * @details 执行流程：
 * 1. 获取当前底牌堆顶部卡牌，计算新卡牌的 Z 序（确保新卡在最上层）
 * 2. 调用 GameLogicService 更新卡牌的逻辑数据（位置、Z序）
 * 3. 更新 StackController 的顶部卡牌引用
 * 4. 在视图层找到对应的 CardView，执行移动动画
 * 
 * @note 这个方法体现了 MVC 的分层：
 *       - Service 层处理数据更新（无状态）
 *       - Controller 协调逻辑与视图
 *       - View 层只负责动画表现
 */
void GameController::performMoveCard(std::shared_ptr<CardModel> card, const cocos2d::Vec2& targetPos) {
    if (!_stackController) return;

    // ========== 数据层更新 ==========
    // 1. 获取当前的“栈顶牌”（也就是底牌堆最上面那张）
    auto topCard = _stackController->getTopCard();
    // 2. 计算层级 (Z-Index)
    // 这是一个关键细节：新飞来的牌必须盖在旧牌上面。
    // 如果有旧牌，新牌的层级 = 旧牌层级 + 1；如果是第一张牌，层级设为 100。
    int newZ = topCard ? topCard->getZIndex() + 1 : 100;

    // 3. 更新卡牌的物理数据
    // 调用 Service 层，把这张牌的 (x, y) 坐标改为目标位置，Z轴改为新层级。
    // 注意：这一步只是改了内存里的数据，屏幕上的牌还没动。
    GameLogicService::applyMove(card.get(), targetPos, newZ);

    // 4. 更新游戏状态
    // 告诉 StackController：“现在这张牌是新的底牌了，以后匹配都要跟它比。”
    _stackController->setTopCard(card);


    // ========== 视图层更新 ==========
    // 遍历所有卡牌视图，找到对应的 CardView 并执行动画
    // 遍历 GameView 里的所有子节点
    for (auto child : _gameView->getChildren()) {
        // 尝试把节点转换成 CardView
        CardView* cv = dynamic_cast<CardView*>(child);
        if (cv && cv->getCardId() == card->getId()) {
            cv->stopAllActions();  // 安全措施：如果这张牌正在动，先打断它

            // 创建移动动画（0.3秒移动到目标位置）
            auto move = MoveTo::create(0.3f, targetPos);

            // 动画完成后的回调：更新Z序并刷新显示
            auto callback = CallFunc::create([cv, newZ]() {
                cv->setLocalZOrder(newZ);
                cv->updateView();
                });

            // ========== 动画组合 ==========
            // Spawn: 同时执行移动 + 缩放弹跳效果
            // Sequence: 先放大 1.2 倍，再缩回 1.0 倍（模拟"吃牌"的反馈）
            cv->runAction(Sequence::create(
                Spawn::create(move, Sequence::create(ScaleTo::create(0.15f, 1.2f), ScaleTo::create(0.15f, 1.0f), nullptr), nullptr),
                callback, nullptr
            ));
            break;
        }
    }
}

/**
 * @brief 处理撤销操作（Undo 按钮点击回调）
 * 
 * @details 执行流程（与 performMoveCard 相反）：
 * 1. 检查 UndoManager 是否有可撤销的操作
 * 2. 从撤销栈中弹出最后一次操作的命令数据
 * 3. 根据命令数据恢复卡牌的位置、Z序、状态
 * 4. 更新底牌堆顶部引用为撤销前的卡牌
 * 5. 在视图层播放反向动画（将卡牌移回原位置）
 * 
 * @note 撤销逻辑完全依赖 UndoCommand 中记录的快照数据：
 *       - cardId: 被操作的卡牌ID
 *       - fromPos: 原始位置
 *       - prevZIndex: 原始 Z 序
 *       - prevState: 原始状态（翻开/覆盖）
 *       - prevTopCardId: 撤销前的底牌堆顶部卡牌ID
 */
void GameController::onUndoClicked() {
    // ========== 前置检查 ==========
    if (!_undoManager || !_undoManager->canUndo()) return;

    // ========== 获取撤销命令 ==========
    UndoCommand cmd = _undoManager->popCommand();

    // 根据 ID 获取相关卡牌的 shared_ptr
    auto currentCard = _gameModel->getCardById(cmd.cardId);
    auto prevTopCard = _gameModel->getCardById(cmd.prevTopCardId);

    if (currentCard && prevTopCard) {

        // ========== 数据层恢复 ==========
        // 恢复卡牌的位置和 Z 序
        GameLogicService::applyMove(currentCard.get(), cmd.fromPos, cmd.prevZIndex);

        // 恢复卡牌的状态（例如从翻开变回覆盖）
        GameLogicService::applyStateChange(currentCard.get(), cmd.prevState);

        // 恢复底牌堆的顶部引用
        if (_stackController) _stackController->setTopCard(prevTopCard);

        // ========== 视图层恢复 ==========
        // 找到对应的 CardView 并播放反向移动动画
        for (auto child : _gameView->getChildren()) {
            CardView* cv = dynamic_cast<CardView*>(child);
            if (cv && cv->getCardId() == currentCard->getId()) {
                cv->stopAllActions(); // 停止当前动画

                // 创建反向移动动画（回到原位置）
                auto move = MoveTo::create(0.3f, cmd.fromPos);

                // 动画完成后更新 Z 序并刷新显示
                auto callback = CallFunc::create([cv, currentCard]() {
                    cv->setLocalZOrder(currentCard->getZIndex());
                    cv->updateView();
                    });
                cv->runAction(Sequence::create(move, callback, nullptr));
            }
        }
    }
}