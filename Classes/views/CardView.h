/**
 * @file CardView.h
 * @brief 卡牌视图组件 - 负责单张卡牌的渲染与交互
 * 
 * @details 职责：
 * 1. **渲染显示**：根据 CardModel 的状态（花色、点数、正反面）绘制卡牌 UI
 * 2. **交互响应**：捕获触摸事件，并通过回调通知 Controller 层
 * 3. **状态同步**：提供 updateView() 方法，当 Model 数据变化时刷新显示
 * 
 * @note 架构定位：
 * - 属于 View 层，只负责"显示"和"输入捕获"
 * - 不包含业务逻辑（如匹配规则判断）
 * - 持有 CardModel 的只读指针（const CardModel*），确保 View 不会修改 Model
 */

#ifndef CARD_VIEW_H
#define CARD_VIEW_H

#include "cocos2d.h"
#include "models/CardModel.h"
#include "configs/GameConsts.h" // 确保包含枚举定义
#include <functional>
#include <string>

/**
 * @class CardView
 * @brief 卡牌 UI 节点，继承自 cocos2d::Node
 */
class CardView : public cocos2d::Node {
public:
    /**
     * @brief 创建卡牌视图实例（静态工厂方法）
     * @param model 卡牌数据模型（只读引用）
     * @return CardView* 自动释放的实例指针
     * @note 遵循 Cocos2d-x 的 create 模式
     */
    static CardView* create(const CardModel* model);

    /**
     * @brief 设置点击回调函数
     * @param callback 接收卡牌ID的函数对象 std::function<void(int)>
     * 
     * @details 当用户点击卡牌时，View 层会调用此回调，将事件传递给 Controller 层
     * @example
     * ```cpp
     * cardView->setClickCallback([this](int cardId) {
     *     this->handleCardClick(cardId);
     * });
     * ```
     */
    void setClickCallback(std::function<void(int)> callback);

    /**
     * @brief 刷新视图状态
     * @details 根据持有的 _model 数据更新 UI：
     * - 如果是 FACE_DOWN：显示牌背
     * - 如果是 FACE_UP：显示牌面（花色、数字）
     * - 更新 Z 序和位置（虽然位置通常由 Action 控制，但这里可做强制同步）
     */
    void updateView();

    /**
     * @brief 获取关联的卡牌 ID
     * @return int 卡牌唯一标识
     */
    int getCardId() const { return _modelId; }

private:
    /**
     * @brief 初始化方法（私有）
     * @param model 卡牌数据模型
     * @return bool 初始化是否成功
     * 
     * @details 
     * 1. 保存 Model 指针和 ID
     * 2. 创建背景精灵（牌背/牌面底图）
     * 3. 创建花色和数字精灵
     * 4. 初始化触摸监听器
     */
    bool init(const CardModel* model);

    // --- 辅助方法 ---

    /**
     * @brief 获取花色对应的资源文件名
     * @param suit 花色枚举
     * @return std::string 图片路径（例如 "images/suit_hearts.png"）
     */
    std::string getSuitFilename(CardSuitType suit);

    /**
     * @brief 获取数字对应的资源文件名
     * @param face 点数枚举
     * @param suit 花色枚举（用于区分红/黑颜色）
     * @param isBig 是否是中间的大数字（true=大图，false=角标小图）
     * @return std::string 图片路径
     */
    std::string getNumberFilename(CardFaceType face, CardSuitType suit, bool isBig);

    // --- UI 组件 ---
    cocos2d::Sprite* _bgSprite;        // 背景精灵（牌背或牌面底板）

    // 关键修改：将 Label 改为 Sprite，支持图片字
    cocos2d::Sprite* _bigNumberSprite; // 中间的大数字图片
    cocos2d::Sprite* _smallNumSprite;  // 左上角的小数字图片
    cocos2d::Sprite* _smallSuitSprite; // 左上角的小花色图片

    // --- 数据引用 ---
    const CardModel* _model;// 持有 Model 的只读指针
    int _modelId;// 缓存 ID，方便快速访问
    std::function<void(int)> _onClickCallback;// 点击回调
};

#endif // CARD_VIEW_H