/**
 * @file CardView.cpp
 * @brief 卡牌视图实现 - 负责卡牌的渲染、布局与交互
 * 
 * @details 职责：
 * 1. **UI 构建**：组合背景、花色、数字等精灵节点
 * 2. **布局管理**：根据卡牌尺寸计算各元素的相对坐标
 * 3. **状态刷新**：响应 Model 变化，切换正反面显示状态
 * 4. **事件分发**：捕获触摸事件并回调给 Controller
 * 
 * @note 资源依赖：
 * - 需要 "card_general.png" 作为通用底板
 * - 需要 "suits/" 和 "number/" 目录下的花色与数字图片
 */

#include "views/CardView.h"

using namespace cocos2d;

/**
 * @brief 创建卡牌视图（静态工厂方法）
 * @param model 卡牌数据模型（只读）
 * @return CardView* 自动释放的实例指针
 */
CardView* CardView::create(const CardModel* model) {
    CardView* pRet = new(std::nothrow) CardView();
    if (pRet && pRet->init(model)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    pRet = nullptr;
    return nullptr;
}


/**
 * @brief 初始化视图组件
 * @param model 卡牌数据模型
 * @return bool 初始化是否成功
 * 
 * @details 布局逻辑：
 * 1. **背景层**：创建卡牌底板精灵
 * 2. **内容层**：
 *    - 左上角：小数字 + 小花色（垂直排列）
 *    - 中间：大数字（偏右下布局）
 * 3. **交互层**：绑定触摸监听器
 */
bool CardView::init(const CardModel* model) {
    if (!Node::init()) return false;

    _model = model;
    _modelId = model->getId();

    // ========== 1. 创建背景底板 ==========
    _bgSprite = Sprite::create("card_general.png");
    Size bgSize = Size(182, 282);

    if (_bgSprite) {
        bgSize = _bgSprite->getContentSize();
        this->addChild(_bgSprite);
    }
    else {
        // 容错处理：如果图片加载失败，绘制一个白色矩形代替
        auto debugLayer = LayerColor::create(Color4B(255, 255, 255, 255), bgSize.width, bgSize.height);
        debugLayer->setPosition(Vec2(-bgSize.width / 2, -bgSize.height / 2));
        this->addChild(debugLayer);
    }

    // ========== 坐标系计算 ==========
    // 坐标原点在卡牌中心 (0,0)
    // 左上角锚点位置：X 轴向右 12%，Y 轴向上 88%
    float leftMargin = bgSize.width * 0.12f;
    float topMargin = bgSize.height * 0.88f;
    Vec2 topLeftPos = Vec2(leftMargin, topMargin);// 实际上是相对于左下角的坐标，需注意 Sprite 坐标系

    // 中间大数字位置：偏右下，避免遮挡左上角
    Vec2 bigNumPos = Vec2(bgSize.width * 0.55f, bgSize.height * 0.40f);


    // 注意：如果 _bgSprite 存在，子节点坐标系以 _bgSprite 左下角为 (0,0)
    // 如果 _bgSprite 不存在，子节点直接加在 this 上，坐标系需调整


    // ========== 2. 创建中间大数字 ==========
    std::string bigNumPath = getNumberFilename(_model->getFace(), _model->getSuit(), true);
    _bigNumberSprite = Sprite::create(bigNumPath);
    if (_bigNumberSprite) {
        _bigNumberSprite->setPosition(bigNumPos);
        // 可选缩放
        // _bigNumberSprite->setScale(0.9f); 
        if (_bgSprite) _bgSprite->addChild(_bigNumberSprite); else this->addChild(_bigNumberSprite);
    }

    // ========== 3. 创建左上角小数字 ==========
    std::string smallNumPath = getNumberFilename(_model->getFace(), _model->getSuit(), false);
    _smallNumSprite = Sprite::create(smallNumPath);
    if (_smallNumSprite) {
        _smallNumSprite->setPosition(topLeftPos);
        _smallNumSprite->setScale(0.6f); // 缩小至 60%
        if (_bgSprite) _bgSprite->addChild(_smallNumSprite); else this->addChild(_smallNumSprite);
    }

    // ========== 4. 创建左上角小花色 ==========
    std::string suitPath = getSuitFilename(_model->getSuit());
    _smallSuitSprite = Sprite::create(suitPath);
    if (_smallSuitSprite) {
        // 位于小数字下方 30 像素
        float suitOffset = 30.0f;
        _smallSuitSprite->setPosition(topLeftPos - Vec2(0, suitOffset));
        _smallSuitSprite->setScale(0.35f);// 缩小至 35%
        if (_bgSprite) _bgSprite->addChild(_smallSuitSprite); else this->addChild(_smallSuitSprite);
    }

    // ========== 5. 设置初始位置与层级 ==========
    this->setPosition(_model->getPosition());
    this->setLocalZOrder(_model->getZIndex());

    // ========== 6. 绑定触摸事件 ==========
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);// 吞噬事件，防止穿透

    // 触摸开始：检测点击点是否在卡牌范围内
    listener->onTouchBegan = [this, bgSize](Touch* touch, Event* event) {
        // 将屏幕坐标转换为节点局部坐标
        Vec2 p = this->convertToNodeSpace(touch->getLocation());

        // 简单的矩形碰撞检测（假设锚点在中心）
        if (_bgSprite) {
            return p.x >= -bgSize.width / 2 && p.x <= bgSize.width / 2 &&
                p.y >= -bgSize.height / 2 && p.y <= bgSize.height / 2;
        }
        return false;
        };
    // 触摸结束：触发回调
    listener->onTouchEnded = [this](Touch* touch, Event* event) {
        if (_onClickCallback) _onClickCallback(_modelId);
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 初始刷新一次视图状态
    updateView();
    return true;
}

/**
 * @brief 设置点击回调
 * @param callback 回调函数
 */
void CardView::setClickCallback(std::function<void(int)> callback) {
    _onClickCallback = callback;
}

/**
 * @brief 刷新视图状态
 * @details 根据 Model 的状态更新 UI：
 * - FACE_UP: 显示白色底板、数字和花色
 * - FACE_DOWN: 显示灰色底板（模拟背面），隐藏数字和花色
 * - REMOVED: 隐藏整个节点
 */
void CardView::updateView() {
    if (!_model) return;

    this->setPosition(_model->getPosition());
    this->setLocalZOrder(_model->getZIndex());

    if (_model->getState() == CardState::FACE_UP) {
        // 正面状态
        if (_bgSprite) _bgSprite->setColor(Color3B::WHITE);
        if (_bigNumberSprite) _bigNumberSprite->setVisible(true);
        if (_smallNumSprite) _smallNumSprite->setVisible(true);
        if (_smallSuitSprite) _smallSuitSprite->setVisible(true);
    }
    else {
        // 背面状态（简单变灰模拟，实际项目中应更换为背面图片）
        if (_bgSprite) _bgSprite->setColor(Color3B(150, 150, 150));
        if (_bigNumberSprite) _bigNumberSprite->setVisible(false);
        if (_smallNumSprite) _smallNumSprite->setVisible(false);
        if (_smallSuitSprite) _smallSuitSprite->setVisible(false);
    }

    // 如果状态是 REMOVED，则隐藏节点
    this->setVisible(_model->getState() != CardState::REMOVED);
}

/**
 * @brief 获取花色图片路径
 * @param suit 花色枚举
 * @return 资源路径字符串
 */
std::string CardView::getSuitFilename(CardSuitType suit) {
    switch (suit) {
    case CardSuitType::CST_CLUBS:    return "suits/club.png";
    case CardSuitType::CST_DIAMONDS: return "suits/diamond.png";
    case CardSuitType::CST_HEARTS:   return "suits/heart.png";
    case CardSuitType::CST_SPADES:   return "suits/spade.png";
    default: return "";
    }
}

/**
 * @brief 获取数字图片路径
 * @param face 点数枚举
 * @param suit 花色枚举（用于区分红黑）
 * @param isBig 是否是大图
 * @return 资源路径字符串
 * 
 * @details 命名规则：
 * - 红色系（方块、红桃）：number/big_red_A.png
 * - 黑色系（梅花、黑桃）：number/small_black_10.png
 */
std::string CardView::getNumberFilename(CardFaceType face, CardSuitType suit, bool isBig) {
    std::string color = "black";
    if (suit == CardSuitType::CST_DIAMONDS || suit == CardSuitType::CST_HEARTS) {
        color = "red";
    }

    std::string faceStr;
    switch (face) {
    case CardFaceType::CFT_ACE:   faceStr = "A"; break;
    case CardFaceType::CFT_JACK:  faceStr = "J"; break;
    case CardFaceType::CFT_QUEEN: faceStr = "Q"; break;
    case CardFaceType::CFT_KING:  faceStr = "K"; break;
    default: faceStr = std::to_string((int)face + 1); break;
    }

    std::string prefix = isBig ? "big_" : "small_";
    return "number/" + prefix + color + "_" + faceStr + ".png";
}