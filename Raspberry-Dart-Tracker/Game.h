#pragma once
#include <typeinfo>
#include <opencv2/core/types.hpp>

struct Hit
{
	cv::Point pos;
	int ID, multiplier; // 1x, 2x, 3x
	Hit(cv::Point pos, int id, int multi) : pos(pos), ID(id), multiplier(multi) {}
	Hit() : pos(cv::Point(0, 0)), ID(0), multiplier(0) {}
};

class Game
{
public:
	// Reset the game's state to the start
	virtual void start() = 0;
	virtual void score(Hit h) = 0;
	virtual void undo() = 0;
	virtual void reset() = 0;
	virtual bool* get_victor() = 0;
	friend bool operator==(Game& lhs, Game& rhs);
};

inline bool operator==(Game& lhs, Game& rhs)
{
	return (typeid(lhs) == typeid(rhs));
}
