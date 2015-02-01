#ifndef __MENU_SCENE_H__
#define __MENU_SCENE_H__

#include "cocos2d.h"

USING_NS_CC;

class MenuScene : public Layer
{
public:

	static Scene* createScene();

	virtual bool init();
	CREATE_FUNC(MenuScene);
    
    Size winSize;
    
	void initBG();
	void initMenu();

	void menuCallback(Ref *sender);
};

#endif