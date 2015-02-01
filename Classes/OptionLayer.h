#ifndef __OPTION_LAYER_H__
#define __OPTION_LAYER_H__

#include "cocos2d.h"

USING_NS_CC;

class OptionLayer : public Layer
{
public:

	virtual bool init();
	CREATE_FUNC(OptionLayer);
    
    Size winSize;
    
	void initBG();
    void initMenu();

	void menuCallback(Ref *sender);
};

#endif