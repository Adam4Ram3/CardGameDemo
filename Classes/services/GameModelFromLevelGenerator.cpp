#include "GameModelFromLevelGenerator.h"

std::shared_ptr<GameModel> GameModelFromLevelGenerator::generateGameModel(const LevelConfig& config) {
    auto gameModel = std::make_shared<GameModel>();

    // 全局 ID 计数器，确保每个 CardModel 都有唯一 ID
    int globalIdCounter = 0;

    // 1. 处理主牌区 (Playfield)
    for (const auto& configData : config.playFieldCards) {
        auto cardModel = std::make_shared<CardModel>();

        // 初始化运行时数据
        cardModel->init(
            globalIdCounter++,  // 分配 ID 并自增
            configData.face,
            configData.suit,
            configData.position
        );

        // 默认状态设置：主牌区通常根据逻辑可能有正面或背面，
        // 需求文档提到"翻开或覆盖状态"，这里我们默认先全部覆盖(FACE_DOWN)，
        // 或者根据某种规则(比如位于最上层)设为 FACE_UP。
        // *为了简化，初始全部设为 FACE_UP 方便调试，后续在 LogicService 里根据层级遮挡关系修正状态*
        cardModel->setState(CardState::FACE_UP);

        gameModel->addCard(cardModel);
    }

    // 2. 处理堆牌区 (Stack)
    for (const auto& configData : config.stackCards) {
        auto cardModel = std::make_shared<CardModel>();

        cardModel->init(
            globalIdCounter++,
            configData.face,
            configData.suit,
            configData.position
        );

        // 堆牌区通常只有顶上一张是翻开的，其余是背面
        // 具体逻辑由 StackController 接管初始化，这里暂存数据
        cardModel->setState(CardState::FACE_DOWN);

        gameModel->addCard(cardModel);
    }

    return gameModel;
}