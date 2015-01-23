#include "GameScene.h"

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

	initData();

	initBG();
	initPlayer();
	initScore();
    initHitPoint();

    
	this->scheduleUpdate();
	this->schedule(schedule_selector(GameScene::setMissile), 0.1);
	this->schedule(schedule_selector(GameScene::setItem), itemFrequency);
	this->schedule(schedule_selector(GameScene::setEnemy), enemyFrequency);

	return true;
}

void GameScene::update(float delta)
{
    if ( !isPlayable) {
        return;
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
        if ( removeSpr->getTag() > 0 ) {
            isPowerUp = true;
        }
		resetItem(removeSpr);

		isGetItem = true;

		this->scheduleOnce(schedule_selector(GameScene::resetGetItem), 5.0);
	}

	auto removeMissile = Sprite::create();
	auto removeEnemy = Sprite::create();

	for (Sprite* sprMissile : missiles) {

		Rect rectMissile = sprMissile->getBoundingBox();

		for (Sprite* sprEnemy : enemies) {

			Rect rectEnemy = Rect(sprEnemy->getPositionX()-10, sprEnemy->getPositionY()-10, 20, 20);

			if (rectMissile.intersectsRect(rectEnemy)) {

				int attack = sprMissile->getTag();
				int hp = sprEnemy->getTag();

				removeMissile = sprMissile;

				if (hp-attack>0) {
					sprEnemy->setTag(hp - attack);
					addScore(1);
				}
				else {
					removeEnemy = sprEnemy;
					addScore(100);
				}
			}
		}
	}
   
    for (Sprite* sprEnemy : enemies) {

		Rect rectEnemy = Rect(sprEnemy->getPositionX()-10, sprEnemy->getPositionY()-10, 20, 20);
        if (rectPlayer.intersectsRect(rectEnemy)) {
            removeEnemy = sprEnemy;
            reduceHitPoint(1);
            CCLOG("remains hit point:%d", nHitPoint);
            if ( 0 >= nHitPoint ) {
                endGame();
            }
        }
    }


	if (missiles.contains(removeMissile)) {
		resetMissile(removeMissile);
	}

	if (enemies.contains(removeEnemy)) {
		auto particle = ParticleSystemQuad::create("game/explosion.plist");
		particle->setPosition(removeEnemy->getPosition());
		particle->setScale(0.5);
		this->addChild(particle);

		auto action = Sequence::create(
										DelayTime::create(1.0),
										CallFuncN::create(CC_CALLBACK_1(GameScene::resetBoom, this)),
										NULL);
		particle->runAction(action);

		resetEnemy(removeEnemy);
	}
    

}

void GameScene::resetBoom(Ref *sender)
{
	auto particle = (ParticleSystemQuad*)sender;

	this->removeChild(particle);
}


void GameScene::resetGetItem(float delta)
{
	isGetItem = false;
	isPowerUp = false;
}

void GameScene::initData()
{
	winSize = Director::getInstance()->getWinSize();

	items.clear();
	enemies.clear();
	missiles.clear();

	isGetItem = false;
	isPowerUp = false;
	isPlayable = true;
    itemFrequency = 5.0;
    enemyFrequency = 0.5;
	nScore = 0;
	nScoreHigh = UserDefault::getInstance()->getIntegerForKey("HIGH_SCORE", 0);
}

void GameScene::initBG()
{
	auto bgLayer = Layer::create();
	this->addChild(bgLayer);

	for (int i = 0; i<30; i++) {
		for (int j = 0; j<26; j++) {
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
    SpriteFrameCache::getInstance()->addSpriteFramesWithFile("game/player.plist");
    auto sprPlayer = Sprite::createWithSpriteFrameName("player_1.png");
    sprPlayer->setPosition(Point(winSize.width / 2, winSize.height / 2));
	sprPlayer->setTag(TAG_SPRITE_PLAYER);
    nHitPoint = TAG_DEFAULT_HITPOINT;
	this->addChild(sprPlayer, 1);

}

void GameScene::initHitPoint()
{
   	auto labelHigh = Label::createWithSystemFont("", "", 20);
	labelHigh->setAnchorPoint(Point(1, 1));
	labelHigh->setPosition(Point(winSize.width / 2, winSize.height - 10));
	labelHigh->setColor(Color3B::BLACK);
	labelHigh->setTag(TAG_DEFAULT_HITPOINT);
	this->addChild(labelHigh, 100);
    
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

void GameScene::setMissile(float delta)
{
    if ( !isPlayable ) {
        return;
    }
	auto sprPlayer = (Sprite*)this->getChildByTag(TAG_SPRITE_PLAYER);

	Sprite *sprMissile;

    if (isPowerUp) {
		sprMissile = Sprite::create("game/missile_ext.png");
		sprMissile->setTag(8);
    } else if (isGetItem) {
		sprMissile = Sprite::create("game/missile_pow.png");
		sprMissile->setTag(5);
	}
	else {
		sprMissile = Sprite::create("game/missile.png");
		sprMissile->setTag(1);
	}

	sprMissile->setPosition(sprPlayer->getPosition() + Point(-1, 20));
	this->addChild(sprMissile);

	missiles.pushBack(sprMissile);

	auto action = Sequence::create(
									MoveBy::create(1.0, Point(0, winSize.height)),
									CallFuncN::create(CC_CALLBACK_1(GameScene::resetMissile, this)),
									NULL);
	sprMissile->runAction(action);
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

	auto sprItem = Sprite::create("game/item.png");
	sprItem->setPosition(Point(x, winSize.height));
    if (rand()%2 > 0) {
        sprItem->setScale(2.0, 2.0);
        sprItem->setTag(1);
    } else {
        sprItem->setTag(0);
    }

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

void GameScene::setEnemy(float detla)
{
	int x = PADDING_SCREEN + rand() % ((int)winSize.width - PADDING_SCREEN * 2);

	Sprite *sprEnemy;
    float speed;

	if (rand() % 100<20) {
		sprEnemy = Sprite::create("game/boss.png");
		sprEnemy->setTag(30);
        speed = 10.0;
	}
	else {
		sprEnemy = Sprite::create("game/enemy.png");
		sprEnemy->setTag(5);
        speed = 5.0;
	}
	sprEnemy->setPosition(Point(x, winSize.height));
	this->addChild(sprEnemy);

	enemies.pushBack(sprEnemy);

	auto action = Sequence::create(
									MoveBy::create(speed, Point(0, -winSize.height)),
									CallFuncN::create(CC_CALLBACK_1(GameScene::resetEnemy, this)),
									NULL);
	sprEnemy->runAction(action);
}

void GameScene::reduceHitPoint(int damage)
{
    nHitPoint = nHitPoint - damage;
   
	auto label = (Label*)this->getChildByTag(TAG_DEFAULT_HITPOINT);
	label->setString(StringUtils::format("HP : %d", nHitPoint));
   
    if ( damage > 0 ) {
        auto action = Blink::create(0.5, 1.0);
       
        auto frameId = TAG_DEFAULT_HITPOINT - nHitPoint + 1;
        frameId = frameId < 3 ? frameId : 3;
        CCLOG("frameId:%d", frameId);
    	auto sprPlayer = (Sprite*)this->getChildByTag(TAG_SPRITE_PLAYER);
        auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(
                                                                           StringUtils::format("player_%d.png",frameId));
        sprPlayer->setDisplayFrame(frame);
        
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
    displayMenu();
   
}

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

bool GameScene::onTouchBegan(Touch *touch, Event *unused_event)
{
    if ( !isPlayable ) {
        return false;
    }
    posStartTouch = touch->getLocation();

	auto sprPlayer = (Sprite*)this->getChildByTag(TAG_SPRITE_PLAYER);
	posStartPlayer = sprPlayer->getPosition();

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

void GameScene::displayMenu()
{
   	auto item_0 = MenuItemImage::create("menu/play-0.png", "menu/play-1.png", CC_CALLBACK_1(GameScene::menuCallback, this));
	auto item_1 = MenuItemImage::create("menu/quit-0.png", "menu/quit-0.png", CC_CALLBACK_1(GameScene::menuCallback, this));

	item_0->setTag(TAG_MENUITEM_PLAY);
	item_1->setTag(TAG_MENUITEM_QUIT);

	auto menu = Menu::create(item_0, item_1, NULL);
    menu->setTag(TAG_MENU);
	menu->alignItemsVerticallyWithPadding(20);
	this->addChild(menu);
}

void GameScene::menuCallback(Ref *sender)
{
	auto item = (MenuItem*)sender;

	switch (item->getTag())
	{
	case TAG_MENUITEM_PLAY:
	{
        initPlayer();
        initScore();
        initHitPoint();
        isPlayable = true;
        this->removeChild(this->getChildByTag(TAG_MENU));
        this->removeChild(this->getChildByTag(TAG_LABEL_FINISH));
        this->removeChild(this->getChildByTag(TAG_LABEL_RESULT_SCORE));
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