
# 纸牌消除游戏 (CardGame) 程序设计与架构文档

开发环境：Cocos2d-x 3.17 (C++11 Standard)

------



## 1. 架构概述 (Architecture Overview)



本项目采用严格的 **分层 MVC (Model-View-Controller)** 架构设计，并引入了 **Services (服务层)** 和 **Managers (管理层)** 以进一步解耦业务逻辑与数据状态。



### 1.1 设计原则



- **关注点分离 (Separation of Concerns)**：视图层不处理逻辑，逻辑层不持有视图。
- **数据驱动 (Data-Driven)**：游戏的所有表现均由 `Model` 的状态变更驱动。
- **无状态逻辑 (Stateless Logic)**：核心算法封装在纯静态的 Service 中，便于单元测试和复用。



### 1.2 系统分层图谱



Plaintext

```
[ View Layer ] <---(Observe/Update)--- [ Controller Layer ] ---(Call)---> [ Service/Manager Layer ]
      ^                                         |                                   |
      |                                         |                                   |
(Render/Input)                              (Coordinate)                      (Logic/State)
      |                                         |                                   |
      +---------------------------------> [ Model Layer ] <-------------------------+
                                         (Pure Data)
```

------



## 2. 模块详细设计 (Module Detail)





### 2.1 Models (数据模型层)



- **职责**：仅存储运行时数据，不包含任何业务逻辑或渲染代码。
- **核心类**：
  - `CardModel`: 存储单张卡牌的 ID、点数、花色、当前位置坐标、正反面状态。
  - `GameModel`: 聚合所有 `CardModel`，代表游戏全局数据快照。
  - `UndoCommand`: 定义回退操作的数据结构（命令模式的载体）。



### 2.2 Views (视图层)



- **职责**：负责 UI 渲染、动画播放及用户输入捕获。
- **核心类**：
  - `GameView`: 游戏主场景容器，负责背景绘制和 UI 布局。
  - `CardView`: 单张卡牌的实体，根据 Model 状态动态组装 Sprite（底板+花色+数字）。
- **通信机制**：通过 `std::function` 回调将点击事件向上传递给 Controller，自身不处理逻辑。



### 2.3 Controllers (控制器层)



- **职责**：协调层，连接 Model 与 View，响应用户事件。
- **核心类**：
  - `GameController`: 全局总控，负责关卡加载流程 (`startGame`) 和子控制器管理。
  - `StackController`: 专职管理备用牌堆和翻牌逻辑。
  - `PlayFieldController`: 专职管理桌面牌区和消除匹配逻辑。



### 2.4 Services (服务层) & Managers (管理层)



- **GameLogicService (Service)**：**无状态纯逻辑**。
  - 提供 `canMatch(cardA, cardB)` 算法。
  - 提供 `applyMove` 和 `applyStateChange` 接口修改 Model 数据。
- **UndoManager (Manager)**：**有状态**。
  - 维护 `std::stack<UndoCommand>` 历史记录栈，实现撤销功能。

------



## 3. 扩展性设计指南 (Scalability Guide)



本架构的核心优势在于高可扩展性。以下演示在当前架构下，如何应对未来的新需求。



### 场景一：如何添加一种新卡牌？（例如：“万能牌/Joker”）



**需求描述**：添加一张“Joker”牌，它可以与任何牌匹配消除。

**实现步骤**：

1. **数据定义 (`configs/GameConsts.h`)**:
   - 在 `CardFaceType` 枚举中添加 `CFT_JOKER`。
   - 在 `CardSuitType` 枚举中添加 `CST_JOKER`。
2. **资源映射 (`views/CardView.cpp`)**:
   - 修改 `getNumberFilename` 和 `getSuitFilename`。
   - 增加 `case CFT_JOKER:` 的分支，返回 Joker 专用的图片路径。
   - *优势*：无需修改 View 的渲染逻辑，只需配置资源路径。
3. **核心逻辑 (`services/GameLogicService.cpp`)**:
   - 修改 `canMatch` 函数。
   - 添加规则：`if (faceA == CFT_JOKER || faceB == CFT_JOKER) return true;`
   - *优势*：**Controller 层完全不需要修改**。PlayFieldController 会自动调用更新后的 `canMatch`，Joker 的逻辑立刻生效。

------



### 场景二：如何添加一个新的回退功能？（例如：“回退洗牌”）



**需求描述**：当前回退仅支持“移动/翻牌”。未来如果增加了“洗牌”道具，需要能回退“洗牌”操作。

**实现步骤**：

1. **扩展命令结构 (`models/UndoModel.h`)**:

   - 引入命令类型枚举：`enum class CommandType { MOVE, SHUFFLE };`
   - 修改 `UndoCommand` 结构体，添加 `CommandType type` 字段。
   - *扩展*：如果是洗牌操作，需要在 Command 里存储洗牌前所有卡牌的 ID 顺序列表。

2. **记录操作 (`controllers/GameController.cpp`)**:

   - 在执行洗牌逻辑的地方，构建一个 `type = SHUFFLE` 的 `UndoCommand` 并压入 `UndoManager`。

3. **执行回退 (`controllers/GameController.cpp`)**:

   - 修改 `onUndoClicked` 函数，使用 switch-case 处理类型：

   C++

   ```
   UndoCommand cmd = _undoManager->popCommand();
   switch (cmd.type) {
       case CommandType::MOVE:
           // 执行原本的卡牌移动恢复逻辑
           break;
       case CommandType::SHUFFLE:
           // 执行新的逻辑：根据 cmd 里记录的顺序列表，重新排列所有卡牌
           break;
   }
   ```

   - *优势*：利用 **命令模式 (Command Pattern)**，UndoManager 本身不需要关心具体的业务，只负责存储数据。具体的恢复逻辑由 Controller 根据命令类型分发。

------



## 4. 核心流程时序 (Core Workflows)





### 4.1 游戏初始化流程



1. **User** 选择关卡。
2. **GameController** 调用 `LevelConfigLoader` 读取 JSON。
3. **GameModelFromLevelGenerator** 将 JSON 转换为 `GameModel`。
4. **GameController** 初始化 `StackController` 和 `PlayFieldController`。
5. **StackController** 遍历 Model，筛选属于自己的牌，通过 `GameView` 创建对应的 `CardView`。



### 4.2 卡牌消除流程



1. **User** 点击桌面卡牌 -> `CardView` 捕获点击 -> 回调 `PlayFieldController`。
2. **PlayFieldController** 从 `StackController` 获取当前顶牌。
3. **PlayFieldController** 调用 `GameLogicService::canMatch` 判断是否可消除。
4. **若匹配**：
   - 调用 `UndoManager` 记录当前状态。
   - 调用 `GameController::performMoveCard`。
   - `GameController` 委托 `GameLogicService` 修改 Model 数据。
   - `GameController` 指挥 `GameView` 播放 MoveTo 动画。

------



## 5. 总结 (Conclusion)



本程序设计文档展示了一个基于 C++ 和 Cocos2d-x 的高内聚、低耦合的游戏系统。

- **维护性**：UI 修改仅需动 View，规则修改仅需动 Service。
- **稳定性**：通过智能指针 (`std::shared_ptr`) 管理内存，杜绝了野指针风险。
- **扩展性**：基于数据驱动和命令模式的设计，使得新增卡牌类型和操作类型变得简单且风险可控。
