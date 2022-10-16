#include "FixedScore.h"

#include <string>

FixedScore::FixedScore(int score, int darts, bool double_in_out) : STARTING_SCORE_(score), DART_COUNT_(darts), DOUBLE_IN_(double_in_out),
                                                                   player0_score_(score), player1_score_(score), darts_thrown_(0), turn_(false), victor_(nullptr)
{

}

void FixedScore::score(char* argv[])
{
	bool player = argv[0], doubled = argv[2];
	int score = std::stoi(argv[1]);
	// Add doubled logic for in & out
	if (!player)
		this->player0_score_ -= score;
	else
		this->player1_score_ -= score;
}

void FixedScore::undo()
{
}

void FixedScore::reset()
{
}

bool* FixedScore::get_victor()
{
}

void FixedScore::start(char* argv[])
{

}
