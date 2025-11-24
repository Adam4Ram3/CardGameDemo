/**
 * @file GameView.cpp
 * @brief 游戏主视图实现 - 负责整体场景的布局与渲染
 * 
 * @details 职责：
 * 1. **场景构建**：创建游戏背景（双色分层背景）
 * 2. **UI 布局**：放置全局 UI 控件（如 Undo 按钮）
 * 3. **容器管理**：作为所有 CardView 的父容器，管理它们的添加与移除
 * 4. **事件转发**：将 UI 按钮点击事件转发给 Controller
 * 
 * @note 视觉设计：
 * - 采用上下分屏设计
 * - 上半部分：主牌区（PlayField），背景色为土黄色
 * - 下半部分：操作区（Stack/Undo），背景色为紫色
 */
#include "views/GameView.h"
#include "controllers/GameController.h" 
#include "ui/CocosGUI.h"

using namespace cocos2d;
using namespace cocos2d::ui;

/**
 * @brief 创建包含 GameView 的场景（静态工厂方法）
 * @return Scene* 包含 GameView 图层的场景对象
 * 
 * @note 这是一个便捷方法，通常由 GameController 调用
 *       用于快速构建一个完整的游戏场景
 */
Scene* GameView::createScene() {
    auto scene = Scene::create();
    auto layer = GameView::create();
    scene->addChild(layer);
    return scene;
}

/**
 * @brief 初始化视图
 * @return bool 初始化是否成功
 * 
 * @details 布局逻辑：
 * 1. **获取屏幕尺寸**：适配不同分辨率设备
 * 2. **绘制背景**：
 *    - 底部区域（高度 580px）：放置备用牌堆和底牌堆
 *    - 顶部区域（剩余高度）：放置主牌区金字塔
 * 3. **创建 UI**：
 *    - Undo 按钮：放置在底部区域右侧
 *    - 绑定点击事件：通过 getUserObject() 获取 Controller 并调用 onUndoClicked
 */
bool GameView::init() {
    if (!Layer::init()) return false;

    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 定义底部操作区的高度（像素）
    // 这个高度决定了上下背景的分界线
    float bottomAreaHeight = 580.0f;

    // ========== 1. 创建底部背景（紫色区域） ==========
    // Color4B(R, G, B, A) - 紫色 (146, 54, 147)
    auto bottomBg = LayerColor::create(Color4B(146, 54, 147, 255));
    bottomBg->setContentSize(Size(visibleSize.width, bottomAreaHeight));
    this->addChild(bottomBg, -1);

    // ========== 2. 创建顶部背景（土黄色区域） ==========
    // Color4B(R, G, B, A) - 土黄色 (173, 129, 80)
    // 高度 = 总高度 - 底部高度
    auto topBg = LayerColor::create(Color4B(173, 129, 80, 255));
    topBg->setContentSize(Size(visibleSize.width, visibleSize.height - bottomAreaHeight));
    topBg->setPosition(Vec2(0, bottomAreaHeight));
    this->addChild(topBg, -1);

    // ========== 3. 创建撤销按钮 (Undo) ==========
    auto undoBtn = Button::create();
    undoBtn->setTitleText("Undo");
    undoBtn->setTitleFontSize(40);

    // 布局位置：屏幕宽度 85% 处，底部区域垂直居中
    undoBtn->setPosition(Vec2(visibleSize.width * 0.85f, bottomAreaHeight / 2));

    // 绑定点击事件
    undoBtn->addClickEventListener([this](Ref* sender) {
        // 获取绑定的 Controller 对象（在 GameController::initView 中设置）
        // 使用 static_cast 进行类型转换
        auto controller = static_cast<GameController*>(this->getUserObject());
        if (controller) {
            controller->onUndoClicked();
        }
        });

    this->addChild(undoBtn, 100);// ZOrder 100 确保按钮在最上层，不被卡牌遮挡
    return true;
}

/**
 * @brief 添加卡牌视图到场景中
 * @param cardView 要添加的卡牌节点
 * 
 * @details 
 * - 这是一个简单的封装，方便 Controller 调用
 * - Controller 不需要关心具体的层级管理，只需调用此方法即可
 */
void GameView::addCardView(CardView* cardView) {
    if (cardView) {
        this->addChild(cardView);
    }
}