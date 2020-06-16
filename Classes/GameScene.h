#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"

class GameScene : public cocos2d::Layer
{
public:
	static cocos2d::Scene* createScene();
	virtual bool init();

	//back to welcomeScene
	void menuBackCallback(Ref* pSender);
	CREATE_FUNC(GameScene);
};

#endif // __GAME_SCENE_H__