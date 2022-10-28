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
	darts_thrown_++;
	std::cout << "ID: " << h.ID << " Multi: " << h.multiplier << std::endl;

	if (!this->turn_) { // Player0 turn
		this->who_threw_last_ = 1;
		int score = this->player0_score_;
		this->last_player0_hit_.ID = h.ID;
		this->last_player0_hit_.multiplier = h.multiplier;
		if (h.multiplier == 2)
			this->player0_doubled_ = true;
		if (this->DOUBLE_IN_) { // Check if doubling in is flagged on.
			if (this->player0_doubled_) {
				score -= h.ID * h.multiplier;
				if (score < 0) // Don't update score if the player score's negative, indicating he missed his exit double
					std::cout << "No update, missed the double required to exit: ID: " << this->player0_score_ / 2 << " Multi: 2" << std::endl;
				else if (score == 0 && h.multiplier != 2) // Don't update, didn't hit double.
					std::cout << "No update, missed the double required to exit: ID: " << this->player0_score_ / 2 << " Multi: 2" << std::endl;
				else
					this->player0_score_ = score;
			}
		}
		else // No double flag, goal is to get to 0 in one hit with any multiplier.
		{
			score -= h.ID * h.multiplier;
			if (score < 0)
				std::cout << "No update, overshot the required points to exit: Pts: " << this->player0_score_ << std::endl;
			else
				this->player0_score_ = score;
		}
	}
	else if (this->turn_) { // Player1 turn
		this->who_threw_last_ = 2;
		int score = this->player1_score_;
		this->last_player1_hit_.ID = h.ID;
		this->last_player1_hit_.multiplier = h.multiplier;
		if (h.multiplier == 2)
			this->player1_doubled_ = true;
		if (this->DOUBLE_IN_) {
			if (this->player1_doubled_) {
				score -= h.ID * h.multiplier;
				if (score < 0) // Don't update score if the player score's negative, indicating he missed his exit double
					std::cout << "No update, missed the double required to exit: ID: " << this->player1_score_ / 2 << " Multi: 2" << std::endl;
				else if (score == 0 && h.multiplier != 2) // Don't update, didn't hit double.
					std::cout << "No update, missed the double required to exit: ID: " << this->player1_score_ / 2 << " Multi: 2" << std::endl;
				else
					this->player1_score_ = score;
			}
		}
		else // No double flag, goal is to get to 0 in one hit with any multiplier.
		{
			score -= h.ID * h.multiplier;
			if (score < 0)
				std::cout << "No update, overshot the required points to exit: Pts: " << this->player1_score_ << std::endl;
			else
				this->player1_score_ = score;
		}
	}
	std::cout << "P0 Sc: " << this->player0_score_ << " P1 Sc: " << this->player1_score_ << std::endl;

	// Check for victor
	if (this->player0_score_ == 0)
		victor_ = new bool(false);
	else if (this->player1_score_ == 0)
		victor_ = new bool(true);
	else
	{
		if (darts_thrown_ + 1 > DART_COUNT_) { // Turn change
			this->turn_ = !this->turn_;
			darts_thrown_ = 0;
			std::cout << "Turn change.\n";
		}
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
	return victor_;
}

// 
void FixedScore::start()
{
	return;
}
