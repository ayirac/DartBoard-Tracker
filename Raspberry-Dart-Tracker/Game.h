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
protected:
	int player0_score_, player1_score_;
	Hit last_player0_hit_, last_player1_hit_;
	int who_threw_last_; // 0 none, 1 p0, 2 p1
	bool turn_; // false for p0, true for p1
public:
	// Reset the game's state to the start
	Game(int p0_scr, int p1_scr) : player0_score_(p0_scr), player1_score_(p1_scr), who_threw_last_(0) {}
	virtual void start() = 0;
	virtual void score(Hit h) = 0;
	virtual void undo() = 0;
	virtual void reset() = 0;
	virtual bool* get_victor() = 0;
	friend bool operator==(Game& lhs, Game& rhs);
	int get_score(bool p0) { if (p0) return this->player0_score_; return this->player1_score_; }
	Hit get_player_hit_record(bool p0) { if (p0) return this->last_player0_hit_; return this->last_player1_hit_; }
	bool get_turn() { return turn_; }
	int get_who_threw_last() { return this->who_threw_last_; }
	Hit get_last_hit() { if (who_threw_last_ == 1) { return last_player0_hit_; } else if (who_threw_last_ == 2) return last_player1_hit_; }
};

inline bool operator==(Game& lhs, Game& rhs)
{
	return (typeid(lhs) == typeid(rhs));
}
