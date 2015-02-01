#include "OptionLayer.h"

bool OptionLayer::init()
{
	if (!Layer::init())
	{
		return false;
	}

	CCLOG("OptionLayer::init()");

	winSize = Director::getInstance()->getWinSize();
   
    initBG();
    initMenu();
    
	return true;
}

void OptionLayer::initBG()
{
	auto spr = Sprite::create("menu/menu-bg.png");
	spr->setPosition(Point(winSize.width/2, winSize.height/2));
    spr->setScale(1.0, 1.0);
	this->addChild(spr, 300);
}

void OptionLayer::initMenu()
{
	auto item_0 = MenuItemImage::create("option/btn_sound_on.png", "option/btn_sound_on.png", CC_CALLBACK_1(OptionLayer::menuCallback, this));
	auto item_1 = MenuItemImage::create("option/btn_sound_off.png", "option/btn_sound_off.png", CC_CALLBACK_1(OptionLayer::menuCallback, this));
	auto item_2 = MenuItemImage::create("option/btn_close.png", "option/btn_close.png", CC_CALLBACK_1(OptionLayer::menuCallback, this));
	auto item_3 = MenuItemImage::create("option/btn_retry.png", "option/btn_retry.png", CC_CALLBACK_1(OptionLayer::menuCallback, this));
	auto item_4 = MenuItemImage::create("option/btn_quit.png", "option/btn_quit.png", CC_CALLBACK_1(OptionLayer::menuCallback, this));

	item_0->setTag(TAG_OPTION_SOUND_ON);
	item_1->setTag(TAG_OPTION_SOUND_OFF);
	item_2->setTag(TAG_OPTION_CLOSE);
	item_3->setTag(TAG_OPTION_RETRY);
	item_4->setTag(TAG_OPTION_QUIT);
    
	auto menu = Menu::create(item_0, item_1, item_2, item_3, item_4, NULL);
	menu->alignItemsVerticallyWithPadding(20);
	this->addChild(menu, 300);

}

void OptionLayer::menuCallback(Ref *sender)
{
	auto item = (MenuItem*)sender;

	switch (item->getTag())
	{
	case TAG_OPTION_SOUND_ON:
	{
        this->removeAllChildren();
	}
		break;
            
	case TAG_OPTION_SOUND_OFF:
	{
        this->removeAllChildren();
	}
		break;
            
	case TAG_OPTION_CLOSE:
	{
        Director::getInstance()->resume();
        this->removeAllChildren();
	}
		break;
            
	case TAG_OPTION_RETRY:
	{

	}
		break;
            
	case TAG_OPTION_QUIT:
		Director::getInstance()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
		exit(0);
#endif
		break;
	default:
		break;
	}
}

