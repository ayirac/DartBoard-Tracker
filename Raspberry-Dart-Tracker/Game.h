#pragma once
#include <typeinfo>

class Game
{
public:
	// Reset the game's state to the start
	virtual void start(char* argv[]) = 0;
	virtual void score(char* argv[]) = 0;
	virtual void undo() = 0;
	virtual void reset() = 0;
	virtual bool* get_victor() = 0;
	friend bool operator==(Game& lhs, Game& rhs);
};

inline bool operator==(Game& lhs, Game& rhs)
{
	return (typeid(lhs) == typeid(rhs));
}
