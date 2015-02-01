#include "GameScene.h"
#include "SimpleAudioEngine.h"

using namespace CocosDenshion;

Scene* GameScene::createScene()
{
	auto scene = Scene::create();

	auto layer = GameScene::create();
	scene->addChild(layer);

	return scene;
}

bool GameScene::init()
{
	if (!Layer::init())
	{
		return false;
	}

	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(GameScene::onTouchMoved, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(listener, 1);
    
	winSize = Director::getInstance()->getWinSize();
    soundFlg = true;
    
	initBG();
	initScore();
    initHitPoint();
    initBombStock();
    initDistance();
    displayOption();
    
	initData();
    
	this->scheduleUpdate();
    
    //プレイが一瞬止まることを防ぐため一回実行
    SimpleAudioEngine::getInstance()->playEffect("sound/crash_1.wav", false, 1.0, 0.0, 0.0);
    SimpleAudioEngine::getInstance()->playEffect("sound/missile_4.wav", false, 1.0, 0.0, 0.0);

	return true;
}

void GameScene::update(float delta)
{
    playTime += delta;

    if ( !isPlayable) {
        return;
    }
   
    if ( !isOnBossBattle ) {
        distance += delta*10;
        updateDistance();
        
        int _stageNum = getStageNum(distance);
        if ( curStageNum < _stageNum) {
            CCLOG("curStage:%d, newStage:%d", curStageNum, _stageNum);
            //initStage( _stageNum );
            setBoss(curStageNum);
        }
    }
    
	auto sprPlayer = (Sprite*)this->getChildByTag(TAG_SPRITE_PLAYER);
	Rect rectPlayer = sprPlayer->getBoundingBox();

	auto removeSpr = Sprite::create();

	for (Sprite* sprItem : items) {

		Rect rectItem = sprItem->getBoundingBox();

		if (rectPlayer.intersectsRect(rectItem)) {
			removeSpr = sprItem;
		}
	}

	if (items.contains(removeSpr)) {
        setItemEffect(removeSpr->getTag());
		resetItem(removeSpr);
	}

	auto removeMissile = Sprite::create();
	auto removeEnemy = Sprite::create();
	auto removeBoss = Sprite::create();

	for (Sprite* sprMissile : missiles) {

		Rect rectMissile = sprMissile->getBoundingBox();

		for (Sprite* sprEnemy : enemies) {

			Rect rectEnemy = Rect(sprEnemy->getPositionX()-15, sprEnemy->getPositionY()-15, 30, 30);

            int hp = 0;
			if (rectMissile.intersectsRect(rectEnemy)) {

				int attack = sprMissile->getTag();
				hp = sprEnemy->getTag();

				removeMissile = sprMissile;

				if (hp-attack>0) {
					sprEnemy->setTag(hp - attack);
					addScore(MISSILE_SCORE);
				}
				else {
					removeEnemy = sprEnemy;
				}
			}
           
		}

        for (Sprite* sprBoss: bosses) {
			Rect rectBoss = Rect(sprBoss->getPositionX()-35, sprBoss->getPositionY()-35, 70, 70);
            
            int hp = 0;
            if (rectMissile.intersectsRect(rectBoss)) {
                
                int attack = sprMissile->getTag();
                hp = sprBoss->getTag();
                
                removeMissile = sprMissile;

				if (hp-attack>0) {
					sprBoss->setTag(hp - attack);
					addScore(MISSILE_SCORE);
				}
				else {
					removeBoss = sprBoss;
				}
            }

        }
	}
 
    for (Sprite* sprEnemy : enemies) {

        // 爆弾効果
        // 通常の敵は全部消す,ここでは実装しない
        
        if ( ! isOnUsingItem ) {
    		Rect rectEnemy = Rect(sprEnemy->getPositionX()-15, sprEnemy->getPositionY()-15, 30, 30);
            if (rectPlayer.intersectsRect(rectEnemy)) {
                removeEnemy = sprEnemy;
                reduceHitPoint(1);
                CCLOG("remains hit point:%d", nHitPoint);
                if ( 0 >= nHitPoint ) {
                    endGame();
                }
            }
        }
    }
    for (Sprite* sprBoss : bosses) {

        // 爆弾効果
        // ボスはHPを減らす
        int hp = 0;
        if ( playTime < bombTime ) {
            hp = sprBoss->getTag();
            
            if ( hp - BOMB_E_POWER > 0 ) {
                sprBoss->setTag(hp - BOMB_E_POWER);
                addScore(MISSILE_SCORE);
            } else {
                removeBoss = sprBoss;
            }
        }
        
        if ( ! isOnUsingItem ) {
    		Rect rectBoss = Rect(sprBoss->getPositionX()-35, sprBoss->getPositionY()-35, 70, 70);
            if (rectPlayer.intersectsRect(rectBoss)) {
                removeBoss = sprBoss;
                reduceHitPoint(1);
                CCLOG("remains hit point:%d", nHitPoint);
                if ( 0 >= nHitPoint ) {
                    endGame();
                }
            }
        }
    }

	if (missiles.contains(removeMissile)) {
		resetMissile(removeMissile);
	}

	if (enemies.contains(removeEnemy)) {
		destroyEnemy(removeEnemy);
        addScore(ENEMY_SCORE);
	}
 
	if (bosses.contains(removeBoss)) {
		destroyBoss(removeBoss);
        addScore(BOSS_SCORE);
	}   

}

void GameScene::resetBoom(Ref *sender)
{
	auto particle = (ParticleSystemQuad*)sender;

	this->removeChild(particle);
}

void GameScene::initData()
{
    CCLOG("initData");

    clearBoss(false);
    clearItem();
    clearEnemy(false);

    curStageNum = 1;
    missileType = 0;
	isPlayable = true;
	isOnUsingItem = false;
	isOnLoading = false;
    isOnBossBattle = false;
	nScore = 0;
	nScoreHigh = UserDefault::getInstance()->getIntegerForKey("HIGH_SCORE", 0);
    nHitPoint = 3;
    
    bombStock = 3;
    tabTime = 0;
    playTime = 0;
    bombTime = -100;
    distance = 0;
   
    initStage(curStageNum);
 
    initPlayer();
    reduceHitPoint(0);
    addScore(0);
    updateBomb(0);
 
}

void GameScene::initStage(int stage)
{
    isOnLoading = true;
    auto action_1 = Sequence::create(
                                   Blink::create(1.0, 4),
                                   DelayTime::create(0.5),
                                   CallFuncN::create(CC_CALLBACK_1(GameScene::resetStage, this)),
                                   NULL
    );

    auto label_1 = Label::createWithSystemFont( StringUtils::format("STAGE %d",stage), "", 80 );
	label_1->setPosition(Point(winSize.width / 2, winSize.height / 3 * 2 ));
	label_1->setColor(Color3B::BLACK);
    label_1->runAction(action_1);
    
    this->addChild(label_1, 200) ;
   
    //ここにnew stageでのデーター初期化
    curStageNum = stage;
  
    this->unschedule(schedule_selector(GameScene::setMissile));
    this->unschedule(schedule_selector(GameScene::setItem));
    this->unschedule(schedule_selector(GameScene::setEnemy));
    
    
    float item_interval = 5.0;
    float enemy_interval = (std::max)(1.0 / stage, 0.01);
	this->schedule(schedule_selector(GameScene::setMissile), 0.2);
	this->schedule(schedule_selector(GameScene::setItem), item_interval);
    this->schedule(schedule_selector(GameScene::setEnemy), enemy_interval);

}

void GameScene::resetStage(Ref *sender)
{
    removeLabel(sender);
    isOnLoading = false;
}

int GameScene::getStageNum(int distance) 
{
    int _newStage = 1;
    
    if ( 100 > distance ) {
        _newStage = 1;
    } else if ( 200 > distance ) {
        _newStage = 2;
    } else if ( 300 > distance ) {
        _newStage = 3;
    } else if ( 400 > distance ){
        _newStage = 4;
    } 
    
    return _newStage;
}

void GameScene::initBG()
{
	auto bgLayer = Layer::create();
	this->addChild(bgLayer);

	for (int i = 0; i<30; ++i) {
		for (int j = 0; j<26; ++j) {
			auto spr = Sprite::create("game/tile.png");
			spr->setAnchorPoint(Point::ZERO);
			spr->setPosition(Point(i * 33, j * 49));
			bgLayer->addChild(spr);
		}
	}

	auto action_0 = Sequence::create(
									MoveBy::create(0.5, Point(0, -49)),
									Place::create(Point::ZERO),
									NULL);

	auto action_1 = RepeatForever::create(action_0);
	bgLayer->runAction(action_1);
}

void GameScene::initPlayer()
{
    auto sprPlayer = Sprite::create("game/player_1.png");
    sprPlayer->setPosition(Point(winSize.width / 2, winSize.height / 2));
	sprPlayer->setTag(TAG_SPRITE_PLAYER);
    nHitPoint = TAG_DEFAULT_HITPOINT;
	this->addChild(sprPlayer, 1);

}

void GameScene::initHitPoint()
{
   	auto labelHp = Label::createWithSystemFont("", "", 20);
	labelHp->setAnchorPoint(Point(1, 1));
	labelHp->setPosition(Point(winSize.width / 2, winSize.height - 10));
	labelHp->setColor(Color3B::BLACK);
	labelHp->setTag(TAG_DEFAULT_HITPOINT);
	this->addChild(labelHp, 100);
    
    reduceHitPoint(0);
}

void GameScene::initScore()
{
	auto labelScore = Label::createWithSystemFont("", "", 20);
	labelScore->setAnchorPoint(Point(0, 1));
	labelScore->setPosition(Point(10, winSize.height - 10));
	labelScore->setColor(Color3B::BLACK);
	labelScore->setTag(TAG_LABEL_SCORE);
	this->addChild(labelScore, 100);

	auto labelHigh = Label::createWithSystemFont("", "", 20);
	labelHigh->setAnchorPoint(Point(1, 1));
	labelHigh->setPosition(Point(winSize.width - 10, winSize.height - 10));
	labelHigh->setColor(Color3B::BLACK);
	labelHigh->setTag(TAG_LABEL_HIGHSCORE);
	this->addChild(labelHigh, 100);

	addScore(0);
}

void GameScene::addScore(int add)
{
	nScore += add;

	if (nScore>nScoreHigh) {
		nScoreHigh = nScore;
		UserDefault::getInstance()->setIntegerForKey("HIGH_SCORE", nScoreHigh);
		UserDefault::getInstance()->flush();
	}

	auto labelScore = (Label*)this->getChildByTag(TAG_LABEL_SCORE);
	labelScore->setString(StringUtils::format("SCORE : %d", nScore));

	auto labelHigh = (Label*)this->getChildByTag(TAG_LABEL_HIGHSCORE);
	labelHigh->setString(StringUtils::format("HIGHSCORE : %d", nScoreHigh));

}

void GameScene::initDistance()
{
   
	auto labelDistance = Label::createWithSystemFont("DISTANCE : 0 M", "", 20);
	labelDistance->setAnchorPoint(Point(0, 1));
    labelDistance->setPosition(Point(10, winSize.height - 30));
	labelDistance->setColor(Color3B::BLACK);
	labelDistance->setTag(TAG_LABEL_DISTANCE);
	this->addChild(labelDistance, 100);	
    
}

void GameScene::updateDistance()
{
    auto labelDistance = (Label*)this->getChildByTag(TAG_LABEL_DISTANCE);
    labelDistance->setString(StringUtils::format("DISTANCE : %d M",(int)distance)); 
}

void GameScene::initBombStock()
{
    
    auto bomb = Sprite::create("game/bomb.png");
    bomb->setScale(0.3);
	bomb->setPosition(Point(20, winSize.height - 80));
	this->addChild(bomb, 100);
    
	auto labelBombStock = Label::createWithSystemFont("", "", 20);
	labelBombStock->setAnchorPoint(Point(0, 1));
	labelBombStock->setPosition(Point(40, winSize.height - 80));
	labelBombStock->setColor(Color3B::BLACK);
	labelBombStock->setTag(TAG_LABEL_BOMB_STOCK);
	this->addChild(labelBombStock, 100);
}

void GameScene::updateBomb(int amount)
{
    if ( isOnUsingItem ) return;
    
    if ( amount < 0 ) {
        if ( bombStock + amount >= 0 ) {
            bombStock = bombStock + amount;
            //敵を全部消す
            useBomb();
        }
    } else {
        bombStock = bombStock + amount;
    }
    auto labelBombStock = (Label*)this->getChildByTag(TAG_LABEL_BOMB_STOCK);
    labelBombStock->setString(StringUtils::format("X %d", bombStock));
}

void GameScene::useBomb()
{
	auto sprPlayer = (Sprite*)this->getChildByTag(TAG_SPRITE_PLAYER);
    isOnUsingItem = true;
    
    auto action = Sequence::create(
                                   ScaleTo::create(0.5, 5.0),
                                   CallFuncN::create(CC_CALLBACK_1(GameScene::clearEnemy, this)),
                                   CallFuncN::create(CC_CALLBACK_1(GameScene::setBombEffect, this)),
                                   ScaleTo::create(0.5, 1.0),
                                   NULL
    );
    
    sprPlayer->runAction(action);

}

void GameScene::setBombEffect(Ref *sender)
{
    bombTime = playTime + BOMB_E_TIME;
    
	auto particle = ParticleSystemQuad::create("game/ExplodingRing.plist");
	particle->setPosition(Point(winSize.width / 2, winSize.height / 2));
	particle->setScale(2.0);
	this->addChild(particle);

	auto action = Sequence::create(
									DelayTime::create(1.0),
                                   CallFuncN::create(CC_CALLBACK_1(GameScene::endUseBomb, this)),
									DelayTime::create(2.0),
									NULL);
	particle->runAction(action);
}

void GameScene::endUseBomb(Ref *sender)
{
    auto particle = (ParticleSystemQuad *)sender;
    this->removeChild( particle );
    
    isOnUsingItem = false;
}

void GameScene::setMissile(float delta)
{
    if ( !isPlayable || isOnUsingItem ) {
        return;
    }
	auto sprPlayer = (Sprite*)this->getChildByTag(TAG_SPRITE_PLAYER);


    
    if ( missileType == MISSILE_B ) {
        playSound(SOUND_MISSILE_1);
        
        for (int i=-1; i<=1; ++i) {
    		auto sprMissile = Sprite::create("game/missile_b.png");
    		sprMissile->setTag(MISSILE_B_POWER);

            auto action = Sequence::create(
    					MoveBy::create(1.0, Point(winSize.width*i, winSize.height)),
    					CallFuncN::create(CC_CALLBACK_1(GameScene::resetMissile, this)),
    					NULL);

        	sprMissile->setPosition(sprPlayer->getPosition() + Point(-1, 20));
        	this->addChild(sprMissile);

        	missiles.pushBack(sprMissile);

        	sprMissile->runAction(action);
        }
    }
    else if ( missileType == MISSILE_D ) {
        playSound(SOUND_MISSILE_1);
        
        for (int i=0; i<=1; ++i) {
    		auto sprMissile = Sprite::create("game/missile_c.png");
    		sprMissile->setTag(MISSILE_D_POWER);

            auto action = Sequence::create(
    					MoveBy::create(0.3, Point(50 + -100 * i , 0)),
    					MoveBy::create(1.0, Point(0, winSize.height)),
    					CallFuncN::create(CC_CALLBACK_1(GameScene::resetMissile, this)),
    					NULL);

        	sprMissile->setPosition(sprPlayer->getPosition() + Point(-1, 20));
        	this->addChild(sprMissile);

        	missiles.pushBack(sprMissile);

        	sprMissile->runAction(action);

        }

    } else {
        playSound(SOUND_MISSILE_1);
        
    	Sprite *sprMissile;
        switch (missileType) {
            case MISSILE_A:
        		sprMissile = Sprite::create("game/missile_a.png");
        		sprMissile->setTag(MISSILE_A_POWER);
                break;
             case MISSILE_C:
        		sprMissile = Sprite::create("game/missile_c.png");
        		sprMissile->setTag(MISSILE_C_POWER);
                break;
            default:
        		sprMissile = Sprite::create("game/missile.png");
        		sprMissile->setTag(BASIC_POWER);
                break;
        }

        auto action = Sequence::create(
    									MoveBy::create(1.0, Point(0, winSize.height)),
    									CallFuncN::create(CC_CALLBACK_1(GameScene::resetMissile, this)),
    									NULL);
    	sprMissile->setPosition(sprPlayer->getPosition() + Point(-1, 20));
        this->addChild(sprMissile, 10);

    	missiles.pushBack(sprMissile);

    	sprMissile->runAction(action);

    }

}

void GameScene::resetMissile(Ref *sender)
{
	auto sprMissile = (Sprite*)sender;

	missiles.eraseObject(sprMissile);

	this->removeChild(sprMissile);
}

void GameScene::setItem(float delta)
{
	int x = PADDING_SCREEN + rand() % ((int)winSize.width - PADDING_SCREEN * 2);

	Sprite *sprItem ;
    int rand_num = rand() % 5;

    if (rand_num <= 0) {
        sprItem = Sprite::create("game/item_a.png");
        sprItem->setTag(ITEM_A);
    }
    else if (rand_num <= 1) {
        sprItem = Sprite::create("game/item_b.png");
        sprItem->setTag(ITEM_B);
    }
    else if (rand_num <= 2) {
        sprItem = Sprite::create("game/item_c.png");
        sprItem->setTag(ITEM_C);
    }
    else if (rand_num <= 3) {
        sprItem = Sprite::create("game/item_d.png");
        sprItem->setTag(ITEM_D);
    }
    else if ( rand_num <= 4 ){
        sprItem = Sprite::create("game/item_e.png");
        sprItem->setTag(ITEM_E);
    }

	sprItem->setPosition(Point(x, winSize.height));
    
	this->addChild(sprItem);

	items.pushBack(sprItem);

	auto action = Sequence::create(
		MoveBy::create(3.0, Point(0, -winSize.height)),
		CallFuncN::create(CC_CALLBACK_1(GameScene::resetItem, this)),
		NULL);
	sprItem->runAction(action);
}

void GameScene::resetItem(Ref *sender)
{
	auto sprItem = (Sprite*)sender;

	items.eraseObject(sprItem);

	this->removeChild(sprItem);
}

void GameScene::setItemEffect(int itemType)
{
    switch (itemType) {
        case ITEM_A:
            missileType = MISSILE_A;
            break;
        case ITEM_B:
            missileType = MISSILE_B;
            break;
        case ITEM_C:
            missileType = MISSILE_C;
            break;
        case ITEM_D:
            missileType = MISSILE_D;
            break;
        case ITEM_E:
            updateBomb(1);
            break;
            
        default:
            break;
    }
}

void GameScene::setEnemy(float detla)
{
    if ( isOnLoading ) return;

    int rand_num = rand()%100;

    switch (curStageNum) {
        case 1:
        {
            if ( rand_num < 80) {
                setEnemy1();
            } else {
                setEnemy2();
            }
        }
            break;
            
        case 2:
        {
            if ( rand_num < 50 ) {
                setEnemy1();
            } else if ( rand_num < 80) {
                setEnemy2();
            } else {
                setEnemy3();
            }
        }
            break;
                       
        case 3:
        {
            if ( rand_num < 40) {
                setEnemy1();
            } else if ( rand_num < 60) {
                setEnemy2();
            } else if ( rand_num < 80 ) {
                setEnemy3();
            } else {
                setEnemy4();
            }
        }
            break;
            
        case 4:
        {
            if ( rand_num < 30 ) {
                setEnemy1();
            } else if ( rand_num < 50 ) {
                setEnemy2();
            } else if ( rand_num < 70 ) {
                setEnemy3();
            } else if ( rand_num < 90) {
                setEnemy4();
            } else {
                setEnemy5();
            }

        }
            break;
            
        default:
        {
            if ( rand_num < 10 ) {
                setEnemy1();
            } else if ( rand_num < 30 ) {
                setEnemy2();
            } else if ( rand_num < 50 ) {
                setEnemy3();
            } else if ( rand_num < 80) {
                setEnemy4();
            } else {
                setEnemy5();
            }
        }
            break;
    }

}

void GameScene::setEnemy1()
{
	int x = PADDING_SCREEN + rand() % ((int)winSize.width - PADDING_SCREEN * 2);
    
    auto sprEnemy = Sprite::create("game/enemy_1.png");
    sprEnemy->setTag(ENEMY_1_HP);
    float speed = 10.0;
    
    sprEnemy->setPosition(Point(x, winSize.height));
    auto action = Sequence::create(
					MoveBy::create(speed, Point(0, -winSize.height)),
					CallFuncN::create(CC_CALLBACK_1(GameScene::resetEnemy, this)),
					NULL);
	this->addChild(sprEnemy);

	enemies.pushBack(sprEnemy);

    sprEnemy->runAction(action);
}

void GameScene::setEnemy2()
{
 	int x = PADDING_SCREEN + rand() % ((int)winSize.width - PADDING_SCREEN * 2);
    
    auto sprEnemy = Sprite::create("game/enemy_2.png");
    sprEnemy->setTag(ENEMY_2_HP);
    float speed = 6.0;
    
    sprEnemy->setPosition(Point(x, winSize.height));
    auto action = Sequence::create(
					MoveBy::create(speed, Point(0, -winSize.height)),
					CallFuncN::create(CC_CALLBACK_1(GameScene::resetEnemy, this)),
					NULL);
	this->addChild(sprEnemy);

	enemies.pushBack(sprEnemy);

    sprEnemy->runAction(action);   
}

void GameScene::setEnemy3()
{
  	int x = PADDING_SCREEN + rand() % ((int)winSize.width - PADDING_SCREEN * 2);
    
    int enemy_count = (std::min)(rand() % 2, 5);
    float speed1 = 1.0;
    float speed2 = 2.0;
    for (int i = 0; i < enemy_count; ++i) {
        auto sprEnemy = Sprite::create("game/enemy_3.png");
        sprEnemy->setTag(ENEMY_3_HP);

        sprEnemy->setPosition(Point(x, winSize.height));
        
        Sequence *action;

        action = Sequence::create(
                                  MoveBy::create(0.3 * i, Point(0, 0)),
                                  MoveBy::create(speed1, Point(-100, -100)),
                                  MoveBy::create(speed1, Point(200, -100)),
                                  MoveBy::create(speed1, Point(100, -100)),
                                  MoveBy::create(speed2, Point(0, -winSize.height)),
                                  CallFuncN::create(CC_CALLBACK_1(GameScene::resetEnemy, this)),
                                  NULL);

        
        this->addChild(sprEnemy);
        
        enemies.pushBack(sprEnemy);
        
        sprEnemy->runAction(action);
    }
}

void GameScene::setEnemy4()
{
  	int x = PADDING_SCREEN + rand() % ((int)winSize.width - PADDING_SCREEN * 2);
    
    int enemy_count = (std::min)(rand() % 2, 5);
    float speed1 = 1.0;
    float speed2 = 1.0;
    for (int i = 0; i < enemy_count; ++i) {
        auto sprEnemy = Sprite::create("game/enemy_4.png");
        sprEnemy->setTag(ENEMY_4_HP);
        
        sprEnemy->setPosition(Point(x, winSize.height));
        
        Sequence *action;

        action = Sequence::create(
                                  MoveBy::create(0.3 * i, Point(0, 0)),
                                  MoveBy::create(speed1, Point(-100, -100)),
                                  MoveBy::create(speed1, Point(200, -100)),
                                  MoveBy::create(speed1, Point(100, -100)),
                                  MoveBy::create(speed2, Point(0, -winSize.height)),
                                  CallFuncN::create(CC_CALLBACK_1(GameScene::resetEnemy, this)),
                                  NULL);

        
        this->addChild(sprEnemy);
        
        enemies.pushBack(sprEnemy);
        
        sprEnemy->runAction(action);
    }
}

void GameScene::setEnemy5()
{
  	int x = PADDING_SCREEN + rand() % ((int)winSize.width - PADDING_SCREEN * 2);
    
    auto action_1 = Sequence::create(
                                     DelayTime::create(1.0),
                                     MoveBy::create(2, Point(0,-winSize.height)),
                                     MoveBy::create(3, Point(0,winSize.height)),
                                     NULL);
    
    auto action_2 = Sequence::create(
                                     Repeat::create(action_1, 2),
                                     MoveBy::create(2, Point(0,-winSize.height-50)),
                                     CallFuncN::create(CC_CALLBACK_1(GameScene::resetEnemy, this)),
                                     NULL);
    
    auto sprEnemy = Sprite::create("game/boss_1.png");
    sprEnemy->setTag(ENEMY_5_HP);
    sprEnemy->setScale(0.5, 0.5);
    sprEnemy->setPosition(Point(x, winSize.height));
    
    this->addChild(sprEnemy, 1);
    enemies.pushBack(sprEnemy);
    
    sprEnemy->runAction(action_2);
}

void GameScene::setBoss(int stage)
{
    isOnBossBattle = true;
  
    auto action_1 = Sequence::create(
                                   Blink::create(3.0, 3),
                                   CallFuncN::create(CC_CALLBACK_1(GameScene::removeLabel, this)),
                                   NULL
    );
    
    auto label = Label::createWithSystemFont( "ALERT", "", 80 );
	label->setPosition(Point(winSize.width / 2, winSize.height / 3 * 2 ));
	label->setColor(Color3B::RED);
    label->runAction(action_1);
    this->addChild(label, 200) ;
    
    auto action_2 = Sequence::create(
                                     DelayTime::create(1.0),
                                     MoveTo::create(3, Point(winSize.width/2,-30)),
                                     MoveTo::create(4, Point(winSize.width/2,winSize.height/3*2)),
                                     NULL);
    
    auto action_3 = Sequence::create(
                                     Repeat::create(action_2, 5),
                                     MoveTo::create(3, Point(0,0)),
                                     CallFuncN::create(CC_CALLBACK_1(GameScene::resetBoss, this)),
                                     NULL);
    
    auto sprBoss = Sprite::create("game/boss_1.png");
    sprBoss->setTag(BOSS_1_HP * stage);
    sprBoss->setScale(1.0, 1.0);
    sprBoss->setPosition(Point(winSize.width/2, winSize.height));
    this->addChild(sprBoss, 1);
    bosses.pushBack(sprBoss);
    
    sprBoss->runAction(action_3);

}

void GameScene::resetBoss(Ref *sender)
{
    auto sprBoss = (Sprite*)sender;
    
    isOnBossBattle = false;

    bosses.eraseObject(sprBoss);
    
    this->removeChild(sprBoss);
    
    clearEnemy(false);
 
    curStageNum = curStageNum <= MAX_STAGE ? curStageNum + 1 : MAX_STAGE;
    initStage(curStageNum);
}

void GameScene::destroyBoss(Ref *sender)
{
    auto sprBoss = (Sprite*)sender;
    
    playSound(SOUND_CRASH_1);
   
    isOnBossBattle = false;
    
	auto particle = ParticleSystemQuad::create("game/explosion.plist");
	particle->setPosition(sprBoss->getPosition());
	particle->setScale(2.0);
	this->addChild(particle);

	auto action = Sequence::create(
									DelayTime::create(1.5),
									CallFuncN::create(CC_CALLBACK_1(GameScene::resetBoom, this)),
									NULL);
	particle->runAction(action);
    
    bosses.eraseObject(sprBoss);
    
    this->removeChild(sprBoss);
    
    clearEnemy(true);
 
    curStageNum = curStageNum <= MAX_STAGE ? curStageNum + 1 : MAX_STAGE;
    initStage(curStageNum);   
}

void GameScene::clearItem()
{
    for (Sprite* sprItem : items) {
        this->removeChild(sprItem);
    }
    items.clear();
}

void GameScene::clearEnemy(bool addScoreFlg)
{
    if ( addScoreFlg ) {
        int cnt = enemies.size();
        addScore(ENEMY_SCORE * cnt);
    }
    
    for (Sprite* sprEnemy: enemies) {

		auto action = Sequence::create(
										DelayTime::create(0.1),
                                        CallFuncN::create(CC_CALLBACK_1(GameScene::destroyEnemy, this)),
										NULL);
		sprEnemy->runAction(action);
    }
    enemies.clear();

}

void GameScene::clearBoss(bool addScoreFlg)
{
    if ( addScoreFlg ) {
        int cnt = bosses.size();
        addScore(BOSS_SCORE * cnt);
    }
    
    for (Sprite* sprBoss : bosses) {
        this->removeChild(sprBoss);
    }
    bosses.clear();
}

void GameScene::removeLabel(Ref *sender)
{
    auto label = (Label*)sender;
    this->removeChild(label);
}

void GameScene::reduceHitPoint(int damage)
{
    nHitPoint = nHitPoint - damage;
   
	auto label = (Label*)this->getChildByTag(TAG_DEFAULT_HITPOINT);
	label->setString(StringUtils::format("HP : %d", nHitPoint));
   
    if ( damage > 0 ) {
        auto action = Blink::create(1.0, 4);
       
    	auto sprPlayer = (Sprite*)this->getChildByTag(TAG_SPRITE_PLAYER);
//        auto frameId = TAG_DEFAULT_HITPOINT - nHitPoint + 1;
//        frameId = frameId < 3 ? frameId : 3;
//        
//        auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(StringUtils::format("player_%d.png",frameId));
//        sprPlayer->setDisplayFrame(frame);
        sprPlayer->runAction(action);
    }
}

void GameScene::endGame()
{
	auto sprPlayer = (Sprite*)this->getChildByTag(TAG_SPRITE_PLAYER);
    isPlayable = false;
    
	auto particle = ParticleSystemQuad::create("game/explosion.plist");
	particle->setPosition(sprPlayer->getPosition());
	particle->setScale(2.0);
	this->addChild(particle);

	auto action = Sequence::create(
									DelayTime::create(1.0),
									CallFuncN::create(CC_CALLBACK_1(GameScene::resetBoom, this)),
									NULL);
    
	particle->runAction(action);
    this->removeChild(sprPlayer);
    
    displayResult();
   
}

void GameScene::retryGame()
{
 	auto sprPlayer = (Sprite*)this->getChildByTag(TAG_SPRITE_PLAYER);
    this->removeChild(sprPlayer);

    initData();
    
    this->removeChild(this->getChildByTag(TAG_MENU));
    this->removeChild(this->getChildByTag(TAG_LABEL_FINISH));
    this->removeChild(this->getChildByTag(TAG_LABEL_RESULT_SCORE));}

void GameScene::displayResult()
{

    auto action_1 = Sequence::create(
                                   ScaleTo::create(1.0, 1.5),
                                   ScaleTo::create(0.5, 1.0),
                                   NULL);
    
    auto label_1 = Label::createWithSystemFont( "FINISHED", "", 80 );
    label_1->setPosition(Point(winSize.width/2, winSize.height - 100));
    label_1->setScale(0.0);
    label_1->setTag(TAG_LABEL_FINISH);
    label_1->runAction(action_1);
    
    this->addChild(label_1) ;
 
    auto action_2 = Sequence::create(
                                   ScaleTo::create(1.0, 1.5),
                                   ScaleTo::create(0.5, 1.0),
                                   NULL);
    
    auto label_2 = Label::createWithSystemFont( StringUtils::format("SCORE : %d", nScore), "", 40 );
    label_2->setPosition(Point(winSize.width/2, winSize.height - 150));
    label_2->setTag(TAG_LABEL_RESULT_SCORE);
    label_2->runAction(action_2);
    
    this->addChild(label_2);
}


void GameScene::resetEnemy(Ref *sender)
{
	auto sprEnemy = (Sprite*)sender;

	enemies.eraseObject(sprEnemy);

	this->removeChild(sprEnemy);
}

void GameScene::destroyEnemy(Ref *sender)
{
 	auto sprEnemy = (Sprite*)sender;
    
    playSound(SOUND_CRASH_1);
   
	auto particle = ParticleSystemQuad::create("game/explosion.plist");
	particle->setPosition(sprEnemy->getPosition());
	particle->setScale(0.5);
	this->addChild(particle);

	auto action = Sequence::create(
									DelayTime::create(1.0),
									CallFuncN::create(CC_CALLBACK_1(GameScene::resetBoom, this)),
									NULL);
	particle->runAction(action);

	enemies.eraseObject(sprEnemy);

	this->removeChild(sprEnemy);   
}

//いい音に変える
void GameScene::playSound(int type)
{
    if (!soundFlg) return;
    
    switch (type)
    {
        case SOUND_CRASH_1:
        {
            SimpleAudioEngine::getInstance()->playEffect("sound/crash_1.wav");
        }
            break;
           
        case SOUND_MISSILE_1:
        {
            SimpleAudioEngine::getInstance()->playEffect("sound/missile_4.wav", false, 1.0, 0.0, 0.05);
        }
            break;
        default:
            break;
    }
}

bool GameScene::onTouchBegan(Touch *touch, Event *unused_event)
{
    if ( !isPlayable ) {
        return false;
    }
    posStartTouch = touch->getLocation();

	auto sprPlayer = (Sprite*)this->getChildByTag(TAG_SPRITE_PLAYER);
	posStartPlayer = sprPlayer->getPosition();

    //ダブルクリック判定
    CCLOG("play Time : %f", playTime);
    CCLOG("tab Time : %f", tabTime);
    CCLOG("bombStock : %d", bombStock);
    if ( playTime - tabTime <= 0.3 ) {
        updateBomb(-1);
    }
    tabTime = playTime;
    
	return true;
}

void GameScene::onTouchMoved(Touch *touch, Event *unused_event)
{
    if ( !isPlayable ) {
        return;
    }
	Point location = touch->getLocation();

	Point posChange = location - posStartTouch;

	auto sprPlayer = (Sprite*)this->getChildByTag(TAG_SPRITE_PLAYER);
   
    auto newPos = posStartPlayer + posChange;
    if ( newPos.x > 10 && newPos.y > 10
         && newPos.x < winSize.width - 10 && newPos.y < winSize.height - 20 )
    {
        sprPlayer->setPosition(posStartPlayer + posChange);
    }
    
}

void GameScene::displayOption()
{
   	auto item_0 = MenuItemImage::create("option/option.png", "option/option.png", CC_CALLBACK_1(GameScene::menuCallback, this));

	item_0->setTag(TAG_OPTION);

	auto menu = Menu::create(item_0, NULL);
	menu->alignItemsVerticallyWithPadding(20);
    menu->setPosition(winSize.width - 50, winSize.height - 60);
	this->addChild(menu, 200);
}

void GameScene::displayMenu()
{
   	auto item_0 = MenuItemImage::create("menu/play-0.png", "menu/play-1.png", CC_CALLBACK_1(GameScene::menuCallback, this));
	auto item_1 = MenuItemImage::create("menu/quit-0.png", "menu/quit-0.png", CC_CALLBACK_1(GameScene::menuCallback, this));

	item_0->setTag(TAG_MENUITEM_PLAY);
	item_1->setTag(TAG_MENUITEM_QUIT);

	auto menu = Menu::create(item_0, item_1, NULL);
    menu->setTag(TAG_MENU);
	menu->alignItemsVerticallyWithPadding(20);
	this->addChild(menu, 200);
}

void GameScene::showOption()
{
    if ( this->getChildByTag(TAG_OPTION) ) return;
    
    Director::getInstance()->pause();
    
    auto spr = Sprite::create("menu/menu-bg.png");
    spr->setTag(TAG_OPTION_BG);
	spr->setPosition(Point(winSize.width/2, winSize.height/2));
    spr->setScale(1.0, 1.0);
	this->addChild(spr, 300);
    
   	auto item_0 = MenuItemImage::create("option/btn_sound_on.png", "option/btn_sound_on.png", CC_CALLBACK_1(GameScene::menuCallback, this));
	auto item_1 = MenuItemImage::create("option/btn_sound_off.png", "option/btn_sound_off.png", CC_CALLBACK_1(GameScene::menuCallback, this));
	auto item_2 = MenuItemImage::create("option/btn_close.png", "option/btn_close.png", CC_CALLBACK_1(GameScene::menuCallback, this));
	auto item_3 = MenuItemImage::create("option/btn_retry.png", "option/btn_retry.png", CC_CALLBACK_1(GameScene::menuCallback, this));
	auto item_4 = MenuItemImage::create("option/btn_quit.png", "option/btn_quit.png", CC_CALLBACK_1(GameScene::menuCallback, this));

	item_0->setTag(TAG_OPTION_SOUND_ON);
	item_1->setTag(TAG_OPTION_SOUND_OFF);
	item_2->setTag(TAG_OPTION_CLOSE);
	item_3->setTag(TAG_OPTION_RETRY);
	item_4->setTag(TAG_OPTION_QUIT);
    
	auto menu = Menu::create(item_0, item_1, item_2, item_3, item_4, NULL);
    menu->setTag(TAG_OPTION);
	menu->alignItemsVerticallyWithPadding(20);
	this->addChild(menu, 300);
}

void GameScene::hideOption()
{
    
    auto menu = (Menu*)this->getChildByTag(TAG_OPTION);
    this->removeChild(menu);
   
    auto sprBg = (Sprite*)this->getChildByTag(TAG_OPTION_BG);
    this->removeChild(sprBg);
    
    Director::getInstance()->resume();
}

void GameScene::menuCallback(Ref *sender)
{
	auto item = (MenuItem*)sender;

	switch (item->getTag())
	{
    case TAG_OPTION:
    {
        showOption();
    }
        break;
            
//	case TAG_MENUITEM_PLAY:
//	{
//
//        initData();
//        
//        this->removeChild(this->getChildByTag(TAG_MENU));
//        this->removeChild(this->getChildByTag(TAG_LABEL_FINISH));
//        this->removeChild(this->getChildByTag(TAG_LABEL_RESULT_SCORE));
//	}
//		break;
   	case TAG_OPTION_SOUND_ON:
	{
        soundFlg = true;
        hideOption();
	}
		break;
            
	case TAG_OPTION_SOUND_OFF:
	{
        soundFlg = false;
        hideOption();
	}
		break;
            
	case TAG_OPTION_CLOSE:
	{
        hideOption();
	}
		break;
            
	case TAG_OPTION_RETRY:
	{
        retryGame();
        hideOption();
       
	}
		break;
            
	case TAG_OPTION_QUIT:
	{
		Director::getInstance()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
		exit(0);
#endif
	}
		break;
            
//	case TAG_MENUITEM_QUIT:
//		Director::getInstance()->end();
//#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
//		exit(0);
//#endif
//		break;
	default:
		break;
	}
}