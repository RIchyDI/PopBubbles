#include "HelloWorldScene.h"
#include "GameDefine.h"
#include "GameScene.h"

USING_NS_CC;

static void problemLoading(const char* filename)
{
	printf("Error while loading: %s\n", filename);
	printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

Scene* GameScene::createScene() {
	auto scene = Scene::create();
	auto layer = GameScene::create();
	scene->addChild(layer);
	return scene;
}

// welcome
bool GameScene::init() {
	// super init first
	if (!Layer::init()) {
		return false;
	}

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// add background picture
	auto sceneItem = Sprite::create("scene_game.png");
	if (sceneItem == nullptr)
	{
		problemLoading("'scene_game.png'");
	}
	else
	{
		sceneItem->setScaleX(2);
		sceneItem->setScaleY(2);

		// position the sceneItem on the center of the screen
		sceneItem->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));

		// add the sceneItem as a child to this layer
		this->addChild(sceneItem, 0);
	}

	// add back button
	auto backItem = MenuItemImage::create("btn_back01.png",
		"btn_back02.png",
		CC_CALLBACK_1(GameScene::menuBackCallback, this));
	if (backItem == nullptr ||
		backItem->getContentSize().width <= 0 ||
		backItem->getContentSize().height <= 0)
	{
		problemLoading("'btn_back01.png' and 'btn_back02.png'");
	}
	else
	{
		backItem->setPosition(Vec2(GAME_SCREEN_WIDTH -160 , 10));
	}

	auto menuBack = Menu::create(backItem, NULL);
	menuBack->setPosition(Vec2::ZERO);
	this->addChild(menuBack, 1);

	return true;
}

// back to welcomeScene
void GameScene::menuBackCallback(Ref* pSender) {
	auto scene = HelloWorld::createScene();
	CCDirector::sharedDirector()->replaceScene(scene);
}