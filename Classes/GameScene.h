#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"

USING_NS_CC;


#define TAG_SPRITE_PLAYER               1000
#define TAG_LABEL_SCORE                 1001
#define TAG_LABEL_HIGHSCORE             1002
#define TAG_LABEL_HITPOINT              1003
#define TAG_LABEL_FINISH                1004
#define TAG_LABEL_RESULT_SCORE          1005

#define TAG_MENU                        100
#define TAG_MENUITEM_PLAY               101
#define TAG_MENUITEM_QUIT               102

#define PADDING_SCREEN					10
#define TAG_DEFAULT_HITPOINT            3

class GameScene : public Layer
{
public:

	static Scene* createScene();

	virtual bool init();
	CREATE_FUNC(GameScene);

	Size winSize;
	Point posStartTouch, posStartPlayer;

	Vector<Sprite*> items, enemies, missiles;

	bool isGetItem;
	bool isPowerUp;
	bool isPlayable;
	int nScore, nScoreHigh;
    int nHitPoint;
    float itemFrequency, enemyFrequency;

	void update(float delta);

	void resetBoom(Ref *sender);

	void resetGetItem(float delta);

	void initData();

	void initBG();
	void initPlayer();

	void initScore();
	void addScore(int add);

	void setMissile(float delta);
	void resetMissile(Ref *sender);
    
	void setPlayerPower(float delta);
	void resetPlayerPower(Ref *sender);

	void setItem(float delta);
	void resetItem(Ref *sender);

	void setEnemy(float delta);
	void resetEnemy(Ref *sender);

    void initHitPoint();
    void reduceHitPoint(int damage);
    void setDamaged();
    
    void endGame();
    
	bool onTouchBegan(Touch *touch, Event *unused_event);
	void onTouchMoved(Touch *touch, Event *unused_event);
    
    void displayResult();
    void displayScore();
    void displayMenu();
    
    void menuCallback(Ref *sender);
};

#endif