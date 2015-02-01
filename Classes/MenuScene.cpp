#include "MenuScene.h"
#include "GameScene.h"


Scene* MenuScene::createScene()
{
	auto scene = Scene::create();

	auto layer = MenuScene::create();
	scene->addChild(layer);

	return scene;
}

bool MenuScene::init()
{
	if (!Layer::init())
	{
		return false;
	}

	winSize = Director::getInstance()->getWinSize();
    
	initBG();
	initMenu();

    
	return true;
}

void MenuScene::initBG()
{
	auto spr = Sprite::create("menu/menu-bg.png");
	spr->setPosition(Point(winSize.width/2, winSize.height/2));
    spr->setScale(2.5, 2.5);
	this->addChild(spr);
}

void MenuScene::initMenu()
{
	auto item_0 = MenuItemImage::create("menu/play-0.png", "menu/play-1.png", CC_CALLBACK_1(MenuScene::menuCallback, this));
	auto item_1 = MenuItemImage::create("menu/quit-0.png", "menu/quit-1.png", CC_CALLBACK_1(MenuScene::menuCallback, this));

	item_0->setTag(TAG_MENUITEM_PLAY);
	item_1->setTag(TAG_MENUITEM_QUIT);

	auto menu = Menu::create(item_0, item_1, NULL);
	menu->alignItemsVerticallyWithPadding(20);
	this->addChild(menu);

    auto label = Label::createWithSystemFont("指で飛行機を操作 \nBomb攻撃はダブルタップで", "", 30);
	label->setPosition(Point(winSize.width/2, winSize.height/3 ));
    label->setColor(Color3B::BLACK);
    this->addChild(label);
}

void MenuScene::menuCallback(Ref *sender)
{
	auto item = (MenuItem*)sender;

	switch (item->getTag())
	{
	case TAG_MENUITEM_PLAY:
	{
		auto scene = TransitionFadeTR::create(1.0, GameScene::createScene());
		Director::getInstance()->replaceScene(scene);
	}
		break;
	case TAG_MENUITEM_QUIT:
		Director::getInstance()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
		exit(0);
#endif
		break;
	default:
		break;
	}
}

