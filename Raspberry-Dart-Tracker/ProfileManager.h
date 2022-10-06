#pragma once
#include <sstream>
#include <vector>
#include "Structs.h"

class ProfileManager
{
private:
	std::vector<std::vector<CircleParams>> circle_params_;
	std::vector<ThreshParams> thresh_params_;
	std::vector<LinesParams> line_params_;
	std::ostringstream saved_states_;
	unsigned current_profile_;
public:
	ProfileManager();
	void load_profile_file();
	CircleParams& get_circle(int ind);
	ThreshParams& get_thresh();
	LinesParams& get_line();
	unsigned get_current_profile() { return current_profile_; };
	void save_state(int state);
	void save_profile(int state);
	void display_profile(unsigned x);
};

