#include "ProfileManager.h"

#include <fstream>
#include <iomanip>
#include <iostream>

ProfileManager::ProfileManager() :current_profile_(0) {}

// Load the profile file & allows the user to select which profile to load, creates a default profile if none exists
void ProfileManager::load_profile_file()
{
	bool corrupt_profile = false;
	char eater;
	int dist;
	std::ifstream profiles("profile.dat");
	if (profiles.good())
	{
		int n = 0; // current profile


		while (!profiles.eof() && !corrupt_profile) // Loads all profiles into an array
		{
			int numberParams = 0;
			// Push back empty params into parameters vectors
			this->thresh_params_.push_back(ThreshParams());
			std::vector<CircleParams> circle_values;
			for (int i = 0; i < 7; i++)
				circle_values.push_back(CircleParams());
			this->circle_params_.push_back(circle_values);
			this->line_params_.push_back(LinesParams());

			// Load Thresh parameters
			profiles >> this->thresh_params_[n].thresh >> eater >> this->thresh_params_[n].lowH >> eater >> this->thresh_params_[n].lowS >> eater >> this->thresh_params_[n].lowV >> eater
				>> this->thresh_params_[n].highH >> eater >> this->thresh_params_[n].highS >> eater >> this->thresh_params_[n].highV >> eater >> this->thresh_params_[n].warpX >> eater >> this->thresh_params_[n].warpY;
			// Loads Circle parameters
			for (int i = 0; i < 7; i++)
			{
				profiles >> dist >> eater; // Check for invalid profile #999 & set default values
				if (dist == 999) {
					std::cout << "Profile not complete, configure manually.\n";
					for (int i = 0; i < 8 - numberParams; i++)
						this->circle_params_[0][i].dist = 80, this->circle_params_[0][i].p1 = 100, this->circle_params_[0][i].p2 = 120, this->circle_params_[0][i].minR = 100, this->circle_params_[0][i].maxR = 200;
					this->line_params_[n - 1].p1 = 100, this->line_params_[n - 1].p2 = 100, this->line_params_[n - 1].p3 = 100, this->line_params_[n - 1].e_p1 = 80, this->line_params_[n - 1].e_p2 = 80;
					corrupt_profile = true;
					break;
				}
				this->circle_params_[n][i].dist = dist;
				profiles >> this->circle_params_[n][i].p1 >> eater;
				profiles >> this->circle_params_[n][i].p2 >> eater;
				profiles >> this->circle_params_[n][i].minR >> eater;
				profiles >> this->circle_params_[n][i].maxR;
				numberParams++;
			} // Loads the Line parameters
			profiles >> dist >> eater; // Check for invalid profile #999 & set default values
			if (dist == 999) {
				std::cout << "Profile not complete, configure manually.\n";
				for (int i = 0; i < 8 - numberParams; i++)
					this->circle_params_[0][i].dist = 80, this->circle_params_[0][i].p1 = 100, this->circle_params_[0][i].p2 = 120, this->circle_params_[0][i].minR = 100, this->circle_params_[0][i].maxR = 200;
				this->line_params_[n - 1].p1 = 100, this->line_params_[n - 1].p2 = 100, this->line_params_[n - 1].p3 = 100, this->line_params_[n - 1].e_p1 = 80, this->line_params_[n - 1].e_p2 = 80;
				corrupt_profile = true;
				break;
			}
			this->line_params_[n].p1 = dist;
			profiles >> this->line_params_[n].p2 >> eater;
			profiles >> this->line_params_[n].p3 >> eater;
			profiles >> this->line_params_[n].e_p1 >> eater;
			profiles >> this->line_params_[n].e_p2;
			n++;
		}
		profiles.close();

		if (n != 1)
		{
			do
			{
				std::cout << "Choose between profile 0 and profile " << n - 1 << ".\n";
				std::cin >> this->current_profile_;
			} while (this->current_profile_ > n || this->current_profile_ < 0);
		}
		std::cout << "Loaded profile " << this->current_profile_ << std::endl;
		
	}
	else { // Set default values if no profile file exists
		this->thresh_params_[0].thresh = 20, this->thresh_params_[0].lowH = 100, this->thresh_params_[0].lowS = 100, this->thresh_params_[0].lowV = 100,
			this->thresh_params_[0].highH = 100, this->thresh_params_[0].highS = 100, this->thresh_params_[0].highV = 100, this->thresh_params_[0].warpX = 100, this->thresh_params_[0].warpY = 100;
		for (int i = 0; i < 8; i++)
			this->circle_params_[0][i].dist = 80, this->circle_params_[0][i].p1 = 100, this->circle_params_[0][i].p2 = 120, this->circle_params_[0][i].minR = 100, this->circle_params_[0][i].maxR = 200;
		this->line_params_[0].e_p1 = 80, this->line_params_[0].e_p2 = 80, this->line_params_[0].p1 = 100, this->line_params_[0].p2 = 120, this->line_params_[0].p3 = 120;
		std::cout << "File not found, setting default values." << std::endl;
	}
	this->display_profile(this->current_profile_);

}

// Returns the profiles circle parameters
CircleParams& ProfileManager::get_circle(int ind)
{
	return this->circle_params_[this->current_profile_][ind];
}

// Returns the profile's thresh parameters
ThreshParams& ProfileManager::get_thresh()
{
	return this->thresh_params_[this->current_profile_];
}

// Return the profile's line parameters
LinesParams& ProfileManager::get_line()
{
	return this->line_params_[this->current_profile_];
}

// Save the state of the profile
void ProfileManager::save_state(int state)
{
	if (state == -1) {
		this->saved_states_ << this->get_thresh().thresh << ',' << this->get_thresh().lowH << ',' << this->get_thresh().lowS << ',' << this->get_thresh().lowV << ','
			<< this->get_thresh().highH << ',' << this->get_thresh().highS << ',' << this->get_thresh().highV << ','
			<< this->get_thresh().warpX << ',' << this->get_thresh().warpY << '\n';
	}
	else if (state < 7 && state >= 0) {
		this->saved_states_ << this->get_circle(state).dist << ',' << this->get_circle(state).p1 << ',' << this->get_circle(state).p2 << ','
			<< this->get_circle(state).minR << ',' << this->get_circle(state).maxR << '\n';
	}
	else if (state == 7) {
		this->saved_states_ << this->get_line().p1 << ',' << this->get_line().p2 << ',' << this->get_line().p3 << ','
			<< this->get_line().e_p1 << ',' << this->get_line().e_p2 << '\n';
	}
}

// Save and append the profile to a file, creates a file if it doesn't exist.
void ProfileManager::save_profile(int state)
{
	std::ifstream iprofile("profile.dat"); // Check if profile file exists
	if (iprofile.good())
	{
		std::ofstream oprofile("profile.dat", std::ios_base::app);
		oprofile << "\n" << this->saved_states_.str();
		for (int i = 0; i < 8 - state; i++) // Add -, to indicate a missing field
			if (i != 7 - state)
				oprofile << "999\n";
	}
	else {
		std::ofstream oprofile("profile.dat");
		oprofile << this->saved_states_.str();
		for (int i = 0; i < 8 - state; i++) // Add -, to indicate a missing field
			oprofile << "999\n";

	}
	iprofile.close();
}

// Prints the values of a profile to console
void ProfileManager::display_profile(unsigned x)
{
	std::cout << "Profile " << this->current_profile_ << ":\n";
	std::cout << std::setw(6) << "thresh" << std::setw(6) << "lowH" << std::setw(6) << "lowS" << std::setw(6) << "lowV"
		<< std::setw(6) << "highH" << std::setw(6) << "highS" << std::setw(6) << "highV" << std::endl;
	std::cout << std::setw(6) << this->thresh_params_[x].thresh << std::setw(6) << this->thresh_params_[x].lowH << std::setw(6) << this->thresh_params_[x].lowS << std::setw(6) << this->thresh_params_[x].lowV
		<< std::setw(6) << this->thresh_params_[x].highH << std::setw(6) << this->thresh_params_[x].highS << std::setw(6) << this->thresh_params_[x].highV << std::endl << std::endl;

	std::cout << std::setw(6) << "dist" << std::setw(6) << "p1" << std::setw(6) << "p2" << std::setw(6) << "minR" << std::setw(6) << "maxR" << std::endl;
	for (int i = 0; i < circle_params_[x].size(); i++)
	{
		std::cout << std::setw(6) << circle_params_[x][i].dist << std::setw(6) << circle_params_[x][i].p1 << std::setw(6) << circle_params_[x][i].p2 << std::setw(6) << circle_params_[x][i].minR
			<< std::setw(6) << circle_params_[x][i].maxR << std::endl;
	}
	std::cout << std::endl;

	std::cout << std::setw(6) << "p1" << std::setw(6) << "p2" << std::setw(6) << "p3" << std::setw(6) << "minR"
		<< std::setw(6) << "e_p1" << std::setw(6) << "e_p2" << std::endl;
	std::cout << std::setw(6) << this->line_params_[x].p1 << std::setw(6) << this->line_params_[x].p2 << std::setw(6) << this->line_params_[x].p3 << std::setw(6) << this->line_params_[x].e_p1
		<< std::setw(6) << this->line_params_[x].e_p2 << std::endl;
}
