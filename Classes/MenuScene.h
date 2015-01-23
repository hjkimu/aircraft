#ifndef __MENU_SCENE_H__
#define __MENU_SCENE_H__

#include "cocos2d.h"

USING_NS_CC;

#define TAG_MENUITEM_PLAY           101
#define TAG_MENUITEM_QUIT           102

class MenuScene : public Layer
{
public:

	static Scene* createScene();

	virtual bool init();
	CREATE_FUNC(MenuScene);

	void initBG();
	void initMenu();

	void menuCallback(Ref *sender);
};

#endif