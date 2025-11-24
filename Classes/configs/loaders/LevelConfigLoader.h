#ifndef LEVEL_CONFIG_LOADER_H
#define LEVEL_CONFIG_LOADER_H

#include "configs/models/LevelConfig.h"
#include <string>

/**
 * @brief 静态配置加载器
 * 职责：负责读取 JSON 文件并解析为 LevelConfig 结构体
 */
class LevelConfigLoader {
public:
    /**
     * 加载指定关卡的配置
     * @param filename json文件路径 (例如 "levels/level_1.json")
     * @return 解析后的关卡配置对象
     */
    static LevelConfig loadLevelConfig(const std::string& filename);

private:
    // 辅助函数：解析单个卡牌的 JSON 对象
    // 使用 void* 是为了避免在头文件中包含 rapidjson 依赖，保持头文件清洁
    // 在 .cpp 实现中会强制转换为 const rapidjson::Value&
    static CardConfigData parseCardNode(const void* jsonValue);
};

#endif // LEVEL_CONFIG_LOADER_H