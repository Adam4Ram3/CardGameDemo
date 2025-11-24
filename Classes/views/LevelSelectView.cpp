#include "views/LevelSelectView.h"
#include "controllers/GameController.h" // �������ã��Ա���� startGame
#include "ui/CocosGUI.h" // ���� UI �����

using namespace cocos2d;
using namespace cocos2d::ui;

bool LevelSelectView::init() {
    if (!Scene::init()) return false;

    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 1. ������ɫ (���ɫ)
    auto bg = LayerColor::create(Color4B(50, 50, 50, 255));
    this->addChild(bg);

    // 2. �����ı�
    auto label = Label::createWithSystemFont("SELECT LEVEL", "Arial", 60);
    label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.7));
    this->addChild(label);

    // 3. ��������1�ء���ť
    // ����ʹ�� Cocos �Դ��� Button �ؼ�
    // ����1: ����ͼƬ, ����2: ����ͼƬ, ����3: ����ͼƬ (����ֻ�����֣����մ�)
    auto btnLevel1 = Button::create();
    btnLevel1->setTitleText("Level 1");
    btnLevel1->setTitleFontSize(50);
    btnLevel1->setTitleColor(Color3B::WHITE);
    // ��һ���򵥵ı���ͼ���߷Ŵ�ߴ��Ա���
    btnLevel1->setScale(2.0f);
    btnLevel1->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.5));

    // ���ӵ������
    btnLevel1->addClickEventListener([this](Ref* sender) {
        this->onLevelSelected(1); // ����ؿ� ID = 1
        });

    this->addChild(btnLevel1);

    return true;
}

void LevelSelectView::onLevelSelected(int levelId) {
    CCLOG("UI: User selected Level %d", levelId);

    // ���� GameController �ľ�̬��ڿ�ʼ��Ϸ
    // GameController ���Զ������³�����ִ�� replaceScene
    GameController::startGame(levelId);
}