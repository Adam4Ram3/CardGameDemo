# Cocos2d-x Card Elimination Game

基于 Cocos2d-x 3.17 开发的纸牌消除游戏 (C++ MVC 架构)。

## ⚠️ 编译前必读 (Setup Guide)
为了减小仓库体积，本项目**未包含 cocos2d 引擎源码**。在运行前请执行以下步骤：
1.  确保你本地拥有 **Cocos2d-x 3.17** 版本。
2.  新建一个空的 Cocos 项目，或者从现有项目中**复制 `cocos2d` 文件夹**。
3.  将 `cocos2d` 文件夹粘贴到本项目的根目录下。
4.  使用 Visual Studio 2022 打开 `proj.win32/cardGame.sln` 即可编译。

## 🏛️ 架构设计
本项目采用 **分层 MVC + 命令模式** 设计，旨在实现高内聚低耦合：
* **Model**: 纯数据模型 (`CardModel`), 不含任何逻辑和渲染。
* **View**: 视图层 (`CardView`), 负责渲染和动画，基于 Node 组合模式。
* **Controller**: 业务控制层 (`GameController`), 协调逻辑。
* **Service**: 无状态服务层 (`GameLogicService`), 封装消除算法。
* **Manager**: 状态管理 (`UndoManager`), 基于 Command 模式实现无限回退。

## ✨ 核心功能
* **备用牌堆**: 支持循环发牌、视觉堆叠效果。
* **消除规则**: 经典 TriPeaks 玩法 (点数差1消除)，支持 K-A 循环消除。
* **回退系统**: 完整的动作回退，支持动画反向播放。
* **扩展性**: 代码结构支持轻松扩展新卡牌（如 Joker）或新道具。

## 📂 目录结构
* `Classes/` - 核心 C++ 源码
* `Resources/` - 游戏资源与关卡配置
* `proj.win32/` - Windows 工程文件

---
*Created for Technical Interview Task*