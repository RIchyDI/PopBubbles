#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include "cocos2d.h"

class Element : public cocos2d::Sprite
{
public:
	virtual bool init();

	int element_type; // image

	void appear(); // appear animation
	void appearSchedule(float dt); // delay animation

	void vanish(); // disappear animation
	void vanishCallback(); // disappear

	CREATE_FUNC(Element);
}; 

#endif

