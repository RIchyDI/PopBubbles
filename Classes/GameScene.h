#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

class Element;

// sprite element
struct ElementPos
{
	int row;
	int col;
	
};

// sprite struct
struct ElementProto
{
	int type;
	bool marked;
};


class GameScene : public cocos2d::Layer
{
public:
	static cocos2d::Scene *createScene();

	virtual bool init();
	virtual void onEnter();
	virtual void onExit();

	virtual void update(float dt);

	// touch detection
	virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event);
	virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event);
	virtual void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *event);

	CREATE_FUNC(GameScene);

private:
	//sprites vector
	std::vector<std::vector<ElementProto>> _game_board; 
	//start touch position and end touch position
	ElementPos _start_pos, _end_pos; 
	//moving
	bool _is_moving; 
	//can be touched
	bool _is_can_touch; 
	//can be elemilate
	int _is_can_elimate; 
	//get element position
	ElementPos getElementPosByCoordinate(float x, float y);
	//score
	int _score; 
	//score label
	cocos2d::Label *_score_label; 
	//score addition
	int _animation_score; 
	//progress timer
	cocos2d::ProgressTimer *_progress_timer; 
	//encourage label
	cocos2d::Label *_combo_label; 
	//fill map
	void fillGameBoard(int row, int col); 
	//draw map
	void drawGameBoard(); 
	//fill blank
	void dropElements(float dt); 
	//fill sprites
	void fillVacantElements(); 
	//exchange sprites
	void swapElementPair(ElementPos p1, ElementPos p2, bool is_reverse); 
	//check elimination
	bool hasEliminate(); 
	//get elimination set
	std::vector<ElementPos> getEliminateSet(); 
	//eliminate
	void batchEliminate(const std::vector<ElementPos> &eliminate_list); 
	//delay eliminate
	void delayBatchEliminate(float dt); 
	//check elimination
	ElementPos checkGameHint(); 
	//add score
	void addScore(int delta_score); 
	//timer
	void addScoreCallback(float dt); 
	//update timer
	void tickProgress(float dt); 
};

#endif // __HELLOWORLD_SCENE_H__
