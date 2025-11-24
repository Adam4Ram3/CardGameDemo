/**
 * @file LevelSelectView.cpp
 * @brief 关卡选择场景实现 - 游戏的入口界面
 * 
 * @details 职责：
 * 1. **展示入口**：作为游戏启动后的第一个场景，展示关卡列表
 * 2. **用户交互**：提供按钮供玩家选择关卡
 * 3. **流程跳转**：捕获选择事件，调用 GameController 启动具体关卡
 * 
 * @note 设计风格：
 * - 采用极简设计：深灰色背景 + 白色文字
 * - 使用 CocosGUI 的 Button 组件，自带点击效果
 */
#include "views/LevelSelectView.h"
#include "controllers/GameController.h"
#include "ui/CocosGUI.h"

using namespace cocos2d;
using namespace cocos2d::ui;

/**
 * @brief 初始化场景
 * @return bool 初始化是否成功
 * 
 * @details 布局逻辑：
 * 1. **背景**：创建全屏深灰色 LayerColor
 * 2. **标题**：屏幕上方 70% 处显示 "SELECT LEVEL"
 * 3. **按钮**：屏幕中央显示 "Level 1" 按钮
 *    - 使用 Lambda 表达式绑定点击事件
 *    - 点击后调用 onLevelSelected(1)
 */
bool LevelSelectView::init() {
    // 调用父类 Scene 的 init 方法，确保基础场景功能正常
    if (!Scene::init()) return false;
    // 获取屏幕可见区域大小，用于适配布局
    Size visibleSize = Director::getInstance()->getVisibleSize();

        // ========== 1. 创建背景 (深灰色) ==========
    // Color4B(R, G, B, A) - 深灰 (50, 50, 50)
    auto bg = LayerColor::create(Color4B(50, 50, 50, 255));
    this->addChild(bg);

    // ========== 2. 创建标题文本 ==========
    // 使用系统字体 Arial，字号 60
    auto label = Label::createWithSystemFont("SELECT LEVEL", "Arial", 60);
    label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.7));
    this->addChild(label);

    // ========== 3. 创建"第1关"按钮 ==========
    // 这里使用 Cocos 自带的 Button 控件
    // create() 创建一个空按钮（无背景图），仅显示文字
    auto btnLevel1 = Button::create();
    btnLevel1->setTitleText("Level 1");
    btnLevel1->setTitleFontSize(50);
    btnLevel1->setTitleColor(Color3B::WHITE);

    btnLevel1->setScale(2.0f);
    btnLevel1->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.5));

    btnLevel1->addClickEventListener([this](Ref* sender) {
        this->onLevelSelected(1); // 硬编码：选择关卡 ID = 1
        });

    this->addChild(btnLevel1);

    return true;
}

/**
 * @brief 处理关卡选择事件
 * @param levelId 用户选择的关卡ID
 * 
 * @details 
 * 1. 打印日志，方便调试
 * 2. 委托 GameController 启动游戏流程
 * 3. GameController::startGame 会负责创建新场景并切换，本场景将被销毁
 */
void LevelSelectView::onLevelSelected(int levelId) {
    CCLOG("UI: User selected Level %d", levelId);
    // 调用 GameController 的静态工厂方法启动游戏
    // GameController 会自动加载 level_1.json 并切换场景
    GameController::startGame(levelId);
}