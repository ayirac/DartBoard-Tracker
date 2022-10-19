#include "FixedScore.h"

#include <string>
#include <iostream>



FixedScore::FixedScore(int score, int darts, bool double_in_out) : STARTING_SCORE_(score), DART_COUNT_(darts), DOUBLE_IN_(double_in_out),
	Game(score, score), darts_thrown_(0), victor_(nullptr), player0_doubled_(false), player1_doubled_(false)
{
	this->turn_ = new bool(false);
}
FixedScore::FixedScore() : STARTING_SCORE_(0), DART_COUNT_(0), DOUBLE_IN_(nullptr), Game(0, 0){
	
}
void FixedScore::score(Hit h)
{
	// Add doubled logic for in & out

	darts_thrown_++;
	std::cout << "ID: " << h.ID << " Multi: " << h.multiplier << std::endl;

	
		
	
	
	if (!*this->turn_) { // Player0 turn
		this->last_player0_hit_.ID = h.ID;
		this->last_player0_hit_.multiplier = h.multiplier;
		if (h.multiplier == 2)
			this->player0_doubled_ = true;
		if (this->DOUBLE_IN_) {
			if (this->player0_doubled_) {
				this->player0_score_ -= h.ID * h.multiplier;
			}
		}
		else
			this->player0_score_ -= h.ID * h.multiplier;
	}
	else if (*this->turn_) { // Player1 turn
		this->last_player1_hit_.ID = h.ID;
		this->last_player1_hit_.multiplier = h.multiplier;
		if (h.multiplier == 2)
			this->player1_doubled_ = true;
		if (this->DOUBLE_IN_) {
			if (this->player1_doubled_) {
				this->player1_score_ -= h.ID * h.multiplier;
			}
		}
		else
			this->player1_score_ -= h.ID * h.multiplier;
	}
	std::cout << "P0 Sc: " << this->player0_score_ << " P1 Sc: " << this->player1_score_ << std::endl;

	if (darts_thrown_ + 1 > DART_COUNT_) { // Turn change
		*this->turn_ = !*this->turn_;
		darts_thrown_ = 0;
		std::cout << "Turn change.\n";
	}
}

void FixedScore::undo()
{
	return;
}

void FixedScore::reset()
{
	return;
}

bool* FixedScore::get_victor()
{
	return nullptr;
}

// 
void FixedScore::start()
{
	return;
}
