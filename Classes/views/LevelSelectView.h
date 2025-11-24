#ifndef LEVEL_SELECT_VIEW_H
#define LEVEL_SELECT_VIEW_H

#include "cocos2d.h"

/**
 * @brief 关卡选择界面
 * 游戏的入口场景，包含选择关卡的按钮
 */
class LevelSelectView : public cocos2d::Scene {
public:
    // 标准 Cocos 创建宏
    CREATE_FUNC(LevelSelectView);

    // 初始化函数
    virtual bool init();

private:
    // 处理点击事件
    void onLevelSelected(int levelId);
};

#endif // LEVEL_SELECT_VIEW_H