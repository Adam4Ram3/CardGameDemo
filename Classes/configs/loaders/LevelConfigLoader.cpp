/**
 * @file LevelConfigLoader.cpp
 * @brief 关卡配置加载器实现 - Config 层的核心模块
 * 
 * @details 职责：
 * 1. 从 JSON 文件读取关卡配置数据
 * 2. 解析 JSON 结构并转换为 LevelConfig 对象
 * 3. 提供统一的配置加载接口给 Controller 层
 * 
 * @note 架构定位：
 * - 属于 Config 层，只负责"读取和解析"静态配置
 * - 不涉及业务逻辑，不持有运行时数据
 * - 可以提供为静态方法（无状态服务）
 * 
 * @warning 依赖：
 * - 使用 RapidJSON 进行 JSON 解析（Cocos2d-x 内置）
 * - 文件路径必须存在于 Resources 目录中
 */
#include "LevelConfigLoader.h"
#include "cocos2d.h"
#include "json/rapidjson.h"
#include "json/document.h"

using namespace cocos2d;

/**
 * @brief 加载关卡配置文件（静态方法）
 * @param filename 配置文件路径，相对于 Resources 目录
 *                 例如："levels/level_1.json"
 * 
 * @return LevelConfig 对象，包含主牌区和备用牌堆的配置数据
 * 
 * @details 执行流程：
 * 1. **读取文件**：通过 FileUtils 读取 JSON 字符串
 * 2. **解析 JSON**：使用 RapidJSON 将字符串转换为 Document 对象
 * 3. **提取数据**：遍历 "Playfield" 和 "Stack" 数组，解析每张卡牌
 * 4. **返回结果**：封装为 LevelConfig 结构体返回
 * 
 * @note 错误处理：
 * - 如果文件不存在，返回空的 LevelConfig（playFieldCards 和 stackCards 为空）
 * - 如果 JSON 解析失败，同样返回空配置
 * - 调用方应检查返回的配置是否为空
 * 
 * @example
 * ```cpp
 * LevelConfig config = LevelConfigLoader::loadLevelConfig("levels/level_1.json");
 * if (config.playFieldCards.empty()) {
 *     CCLOG("Failed to load level config");
 * }
 * ```
 * 
 * @see LevelConfig - 配置数据结构定义
 * @see CardConfigData - 单张卡牌配置数据
 */
LevelConfig LevelConfigLoader::loadLevelConfig(const std::string& filename) {
    LevelConfig config;

    // ========== 步骤1: 读取文件内容 ==========
    // FileUtils::getInstance() 是 Cocos2d-x 的文件管理单例
    // getStringFromFile() 会自动搜索 Resources 目录
    std::string jsonContent = FileUtils::getInstance()->getStringFromFile(filename);
    if (jsonContent.empty()) {
        CCLOG("LevelConfigLoader: Failed to read file %s", filename.c_str());
        return config;
    }

    // ========== 步骤2: 解析 JSON 字符串 ==========
    rapidjson::Document doc;
    doc.Parse(jsonContent.c_str());

    // JSON 语法错误检测（例如缺少逗号、括号不匹配等）
    if (doc.HasParseError()) {
        CCLOG("LevelConfigLoader: Parse error in %s", filename.c_str());
        return config;
    }

    // ========== 步骤3: 解析 "Playfield" 数组（主牌区配置）==========
    // 检查 JSON 中是否存在 "Playfield" 字段，且为数组类型
    if (doc.HasMember("Playfield") && doc["Playfield"].IsArray()) {
        const auto& playFieldArray = doc["Playfield"];
        
        // 遍历数组中的每个元素（每个元素代表一张卡牌）
        for (const auto& item : playFieldArray.GetArray()) {
            // 调用辅助方法解析单张卡牌的数据
            config.playFieldCards.push_back(parseCardNode(&item));
        }
    }

    // ========== 步骤4: 解析 "Stack" 数组（备用牌堆配置）==========
    // 与 Playfield 解析逻辑相同
    if (doc.HasMember("Stack") && doc["Stack"].IsArray()) {
        const auto& stackArray = doc["Stack"];
        for (const auto& item : stackArray.GetArray()) {
            config.stackCards.push_back(parseCardNode(&item));
        }
    }

    // ========== 步骤5: 输出加载日志 ==========
    // 记录成功加载的卡牌数量，便于调试
    CCLOG("LevelConfigLoader: Loaded %s, Playfield: %d, Stack: %d",
        filename.c_str(), (int)config.playFieldCards.size(), (int)config.stackCards.size());

    return config;
}

/**
 * @brief 解析单张卡牌的 JSON 节点（私有辅助方法）
 * @param jsonValuePtr 指向 RapidJSON Value 对象的指针（类型擦除，避免头文件依赖）
 * 
 * @return CardConfigData 对象，包含卡牌的点数、花色、位置
 * 
 * @details JSON 节点结构示例：
 * ```json
 * {
 *     "CardFace": 12,       // 点数（0=A, 12=K，参见 CardFaceType 枚举）
 *     "CardSuit": 0,        // 花色（0=梅花, 1=方块, 2=红桃, 3=黑桃）
 *     "Position": {
 *         "x": 250,         // X 坐标（屏幕坐标系，左下角为原点）
 *         "y": 1000         // Y 坐标
 *     }
 * }
 * ```
 * 
 * @note 设计考量：
 * - 使用 void* 而不是直接传 rapidjson::Value& 的原因：
 *   避免在头文件中包含 RapidJSON，减少编译依赖
 * - 所有字段都提供默认值，防止 JSON 缺少字段时崩溃
 * 
 * @warning 
 * - 传入的指针必须指向有效的 rapidjson::Value 对象
 * - 调用方负责确保指针的生命周期
 * 
 * @see CardConfigData - 卡牌配置数据结构
 * @see CardFaceType - 点数枚举定义
 * @see CardSuitType - 花色枚举定义
 */

CardConfigData LevelConfigLoader::parseCardNode(const void* jsonValuePtr) {

    // ========== 类型还原 ==========
    // 将 void* 强制转换回 rapidjson::Value&（在 .cpp 中允许这样做）
    const rapidjson::Value& item = *static_cast<const rapidjson::Value*>(jsonValuePtr);
    CardConfigData data;

    // ========== 解析点数字段 "CardFace" ==========
    // 检查字段是否存在，避免访问不存在的键导致异常
    if (item.HasMember("CardFace")) {
        // GetInt() 读取整数值，然后强制转换为 CardFaceType 枚举
        data.face = static_cast<CardFaceType>(item["CardFace"].GetInt());
    }
    // 如果字段不存在，使用 CardConfigData 构造函数的默认值（CFT_NONE）

    // ========== 解析花色字段 "CardSuit" ==========
    if (item.HasMember("CardSuit")) {
        data.suit = static_cast<CardSuitType>(item["CardSuit"].GetInt());
    }
    // 默认值：CST_NONE

    // ========== 解析位置对象 "Position" ==========
    // 位置是一个嵌套对象，需要额外检查 IsObject()
    if (item.HasMember("Position") && item["Position"].IsObject()) {
        const auto& posObj = item["Position"];

        // 提取 x 坐标，如果不存在则使用 0.0f
        float x = posObj.HasMember("x") ? posObj["x"].GetFloat() : 0.0f;

        // 提取 y 坐标，如果不存在则使用 0.0f
        float y = posObj.HasMember("y") ? posObj["y"].GetFloat() : 0.0f;

        // 构造 Cocos2d-x 的 Vec2 对象
        data.position = Vec2(x, y);
    }
    // 默认值：Vec2(0, 0)
    return data;
}