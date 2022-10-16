#pragma once
#include "Game.h"


// Allows for games of 301, 501, and any other score based combination with options for amount of darts & doubling in flags
class FixedScore : public Game
{
private:
	const int STARTING_SCORE_, DART_COUNT_;
	const bool DOUBLE_IN_;
	int player0_score_, player1_score_;
	int darts_thrown_;
	bool* turn_; // NULL game not started, false p0, true p1
	bool* victor_;
	bool player0_doubled_, player1_doubled_;
public:
	// Default Game creation
	FixedScore();
	// Create a game with a starting score that must reach zero. Parameters for darts per turn & doubling in.
	FixedScore(int score, int darts, bool double_in_out);
	// Scores a player (false for player0, true for player1), with a flag for checking if it was a double for ending/starting the game.
	void score(Hit h); //  bool player, int score, bool doubled
	// Undo the last score
	void undo();
	// Reset the game's state to the start
	void reset();
	// Returns a bool pointer representing which player won. false for player0, true for player1, or NULL for no victor yet.
	bool* get_victor();
	// Starts the game given the parameters for player, score, and doubled in the args array
	void start();
};

