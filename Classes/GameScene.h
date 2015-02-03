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
#define TAG_LABEL_DISTANCE              1006
#define TAG_LABEL_BOMB_STOCK            1007
#define TAG_PARTICLE_BOMB               1008

#define TAG_MENU                        2000
#define TAG_MENUITEM_PLAY               2001
#define TAG_MENUITEM_QUIT               2002

#define TAG_OPTION                      2100
#define TAG_OPTION_BG                   2101
#define TAG_OPTION_SOUND_ON             2102
#define TAG_OPTION_SOUND_OFF            2103
#define TAG_OPTION_CLOSE                2104
#define TAG_OPTION_RETRY                2105
#define TAG_OPTION_QUIT                 2106


#define PADDING_SCREEN					10
#define DEFAULT_HITPOINT            3
#define MAX_STAGE            4 

#define ENEMY_1_HP           1
#define ENEMY_2_HP           5
#define ENEMY_3_HP           10
#define ENEMY_4_HP           20
#define ENEMY_5_HP           50
#define BOSS_1_HP            100

#define MISSILE_SCORE        1
#define ENEMY_SCORE          100
#define BOSS_SCORE           500

#define ITEM_A               1
#define ITEM_B               2
#define ITEM_C               3
#define ITEM_D               4
#define ITEM_E               5
#define ITEM_F               6 

#define MISSILE_A            1
#define MISSILE_B            2
#define MISSILE_C            3
#define MISSILE_D            4

#define BASIC_POWER            1
#define MISSILE_A_POWER        3
#define MISSILE_B_POWER        5
#define MISSILE_C_POWER        10
#define MISSILE_D_POWER        10

#define BOMB_E_POWER           1
#define BOMB_E_TIME            1 

#define SOUND_MISSILE_1       1
#define SOUND_CRASH_1         2


class GameScene : public Layer
{
public:

	static Scene* createScene();

	virtual bool init();
	CREATE_FUNC(GameScene);

	Size winSize;
	Point posStartTouch, posStartPlayer;

	Vector<Sprite*> items, enemies, missiles, bosses;
    
	bool isPlayable, isOnUsingItem, isOnLoading, isOnBossBattle;
    int curStageNum, tabCnt;
	int itemType, missileType;
	int nScore, nScoreHigh, nHitPoint, bombStock;
    float playTime, distance, tabTime, bombTime;
    float itemFrequency, enemyFrequency;
    bool soundFlg;

	void update(float delta);

	void resetBoom(Ref *sender);

	void initData();
    void resetData();
    
	void initStage(int stage);
    void resetStage(Ref *sender);
    int getStageNum(int distance);

	void initBG();
	void initPlayer();

	void initBombStock();
    void updateBomb(int amount);
    void useBomb();
    void setBombEffect(Ref *sender);
    void endUseBomb(Ref *sender);

	void initScore();
	void addScore(int add);

    void initDistance();
    void updateDistance();
    
	void setMissile(float delta);
	void resetMissile(Ref *sender);
    
	void setPlayerPower(float delta);
	void resetPlayerPower(Ref *sender);

	void setItem(float delta);
	void resetItem(Ref *sender);
    void setItemEffect(int itemType);

	void setEnemy(float delta);
	void resetEnemy(Ref *sender);
	void destroyEnemy(Ref *sender);

	void setBoss(int stage);
    void resetBoss(Ref *sender);
    void destroyBoss(Ref *sender);
    
    void clearBoss(bool addScoreFlg);
    void clearEnemy(bool addScoreFlg);
    void clearItem();
    
    void removeLabel(Ref *sender);
    
    void initHitPoint();
    void updateHitPoint(int hp);
    void setDamaged();
    
    void endGame();
    void retryGame();
    
    void playSound(int type);
    
    void displayResult();
    void displayScore();
    void displayMenu();
     
	bool onTouchBegan(Touch *touch, Event *unused_event);
	void onTouchMoved(Touch *touch, Event *unused_event);
   
    void menuCallback(Ref *sender);
    
    void displayOption();
    void showOption();
    void hideOption();

    void setEnemy1();
    void setEnemy2();
    void setEnemy3();
    void setEnemy4();
    void setEnemy5();
    
};

#endif