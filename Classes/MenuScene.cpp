#include "SimpleAudioEngine.h"
#include "MenuScene.h"
#include "GameScene.h"

USING_NS_CC;

Scene *MenuScene::createScene()
{
	return MenuScene::create();
}

bool MenuScene::init()
{
	if (!Scene::init())
		return false;	

	// get screen size
	const Size kScreenSize = Director::getInstance()->getVisibleSize();
	const Vec2 kScreenOrigin = Director::getInstance()->getVisibleOrigin();

	// init menu scene
	Sprite *menu_background = Sprite::create("menu_bg.png");
	menu_background->setPosition(kScreenOrigin.x + kScreenSize.width / 2, kScreenOrigin.y + kScreenSize.height / 2);
	addChild(menu_background, 0);

	// add start label
	Label *start_label = Label::createWithTTF("Start Game", "fonts/Minecraft.ttf", 35);
	start_label->setTextColor(cocos2d::Color4B::WHITE);


	// ssing lambda expression as menu callback
	MenuItemLabel *start_menu_item = MenuItemLabel::create(start_label, [&](Ref *sender) {
		CCLOG("click start game"); 

		// Go to the game scene
		Scene *main_game_scene = GameScene::createScene();
		TransitionScene *transition = TransitionFade::create(0.5f, main_game_scene, Color3B(255, 255, 255));
		Director::getInstance()->replaceScene(transition);
	});
	start_menu_item->setPosition(kScreenOrigin.x + kScreenSize.width / 2, kScreenOrigin.y + kScreenSize.height / 2);
	
	Menu *menu = Menu::createWithItem(start_menu_item);
	menu->setPosition(Vec2::ZERO);

	addChild(menu, 1);

	return true;
}