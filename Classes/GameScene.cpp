#include "SimpleAudioEngine.h"
#include "GameScene.h"
#include "SpriteElement.h"

USING_NS_CC;
using namespace CocosDenshion;

// layers
const int backGroundLevel = 0; // background
const int gameBoardLevel = 1;  // sprite elements
const int flashLevel = 3; // flash
const int menuLevel = 5; // menu

// sprite images
const std::vector<std::string> kElementImgArray{
	"images/icon_1.png",
	"images/icon_2.png",
	"images/icon_3.png",
	"images/icon_4.png",
	"images/icon_5.png",
	"images/icon_6.png"
};

// score evaluation
const std::vector<std::string> kComboTextArray{
	"Good",
	"Great",
	"Unbelievable"
};

// music
const std::string backgourndMusic = "music/bg.wav";
const std::string welcomeEffect = "music/welcome.wav";
const std::string popEffect = "music/pop.wav";
const std::string kunbelievableEffect = "music/unbelievable.wav";

// score
const int scoreUnit = 10;

// eliminate animation
const int elementEliminateType = 10;
const std::string eliminateStartImg = "images/boom.png";

// margin
const float leftMargin = 20;
const float rightMargin = 20;
const float bottonMargin = 70;

// number of rows and columns
const int rowNum = 8;
const int colNum = 8;

// eliminate enumeration types
const int eliminateInitFlag = 0;
const int eliminateOneReadyFlag = 1;
const int eliminateTwoReadyFlag = 2;

// random spirit
int getRandomSpriteIndex(int len)
{
	return rand() % len;
}

// create game scene
Scene *GameScene::createScene()
{
	Scene *game_scene = Scene::create();
	Layer *game_layer = GameScene::create();
	game_scene->addChild(game_layer);
	return game_scene;
}

// init game scene
bool GameScene::init()
{
	if (!Layer::init())
		return false;

	// get screen size
	const Size kScreenSize = Director::getInstance()->getVisibleSize();
	const Vec2 kScreenOrigin = Director::getInstance()->getVisibleOrigin();

	// loading game scene
	Sprite *game_background = Sprite::create("images/game_bg.png");
	game_background->setPosition(kScreenOrigin.x + kScreenSize.width / 2, kScreenOrigin.y + kScreenSize.height / 2);
	addChild(game_background, backGroundLevel);

	// init map
	for (int i = 0; i < rowNum; i++)
	{
		std::vector<ElementProto> line_elements;
		for (int j = 0; j < rowNum; j++)
		{
			ElementProto element_proto;
			element_proto.type = elementEliminateType; 
			element_proto.marked = false;

			line_elements.push_back(element_proto);
		}
		_game_board.push_back(line_elements);
	}
		
	// draw map
	drawGameBoard();

	// init score
	_score = 0;
	_animation_score = 0;

	_score_label = Label::createWithTTF(StringUtils::format("score: %d", _score), "fonts/Minecraft.ttf", 20);
	_score_label->setTextColor(cocos2d::Color4B::BLUE);
	_score_label->setPosition(kScreenOrigin.x + kScreenSize.width / 2, kScreenOrigin.y + kScreenSize.height * 0.9);
	_score_label->setName("score");
	addChild(_score_label, backGroundLevel);

	// init position
	_start_pos.row = -1;
	_start_pos.col = -1;

	_end_pos.row = -1;
	_end_pos.col = -1;

	// init sprite moving
	_is_moving = false;
	_is_can_touch = true;
	_is_can_elimate = 0; // >=2 means can be eliminated

	// progress bar
	_progress_timer = ProgressTimer::create(Sprite::create("images/progress_bar.png"));//create a progress bar
	_progress_timer->setBarChangeRate(Point(1, 0));
	_progress_timer->setType(ProgressTimer::Type::BAR);
	_progress_timer->setMidpoint(Point(0, 1));
	_progress_timer->setPosition(Point(kScreenOrigin.x + kScreenSize.width / 2, kScreenOrigin.y + kScreenSize.height * 0.8));
	_progress_timer->setPercentage(100.0); 
	addChild(_progress_timer, backGroundLevel);
	schedule(schedule_selector(GameScene::tickProgress), 1.0);

	// background music
	SimpleAudioEngine::getInstance()->playBackgroundMusic(backgourndMusic.c_str(), true);
	SimpleAudioEngine::getInstance()->playEffect(welcomeEffect.c_str());

	// encourage label
	_combo_label = Label::createWithTTF(StringUtils::format("Ready Go"), "fonts/Minecraft.ttf", 40);
	_combo_label->setPosition(kScreenOrigin.x + kScreenSize.width / 2, kScreenOrigin.y + kScreenSize.height / 2);
	addChild(_combo_label, flashLevel);
	_combo_label->runAction(Sequence::create(DelayTime::create(0.8), MoveBy::create(0.3, Vec2(200, 0)), CallFunc::create([=]() {
		
		_combo_label->setVisible(false);
		_combo_label->setPosition(kScreenOrigin.x + kScreenSize.width / 2, kScreenOrigin.y + kScreenSize.height / 2);
	}), NULL));

	// touch event
	EventListenerTouchOneByOne *touch_listener = EventListenerTouchOneByOne::create();
	touch_listener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
	touch_listener->onTouchMoved = CC_CALLBACK_2(GameScene::onTouchMoved, this);
	touch_listener->onTouchEnded = CC_CALLBACK_2(GameScene::onTouchEnded, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(touch_listener, this); 

	scheduleUpdate();

	return true;
}

ElementPos GameScene::getElementPosByCoordinate(float x, float y)
{
	const Size kScreenSize = Director::getInstance()->getVisibleSize();
	const Vec2 kScreenOrigin = Director::getInstance()->getVisibleOrigin();
	float element_size = (kScreenSize.width - leftMargin - rightMargin) / colNum;

	float row = (y - bottonMargin) / element_size;
	float col = (x - leftMargin) / element_size;

	ElementPos pos;
	pos.row = row;
	pos.col = col;

	return pos;
}

// full map
void GameScene::fillGameBoard(int row, int col)
{
	// boundary return encountered
	if (row == -1 || row == rowNum || col == -1 || col == colNum)
		return;

	// random create sprite
	int random_type = getRandomSpriteIndex(kElementImgArray.size());

	// full 
	if (_game_board[row][col].type == elementEliminateType)
	{
		_game_board[row][col].type = random_type;
		
		// continue filling if not eliminated
		if (!hasEliminate())
		{
			// fill four directions
			fillGameBoard(row + 1, col);
			fillGameBoard(row - 1, col);
			fillGameBoard(row, col - 1);
			fillGameBoard(row, col + 1);
		}
		else
			_game_board[row][col].type = elementEliminateType; 
	}
}

void GameScene::drawGameBoard()
{
	srand(unsigned(time(0))); // Initialize random number seed

	fillGameBoard(0, 0);

	/*
	bool is_need_regenerate = false;
	for (int i = 0; i < rowNum; i++)
	{
		for (int j = 0; j < colNum; j++)
		{
			if (_game_board[i][j].type == elementEliminateType)
			{
				is_need_regenerate = true;
			}
		}

		if (is_need_regenerate)
			break;
	}

	// FIXME: sometime will crash
	if (is_need_regenerate)
	{
		CCLOG("redraw game board");
		drawGameBoard();
		return;
	}*/
		

	// get screen size
	const Size kScreenSize = Director::getInstance()->getVisibleSize();
	const Vec2 kScreenOrigin = Director::getInstance()->getVisibleOrigin();

	// add elimination matrix
	float element_size = (kScreenSize.width - leftMargin - rightMargin) / colNum;
	
	for (int i = 0; i < rowNum; i++)
	{
		for (int j = 0; j < colNum; j++)
		{
			Element *element = Element::create();
			
			element->element_type = _game_board[i][j].type;
			element->setTexture(kElementImgArray[element->element_type]); 	
			element->setContentSize(Size(element_size, element_size)); // set size

			// add drop animation
			Point init_position(leftMargin + (j + 0.5) * element_size, bottonMargin + (i + 0.5) * element_size + 0.5 * element_size);
			element->setPosition(init_position);
			Point real_position(leftMargin + (j + 0.5) * element_size, bottonMargin + (i + 0.5) * element_size);
			Sequence *sequence = Sequence::create(MoveTo::create(0.5, real_position), CallFunc::create([=]() {
				element->setPosition(real_position); // lambda
			}), NULL);
			element->runAction(sequence);
		
			std::string elment_name = StringUtils::format("%d_%d", i, j);
			element->setName(elment_name); // Drop animation
			addChild(element, gameBoardLevel);	
		}
	}
}

void GameScene::dropElements(float dt)
{
	_is_can_touch = false;

	// get screen size
	const Size kScreenSize = Director::getInstance()->getVisibleSize();
	const Vec2 kScreenOrigin = Director::getInstance()->getVisibleOrigin();
	float element_size = (kScreenSize.width - leftMargin - rightMargin) / colNum;

	// sprite fall down
	for (int j = 0; j < colNum; j++)
	{
		std::vector<Element *> elements;
		for (int i = rowNum - 1; i >= 0; i--)
		{
			if (_game_board[i][j].type != elementEliminateType)
			{
				std::string element_name = StringUtils::format("%d_%d", i, j);
				Element *element = (Element *)getChildByName(element_name);
				elements.push_back(element);
			}
			else
				break; // add sprites
		}

		if (elements.size() == rowNum || elements.empty())
			continue;

		// inverted order
		std::reverse(elements.begin(), elements.end());

		// fall down
		int k = 0;
		int idx = 0;
		while (k < rowNum)
		{
			// find the first blank
			if (_game_board[k][j].type == elementEliminateType)
				break;
		
			k++;
		}

		for (int idx = 0; idx < elements.size(); idx++)
		{
			_game_board[k][j].type = elements[idx]->element_type;
			_game_board[k][j].marked = false;

			// set sprite position
			Point new_position(leftMargin + (j + 0.5) * element_size, bottonMargin + (k + 0.5) * element_size);
			Sequence *sequence = Sequence::create(MoveTo::create(0.1, new_position), CallFunc::create([=]() {
				elements[idx]->setPosition(new_position); // lambda
			}), NULL);
			elements[idx]->runAction(sequence);
			std::string new_name = StringUtils::format("%d_%d", k, j);
			elements[idx]->setName(new_name);

			k++;
		}

		while (k < rowNum)
		{
			_game_board[k][j].type = elementEliminateType;
			_game_board[k][j].marked = true;
			k++;
		}
		
	}

	// fill in the top gap
	fillVacantElements();

	// eliminate
	scheduleOnce(schedule_selector(GameScene::delayBatchEliminate), 0.9);

	_is_can_touch = true;
}

void GameScene::delayBatchEliminate(float dt)
{
	// continuous elimination
	auto eliminate_set = getEliminateSet();
	if (!eliminate_set.empty())
	{
		batchEliminate(eliminate_set);

		// complete elimination
		_is_can_elimate = eliminateInitFlag;

		// reset
		_start_pos.row = -1;
		_start_pos.col = -1;

		_end_pos.row = -1;
		_end_pos.col = -1;
	}
}

void GameScene::fillVacantElements()
{
	// get screen size
	const Size kScreenSize = Director::getInstance()->getVisibleSize();
	const Vec2 kScreenOrigin = Director::getInstance()->getVisibleOrigin();

	// add elimination matrix
	float element_size = (kScreenSize.width - leftMargin - rightMargin) / colNum;

	int len = kElementImgArray.size();

	srand(unsigned(time(0))); // Initialize random number seed

	// get blank
	for (int i = 0; i < rowNum; i++)
		for (int j = 0; j < colNum; j++)
		{
			if (_game_board[i][j].type == elementEliminateType)
			{
				int random_type = getRandomSpriteIndex(len);
				_game_board[i][j].type = random_type;
				_game_board[i][j].marked = false;

				Element *element = Element::create();

				element->element_type = _game_board[i][j].type;
				element->setTexture(kElementImgArray[element->element_type]); 	
				element->setContentSize(Size(element_size, element_size)); // set size

				Point real_position(leftMargin + (j + 0.5) * element_size, bottonMargin + (i + 0.5) * element_size);
				element->setPosition(real_position); // lambda
				
				// add appear animation
				element->appear();

				std::string elment_name = StringUtils::format("%d_%d", i, j);
				element->setName(elment_name); // name sprite
				addChild(element, gameBoardLevel);
			}
		}
}

void GameScene::swapElementPair(ElementPos p1, ElementPos p2, bool is_reverse)
{
	// no touch state during switching
	_is_can_touch = false;

	const Size kScreenSize = Director::getInstance()->getVisibleSize();
	const Vec2 kScreenOrigin = Director::getInstance()->getVisibleOrigin();
	float element_size = (kScreenSize.width - leftMargin - rightMargin) / colNum;

	// Memory exchange£¬sprite exchange£¬animation exchange

	// get sprite
	std::string name1 = StringUtils::format("%d_%d", p1.row, p1.col);
	std::string name2 = StringUtils::format("%d_%d", p2.row, p2.col);

	Element *element1 = (Element *)getChildByName(name1);
	Element *element2 = (Element *)getChildByName(name2);

	Point position1 = element1->getPosition();
	Point position2 = element2->getPosition();

	int type1 = element1->element_type;
	int type2 = element2->element_type;

	CCLOG(is_reverse ? "==== reverse move ====" : "==== normal move ====");

	CCLOG("before move");

	CCLOG("p1 name: %s", element1->getName().c_str());
	CCLOG("p2 name: %s", element2->getName().c_str());

	CCLOG("position1, x: %f, y: %f", element1->getPosition().x, element1->getPosition().y);
	CCLOG("position2, x: %f, y: %f", element2->getPosition().x, element2->getPosition().y);

	// memory exchange
	std::swap(_game_board[p1.row][p1.col], _game_board[p2.row][p2.col]);

	// move action
	float delay_time = is_reverse ? 0.5 : 0;
	DelayTime *move_delay = DelayTime::create(delay_time); 
	
	MoveTo *move_1to2 = MoveTo::create(0.2, position2);
	MoveTo *move_2to1 = MoveTo::create(0.2, position1);

	CCLOG("after move");
	element1->runAction(Sequence::create(move_delay, move_1to2, CallFunc::create([=]() {
		// lambda 
		// reset position
		CCLOG("e1 moved");
		element1->setPosition(position2);
		// exchange name
		element1->setName(name2);

		_is_can_elimate++;

		CCLOG("p1 name: %s", element1->getName().c_str());
		CCLOG("position1, x: %f, y: %f", element1->getPosition().x, element1->getPosition().y);
	}), NULL));
	element2->runAction(Sequence::create(move_delay, move_2to1, CallFunc::create([=]() {
		CCLOG("e2 moved");
		element2->setPosition(position1);
		element2->setName(name1);

		_is_can_elimate++;

		CCLOG("p2 name: %s", element2->getName().c_str());
		CCLOG("position2, x: %f, y: %f", element2->getPosition().x, element2->getPosition().y);
	}), NULL));

	// Restore touch
	_is_can_touch = true;
}

bool GameScene::hasEliminate()
{
	bool has_elminate = false;
	for (int i = 0; i < rowNum; i++)
	{
		for (int j = 0; j < colNum; j++)
		{
			if (_game_board[i][j].type != elementEliminateType)
			{
				// up and down judgment
				if (i - 1 >= 0
					&& _game_board[i - 1][j].type != elementEliminateType
					&& _game_board[i - 1][j].type == _game_board[i][j].type
					&& i + 1 < rowNum
					&& _game_board[i + 1][j].type != elementEliminateType
					&& _game_board[i + 1][j].type == _game_board[i][j].type)
				{
					has_elminate = true;
					break;
				}

				// left and right judgment
				if (j - 1 >= 0
					&& _game_board[i][j - 1].type != elementEliminateType
					&& _game_board[i][j - 1].type == _game_board[i][j].type
					&& j + 1 < colNum
					&& _game_board[i][j - 1].type != elementEliminateType
					&& _game_board[i][j + 1].type == _game_board[i][j].type)
				{
					has_elminate = true;
					break;
				}
			}
		}

		if (has_elminate)
			break;
	}

	return has_elminate;
}

// scan to eliminate sprites
std::vector<ElementPos> GameScene::getEliminateSet()
{
	std::vector<ElementPos> res_eliminate_list;
	// remove if there are more than or equal to 3 horizontally and vertically
	for (int i = 0; i < rowNum; i++)
		for (int j = 0; j < colNum; j++)
		{
			// up and down judgment
			if (i - 1 >= 0
				&& _game_board[i - 1][j].type == _game_board[i][j].type
				&& i + 1 < rowNum
				&& _game_board[i + 1][j].type == _game_board[i][j].type)
			{
				// add three connected vertical sprites
				if (!_game_board[i][j].marked && _game_board[i][j].type != elementEliminateType)
				{
					ElementPos pos;
					pos.row = i;
					pos.col = j;

					res_eliminate_list.push_back(pos);
					_game_board[i][j].marked = true;
				}
				if (!_game_board[i - 1][j].marked && _game_board[i - 1][j].type != elementEliminateType)
				{
					ElementPos pos;
					pos.row = i - 1;
					pos.col = j;

					res_eliminate_list.push_back(pos);
					_game_board[i - 1][j].marked = true;
				}
				if (!_game_board[i + 1][j].marked && _game_board[i + 1][j].type != elementEliminateType)
				{
					ElementPos pos;
					pos.row = i + 1;
					pos.col = j;

					res_eliminate_list.push_back(pos);
					_game_board[i + 1][j].marked = true;
				}
			}

			// left and right judgement
			if (j - 1 >= 0
				&& _game_board[i][j - 1].type == _game_board[i][j].type
				&& j + 1 < colNum
				&& _game_board[i][j + 1].type == _game_board[i][j].type)
			{
				// add three connected horizontal sprites
				if (!_game_board[i][j].marked && _game_board[i][j].type != elementEliminateType)
				{
					ElementPos pos;
					pos.row = i;
					pos.col = j;

					res_eliminate_list.push_back(pos);
					_game_board[i][j].marked = true;
				}
				if (!_game_board[i][j - 1].marked && _game_board[i][j - 1].type != elementEliminateType)
				{
					ElementPos pos;
					pos.row = i;
					pos.col = j - 1;

					res_eliminate_list.push_back(pos);
					_game_board[i][j - 1].marked = true;
				}
				if (!_game_board[i][j + 1].marked && _game_board[i][j + 1].type != elementEliminateType)
				{
					ElementPos pos;
					pos.row = i;
					pos.col = j + 1;

					res_eliminate_list.push_back(pos);
					_game_board[i][j + 1].marked = true;
				}
			}
		}

	return res_eliminate_list;
}

void GameScene::batchEliminate(const std::vector<ElementPos> &eliminate_list)
{
	// elimination sound effect
	SimpleAudioEngine::getInstance()->playEffect(popEffect.c_str());

	// disapear animation
	const Size kScreenSize = Director::getInstance()->getVisibleSize();
	const Vec2 kScreenOrigin = Director::getInstance()->getVisibleOrigin();
	float element_size = (kScreenSize.width - leftMargin - rightMargin) / colNum;

	for (auto &pos : eliminate_list)
	{
		std::string elment_name = StringUtils::format("%d_%d", pos.row, pos.col);
		Element *element = (Element *)(getChildByName(elment_name));
		_game_board[pos.row][pos.col].type = elementEliminateType; // eliminate
		element->setTexture(eliminateStartImg); // set image
		element->setContentSize(Size(element_size, element_size)); // set size
		element->vanish();
	}

	

	// encourage
	std::string combo_text;
	int len = eliminate_list.size();
	if (len >= 4)
		SimpleAudioEngine::getInstance()->playEffect(kunbelievableEffect.c_str());

	if (len == 4)
		combo_text = kComboTextArray[0];
	else if (len > 4 && len <= 6)
		combo_text = kComboTextArray[1];
	else if (len > 6)
		combo_text = kComboTextArray[2];
	_combo_label->setString(combo_text);
	_combo_label->setVisible(true);
	_combo_label->runAction(Sequence::create(MoveBy::create(0.5, Vec2(0, -50)), CallFunc::create([=]() {
		
		_combo_label->setVisible(false);
		_combo_label->setPosition(kScreenOrigin.x + kScreenSize.width / 2, kScreenOrigin.y + kScreenSize.height / 2);
	}), NULL));

	// add scores
	addScore(scoreUnit * eliminate_list.size());
	
	// sprites fall down
	scheduleOnce(schedule_selector(GameScene::dropElements), 0.5);

}


ElementPos GameScene::checkGameHint()
{
	// Judge the dead end

	ElementPos game_hint_point;
	game_hint_point.row = -1;
	game_hint_point.col = -1;

	for (int i = 0; i < rowNum; i++)
	{
		for (int j = 0; j < colNum; j++)
		{
			// up
			if (i < rowNum - 1)
			{
				// Judge after exchange, and then exchange back
				std::swap(_game_board[i][j], _game_board[i + 1][j]);
				if (hasEliminate())
				{
					game_hint_point.row = i;
					game_hint_point.col = j;

					std::swap(_game_board[i][j], _game_board[i + 1][j]);
					break;
				}
				std::swap(_game_board[i][j], _game_board[i + 1][j]);
			}

			// down
			if (i > 0)
			{
				std::swap(_game_board[i][j], _game_board[i - 1][j]);
				if (hasEliminate())
				{
					game_hint_point.row = i;
					game_hint_point.col = j;

					std::swap(_game_board[i][j], _game_board[i - 1][j]);
					break; 
				}
				std::swap(_game_board[i][j], _game_board[i - 1][j]);
			}

			// left
			if (j > 0)
			{
				std::swap(_game_board[i][j], _game_board[i][j - 1]);
				if (hasEliminate())
				{
					game_hint_point.row = i;
					game_hint_point.col = j;

					std::swap(_game_board[i][j], _game_board[i][j - 1]);
					break;
				}
				std::swap(_game_board[i][j], _game_board[i][j - 1]);
			}

			// right
			if (j < colNum - 1)
			{
				std::swap(_game_board[i][j], _game_board[i][j + 1]);
				if (hasEliminate())
				{
					game_hint_point.row = i;
					game_hint_point.col = j;

					std::swap(_game_board[i][j], _game_board[i][j + 1]);
					break;
				}
				std::swap(_game_board[i][j], _game_board[i][j + 1]);
			}
		}

		// if it is not the dead end,break
		if (game_hint_point.row != -1 && game_hint_point.col != -1)
			break;
	}

	return game_hint_point;
}

void GameScene::addScoreCallback(float dt)
{
	_animation_score++;
	_score_label->setString(StringUtils::format("score: %d", _animation_score));

	// Stop timer when adding scores
	if (_animation_score == _score)
		unschedule(schedule_selector(GameScene::addScoreCallback));
}

void GameScene::addScore(int delta_score)
{
	// add scores and time
	_score += delta_score;
	_progress_timer->setPercentage(_progress_timer->getPercentage() + 3.0);
	if (_progress_timer->getPercentage() > 100.0)
		_progress_timer->setPercentage(100.0);
	
	// Bonus animation
	schedule(schedule_selector(GameScene::addScoreCallback), 0.03);
}

void GameScene::tickProgress(float dt)
{
	// time out
	if (_progress_timer->getPercentage() > 0.0)
		_progress_timer->setPercentage(_progress_timer->getPercentage() - 1.0);
	else
	{
		_combo_label->setString("game over");
		_combo_label->setVisible(true);
		unschedule(schedule_selector(GameScene::tickProgress));
	}
		
}

void GameScene::update(float dt)
{
	if (_start_pos.row == -1 && _start_pos.col == -1
		&& _end_pos.row == -1 && _end_pos.col == -1)
		_is_can_elimate = eliminateInitFlag;

	CCLOG("eliminate flag: %d", _is_can_elimate);

	// check dead end
	ElementPos game_hint_point = checkGameHint();
	if (game_hint_point.row == -1 && game_hint_point.col == -1)
	{
		CCLOG("the game is dead");

		_combo_label->setString("dead game");
		_combo_label->setVisible(true);
		unschedule(schedule_selector(GameScene::tickProgress));
	}
	else
		CCLOG("game hint point: row %d, col %d", game_hint_point.row, game_hint_point.col);

	if (_is_can_elimate == eliminateTwoReadyFlag)
	{
		auto eliminate_set = getEliminateSet();
		if (!eliminate_set.empty())
		{
			batchEliminate(eliminate_set);

			// reset
			_is_can_elimate = eliminateInitFlag; 

			// reset
			_start_pos.row = -1;
			_start_pos.col = -1;

			_end_pos.row = -1;
			_end_pos.col = -1;
		}
		else
		{
			// there is nothing to eliminate
			if (_start_pos.row >= 0 && _start_pos.row < rowNum && _start_pos.col >= 0 && _start_pos.col < colNum
				&&_end_pos.row >= 0 && _end_pos.row < rowNum && _end_pos.row >= 0 && _start_pos.col < colNum
				&& (_start_pos.row != _end_pos.row || _start_pos.col != _end_pos.col))
			{
				// restore flag
				_is_can_elimate = eliminateInitFlag;
				swapElementPair(_start_pos, _end_pos, true);

				// reset
				_start_pos.row = -1;
				_start_pos.col = -1;

				_end_pos.row = -1;
				_end_pos.col = -1;
			}
				
		}
	}
}

bool GameScene::onTouchBegan(Touch *touch, Event *event)
{
	if (_is_can_touch)
	{
		// start touch position
		_start_pos = getElementPosByCoordinate(touch->getLocation().x, touch->getLocation().y);
		CCLOG("start pos, row: %d, col: %d", _start_pos.row, _start_pos.col);
		// moving
		_is_moving = true;
	}
	
	return true;

}

void GameScene::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event)
{
	if (_is_can_touch)
	{
		
		// drag sprite
		if (_start_pos.row > -1 && _start_pos.row < rowNum
			&& _start_pos.col > -1 && _start_pos.col < colNum)
		{
			// determine the direction of movement
			Vec2 cur_loacation = touch->getLocation();
			
			// only neighborhood exchange is allowed
			if (_end_pos.row == -1 && _end_pos.col == -1
				|| _end_pos.row == _start_pos.row && _end_pos.col == _start_pos.col)
				_end_pos = getElementPosByCoordinate(cur_loacation.x, cur_loacation.y);

			if (_is_moving)
			{
				// exchange sprites
				bool is_need_swap = false;

				CCLOG("cur pos, row: %d, col: %d", _end_pos.row, _end_pos.col);
				if (_start_pos.col + 1 == _end_pos.col && _start_pos.row == _end_pos.row) // right
					is_need_swap = true;
				else if (_start_pos.col - 1 == _end_pos.col && _start_pos.row == _end_pos.row) // left
					is_need_swap = true;
				else if (_start_pos.row + 1 == _end_pos.row && _start_pos.col == _end_pos.col) // up
					is_need_swap = true;
				else if (_start_pos.row - 1 == _end_pos.row && _start_pos.col == _end_pos.col) // down
					is_need_swap = true;

				if (is_need_swap)
				{
					// exchange
					swapElementPair(_start_pos, _end_pos, false);

					// stop moving
					_is_moving = false;
				}
			}

		}
	}
}

void GameScene::onTouchEnded(Touch *touch, Event *event)
{
	_is_moving = false;
}

void GameScene::onEnter()
{
	Layer::onEnter();
	CCLOG("enter game scene");
}

void GameScene::onExit()
{
	Layer::onExit();
	CCLOG("exit game scene");
}