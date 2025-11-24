/**
 * @file GameController.h
 * @brief 游戏主控制器 - MVC架构中的核心协调层
 * 
 * @details 职责范围：
 * 1. **游戏生命周期管理**：负责游戏的启动、初始化、场景切换
 * 2. **子控制器协调**：管理 StackController（备用牌堆）和 PlayFieldController（主牌区）
 * 3. **数据与视图桥接**：连接 GameModel（数据层）与 GameView（视图层）
 * 4. **核心业务逻辑**：处理卡牌移动、撤销等跨模块操作
 * 
 * @note 设计原则：
 * - **单一数据源**：所有卡牌状态统一由 GameModel 管理
 * - **职责分离**：不直接操作 UI，通过 GameView 和子控制器间接控制
 * - **可测试性**：所有关键逻辑都可通过 Mock 子控制器进行单元测试
 * 
 * @warning 注意事项：
 * - 本类继承自 cocos2d::Ref，使用引用计数管理生命周期
 * - 子控制器使用原始指针 + retain/release 手动管理
 * - UndoManager 和 GameModel 使用 shared_ptr 自动管理
 * 
 * @see GameModel - 游戏数据模型
 * @see GameView - 游戏主视图
 * @see StackController - 备用牌堆控制器
 * @see PlayFieldController - 主牌区控制器
 */
#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "cocos2d.h"
#include <memory>
#include <string>
#include <vector>

// ========== 前向声明 (Forward Declarations) ==========
// 优点：
// 1. 避免循环依赖（例如 GameView 可能也需要引用 GameController）
// 2. 加快编译速度（减少头文件包含链）
// 3. 降低耦合度（只在 .cpp 中包含具体实现）
class GameModel;
class UndoManager;
class GameView;         // 【关键】只声明，不 include
class StackController;
class PlayFieldController;
class CardModel;


/**
 * @class GameController
 * @brief 游戏主控制器，负责协调所有子系统
 * 
 * @details 类图关系：
 * ```
 * GameController (1) --owns--> (1) GameModel
 *                (1) --owns--> (1) GameView
 *                (1) --owns--> (1) UndoManager
 *                (1) --manages--> (1) StackController
 *                (1) --manages--> (1) PlayFieldController
 * ```
 * 
 * @note 生命周期：
 * - 通过静态工厂方法 startGame() 创建
 * - 初始化完成后由 Director 的场景持有
 * - 场景销毁时自动释放（由 Cocos2d-x 引用计数管理）
 */
class GameController : public cocos2d::Ref {
public:
    // ==================== 公共接口 ====================
    
    /**
     * @brief 游戏启动入口（静态工厂方法）
     * @param levelId 关卡ID，用于加载对应的关卡配置文件
     * 
     * @details 执行流程：
     * 1. 创建 GameController 实例
     * 2. 加载关卡配置 (LevelConfig)
     * 3. 生成运行时数据 (GameModel)
     * 4. 初始化子控制器和视图
     * 5. 切换到游戏场景
     * 
     * @example
     * // 在菜单场景中启动第一关
     * GameController::startGame(1);
     * ```
     * 
     * @warning 
     * - 调用此方法会自动切换场景，之前的场景会被销毁
     * - levelId 必须对应一个存在的配置文件（levels/level_<id>.json）
     */
    static void startGame(int levelId);

    /**
     * @brief 执行卡牌移动操作（核心业务逻辑）
     * @param card 要移动的卡牌数据模型（shared_ptr 确保生命周期安全）
     * @param targetPos 目标位置（通常是底牌堆顶部的坐标）
     * 
     * @details 执行流程：
     * 1. 计算新的 Z 序（确保移动的卡牌显示在最上层）
     * 2. 调用 GameLogicService 更新 Model 数据（位置、Z序）
     * 3. 更新 StackController 的顶部卡牌引用
     * 4. 在 GameView 中找到对应的 CardView，执行移动动画
     * 
     * @example
     * ```cpp
     * auto card = _gameModel->getCardById(123);
     * Vec2 discardPilePos(540, 200); // 底牌堆中心位置
     * _gameController->performMoveCard(card, discardPilePos);
     * ```
     * 
     * @note 此方法体现了 MVC 的分层：
     *       - Service 层更新数据（无状态）
     *       - Controller 协调逻辑
     *       - View 层只负责动画
     */
    void performMoveCard(std::shared_ptr<CardModel> card, const cocos2d::Vec2& targetPos);
        /**
     * @brief 获取备用牌堆控制器（提供给子控制器调用）
     * @return StackController 指针（不转移所有权）
     * 
     * @details 使用场景：
     * - PlayFieldController 需要查询当前底牌堆顶部卡牌
     * - UndoManager 需要恢复底牌堆状态
     * 
     * @example
     * ```cpp
     * auto topCard = _gameController->getStackController()->getTopCard();
     * if (topCard) {
     *     // 检查是否可以与顶部卡牌匹配
     * }
     * ```
     */
    StackController* getStackController() const { return _stackController; }
        /**
     * @brief 处理撤销按钮点击事件
     * 
     * @details 执行流程：
     * 1. 检查 UndoManager 是否有可撤销的操作
     * 2. 从撤销栈中弹出最后一次操作的命令数据（UndoCommand）
     * 3. 根据命令数据恢复卡牌的位置、Z序、状态
     * 4. 更新底牌堆顶部引用为撤销前的卡牌
     * 5. 在视图层播放反向动画
     * 
     * @note 撤销机制：
     * - 基于命令模式 (Command Pattern)
     * - 每次操作前记录快照数据到 UndoCommand
     * - 撤销时根据快照恢复状态
     * 
     * @see UndoManager::popCommand()
     * @see UndoCommand 结构定义
     */
    void onUndoClicked();

protected:
    // ==================== 生命周期管理 ====================
    
    /**
     * @brief 构造函数（保护，防止外部直接 new）
     * @details 初始化所有指针为 nullptr，避免野指针
     * @note 遵循 Cocos2d-x 的两阶段构造模式（构造 + init）
     */
    GameController();
        /**
     * @brief 析构函数（虚析构，支持多态）
     * @details 释放子控制器的引用计数
     * @warning 使用 CC_SAFE_RELEASE 宏确保释放前检查指针有效性
     */
    virtual ~GameController();

private:
    // ==================== 私有方法 ====================
    
    /**
     * @brief 关卡初始化（私有实现）
     * @param levelId 关卡ID
     * 
     * @details 详细步骤（MVC 架构标准流程）：
     * 1. 加载静态配置 (Config 层)
     * 2. 生成运行时数据 (Service 层)
     * 3. 初始化管理器 (Manager 层)
     * 4. 初始化子控制器 (Controller 层)
     * 5. 构建视图层 (View 层)
     * 6. 视图与数据绑定
     * 7. 切换场景
     * 
     * @note 任何步骤失败都应该中断并记录日志
     * @see startGame() - 外部调用入口
     */
    void _initWithLevel(int levelId);

private:
    // ==================== 成员变量 ====================
    
    /**
     * @brief 游戏数据模型（智能指针管理）
     * @details 
     * - 存储所有卡牌的运行时状态（位置、Z序、翻开/覆盖）
     * - 使用 shared_ptr 允许多个控制器共享同一数据源
     * - 生命周期由最后一个持有者自动管理
     */
    std::shared_ptr<GameModel> _gameModel;
        /**
     * @brief 撤销管理器（智能指针管理）
     * @details 
     * - 维护操作历史栈（UndoCommand 队列）
     * - 提供 canUndo/popCommand 等接口
     * - 禁止实现为单例（遵循架构要求）
     */
    std::shared_ptr<UndoManager> _undoManager;

        /**
     * @brief 游戏主视图（原始指针 + Cocos2d-x 引用计数）
     * @details 
     * - 负责管理整个游戏界面的布局
     * - 包含背景、卡牌容器、UI 按钮等
     * - 通过 Scene 的引用计数自动管理生命周期
     * 
     * @warning 不要手动 delete，由 Scene 销毁时自动释放
     */
    GameView* _gameView;

        /**
     * @brief 备用牌堆控制器（原始指针 + 手动 retain/release）
     * @details 
     * - 负责管理抽牌区域的业务逻辑
     * - 处理翻牌操作、备用牌耗尽检测等
     * - 需要在构造函数中 retain，析构函数中 release
     */
    StackController* _stackController;
    
        /**
     * @brief 主牌区控制器（原始指针 + 手动 retain/release）
     * @details 
     * - 负责管理桌面卡牌的点击与匹配逻辑
     * - 判断卡牌是否可点击、是否与底牌匹配
     * - 需要在构造函数中 retain，析构函数中 release
     */
    PlayFieldController* _playFieldController;
};

#endif // GAME_CONTROLLER_H