#include "SpriteElement.h"

USING_NS_CC;


bool Element::init()
{
	if (!Sprite::init())
		return false;

	// init element
	element_type = -1;

	return true;
}

void Element::appear()
{
	// show appear animation
	setVisible(false);
	scheduleOnce(schedule_selector(Element::appearSchedule), 0.3);
}

void Element::appearSchedule(float dt)
{
	setVisible(true);
	setScale(0.5);
	
	ScaleTo *scale_to = ScaleTo::create(0.2, 1.0);
	runAction(scale_to);
}

void Element::vanish()
{
	// show disappear animation
	ScaleTo *scale_to = ScaleTo::create(0.2, 0.5);
	CallFunc *funcall = CallFunc::create(this, callfunc_selector(Element::vanishCallback));
	Sequence *sequence = Sequence::create(DelayTime::create(0.2), scale_to, funcall, NULL);
	runAction(sequence);
}

void Element::vanishCallback()
{
	removeFromParent();
}




