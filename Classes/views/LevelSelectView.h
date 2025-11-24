#ifndef LEVEL_SELECT_VIEW_H
#define LEVEL_SELECT_VIEW_H

#include "cocos2d.h"


class LevelSelectView : public cocos2d::Scene {
public:

    CREATE_FUNC(LevelSelectView);

    virtual bool init();

private:

    void onLevelSelected(int levelId);
};

#endif // LEVEL_SELECT_VIEW_H