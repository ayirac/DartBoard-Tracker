#include "DartBoard.h"
#include "Library.h"
#include <iostream>

DartBoard::DartBoard() : c_state_(-1), new_state_(true), MOG_frame_target_(60), MOG_frame_count_(0)
{
	for (int i = 0; i < 6; i++)
		this->boundaries_.push_back(new BoundaryCircle{ TYPE(i), cv::Vec3f(-1, -1, -1) });
	this->MOG2_ = createBackgroundSubtractorMOG2(60, 152, true);
}

// Draws circles on the board given dist, p1, p2, minR, maxR to find the playing-area boundary circle
cv::Mat DartBoard::calibrate_board(int dist, int p1, int p2, int min_R, int max_R)
{
	cv::Mat frame_gray;
	std::vector<cv::Vec3f> circles;
	this->frame_ = this->temp_frame_.clone();
	cv::cvtColor(this->frame_, frame_gray, cv::COLOR_BGR2GRAY);
	cv::GaussianBlur(frame_gray, frame_gray, cv::Size(5, 5), 2, 2);
	cv::HoughCircles(frame_gray, circles, cv::HOUGH_GRADIENT, 1.1, dist, p1, p2, min_R, max_R);
	cv::Point dartboard_center = cv::Point(this->frame_.cols / 2, this->frame_.rows / 2);
	double smallest_dist = 99999;
	for (size_t i = 0; i < circles.size(); i++)
	{
		cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		//int radius = cvRound(this->potential_circles_[i][2]);
		double distance_to_center = distance_between(dartboard_center, center);
		if (distance_to_center < smallest_dist)
		{
			smallest_dist = distance_to_center;
			this->outer_circle_ = circles[i];
		}
		
	}

	circle(this->frame_, Point(cvRound(this->outer_circle_[0]), cvRound(this->outer_circle_[1])), 3, cv::Scalar(0, 255, 0), -1, 8, 0);
	circle(this->frame_, Point(cvRound(this->outer_circle_[0]), cvRound(this->outer_circle_[1])), this->outer_circle_[2], cv::Scalar(0, 0, 255), 1, 8, 0);

	return frame_gray;
}

// Return the original frame with four corners of on the corkboard for future perspective transformation
cv::Mat DartBoard::locate_four_corners(cv::Mat& input_frame, cv::Scalar lows, cv::Scalar highs, int warpX, int warpY)
{
	// HSV Range segment
	cv::Mat hsv, thresheld, clone;
	cv::cvtColor(input_frame, hsv, cv::COLOR_BGR2HSV);
	cv::inRange(hsv, cv::Scalar(lows[0], lows[1], lows[2]), cv::Scalar(highs[0], highs[1], highs[2]), thresheld);
	cv::dilate(thresheld, thresheld, cv::Mat(), cv::Point(-1, 1), 2);
	clone = input_frame.clone();

	// Find largest contour
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	findContours(thresheld, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	cv::Mat drawing = cv::Mat::zeros(thresheld.size(), CV_8UC3);
	int largest = -5000;
	int largest_idx = 0;
	for (size_t i = 0; i < contours.size(); i++)
	{
		drawContours(clone, contours, (int)i, cv::Scalar(255, 0, 0), 2, cv::LINE_8, hierarchy, 0);

		if (static_cast<int>(contours[i].size()) > largest) {
			largest = contours[i].size();
			largest_idx = i;
		}
	}

	// Get contour corners (src points) & sort in order BL, TL, TR, BR & setup dst points
	if (!contours.empty())
	{
		std::vector<cv::Point> pnts;
		cv::approxPolyDP(contours[largest_idx], pnts, 0.05 * cv::arcLength(contours[largest_idx], true), true);
		if (pnts.size() == 4)
		{
			this->src_pnts[0] = pnts[1];
			this->src_pnts[1] = pnts[0];
			this->src_pnts[2] = pnts[3];
			this->src_pnts[3] = pnts[2];
			this->dst_pnts[0] = cv::Point2f(0, distance_between(pnts[1], pnts[0])* (static_cast<float>(warpY) / 100));
			this->dst_pnts[1] = cv::Point2f(0, 0);
			this->dst_pnts[2] = cv::Point2f(distance_between(pnts[3], pnts[0])* (static_cast<float>(warpX) / 100), 0);
			this->dst_pnts[3] = cv::Point2f(distance_between(pnts[3], pnts[0])* (static_cast<float>(warpX) / 100),  distance_between(pnts[1], pnts[0])* (static_cast<float>(warpY) / 100));
		}
		cv::Scalar cols[4] = { cv::Scalar(255,0,0), cv::Scalar(0,255,0), cv::Scalar(0,0,255),cv::Scalar(90,255,255) };
		for (int i = 0; i < pnts.size(); i++) // Visual representation of corners
			cv::circle(clone, pnts[i], 3, cols[i % 4], 3);
	}
	this->frame_ = clone;
	return thresheld;
}

// Crops out the playing-area boundary circle & saves the snapshot for checking difference for dart detection 
bool DartBoard::take_snapshot()
{
	cv::Point center(cvRound(this->outer_circle_[0]), cvRound(this->outer_circle_[1]));
	int radius = cvRound(this->outer_circle_[2]);
	cv::Mat roi(this->temp_frame_, cv::Rect(center.x - radius, center.y - radius, radius * 2, radius * 2));
	cv::Mat mask(roi.size(), roi.type(), cv::Scalar::all(0));
	circle(mask, cv::Point(radius, radius), radius, cv::Scalar::all(255), -1);
	this->frame_ = roi & mask;
	this->temp_frame_ = roi & mask;
	this->original_frame_ = roi & mask;

	return true;
}

// Warps the frame given parameters warpX & warpY
void DartBoard::take_perspective_transform(int warpX, int warpY)
{
	cv::Mat matrix = cv::getPerspectiveTransform(this->src_pnts, this->dst_pnts), warped;
	cv::Size size(distance_between(this->src_pnts[2], this->src_pnts[1]) * (static_cast<float>(warpX) / 100), distance_between(this->src_pnts[3], this->src_pnts[2]) * (static_cast<float>(warpY)/100));
	cv::warpPerspective(this->cam_frame_, warped, matrix, size, cv::INTER_LINEAR, cv::BORDER_CONSTANT);
	this->frame_ = warped;
	this->temp_frame_ = warped;
}

// Warps the frame given parameters warpX & warpY and returns the warped frame
cv::Mat DartBoard::take_perspective_transform(int warpX, int warpY, bool)
{
	cv::Mat matrix = cv::getPerspectiveTransform(this->src_pnts, this->dst_pnts), warped;
	cv::Size size(distance_between(this->src_pnts[2], this->src_pnts[1]) * (static_cast<float>(warpX) / 100), distance_between(this->src_pnts[3], this->src_pnts[2]) * (static_cast<float>(warpY) / 100));
	cv::warpPerspective(this->cam_frame_, warped, matrix, size, cv::INTER_LINEAR, cv::BORDER_CONSTANT);
	return warped;
}

// Return the frame_;
cv::Mat& DartBoard::get_frame() { return this->frame_; }

// Locate a boundary given params & assign boundary based on current state
cv::Mat DartBoard::locate_boundaries(CircleParams params)
{
	int th = 10;
	std::string bound_names[6] = { "BULLSEYE_INNER", "BULLEYES_OUTER", "TRIPLE_INNER", "TRIPLE_OUTER", "DOUBLE_INNER", "DOUBLE_OUTER" };
	cv::Mat frame_dartboard_gray;
	cv::Vec3f best_circle;
	std::vector<cv::Vec3f> potential_bullseyes;
	cv::cvtColor(this->frame_, frame_dartboard_gray, cv::COLOR_BGR2GRAY);
	cv::Point dartboard_center = cv::Point(this->frame_.cols / 2, this->frame_.rows / 2);
	double smallest_dist = 99999;
	cv::GaussianBlur(frame_dartboard_gray, frame_dartboard_gray, cv::Size(5, 5), 2, 2);
	cv::HoughCircles(frame_dartboard_gray, potential_bullseyes, cv::HOUGH_GRADIENT, 1, params.dist, params.p1, params.p2, params.minR, params.maxR);
	for (size_t i = 0; i < potential_bullseyes.size(); i++)
	{
		cv::Point center(cvRound(potential_bullseyes[i][0]), cvRound(potential_bullseyes[i][1]));
		int radius = cvRound(potential_bullseyes[i][2]);
		double distance_to_center = distance_between(dartboard_center, center);
		if (distance_to_center < smallest_dist)
		{
			smallest_dist = distance_to_center;
			best_circle = potential_bullseyes[i];
		}
	}

	this->boundaries_[this->boundaries_[c_state_-1]->type]->circ = best_circle;

	return frame_dartboard_gray;
}

// Returns the warped playing board with all boundaries and segments shown
cv::Mat DartBoard::get_frame_segments()
{
	Mat frame_segments = this->temp_frame_.clone();

	cv::Scalar colors[3] = { Scalar(255, 0, 0), Scalar(0, 255, 0), Scalar(0, 0, 255) };
	for (int i = 0; i < 6; i++)
	{
		if (this->c_state_ > i) // Draw boundaries (circles)
		{
			int val = ((i - 1) / 2) - ((pow((-1), i - 1) - 1) / -4);
			cv::circle(frame_segments, Point(this->boundaries_[i]->circ[0], this->boundaries_[i]->circ[1]), this->boundaries_[i]->circ[2], colors[val], 2, 8, 0);
		}

	}
	if (this->c_state_ == 8)
	{
		for (int i = 0; i < this->segments_.size(); i++) // Draw segment
		{
			line(frame_segments, Point(this->segments_[i].lines[0][0], this->segments_[i].lines[0][1]), Point(this->segments_[i].lines[0][2], this->segments_[i].lines[0][3]), colors[i % 3], 3, 8);
			line(frame_segments, Point(this->segments_[i].lines[1][0], this->segments_[i].lines[1][1]), Point(this->segments_[i].lines[1][2], this->segments_[i].lines[1][3]), colors[i % 3], 3, 8);
			Point priamry = find_further_point(Point(this->segments_[i].lines[0][0], this->segments_[i].lines[0][1]), Point(this->segments_[i].lines[0][2], this->segments_[i].lines[0][3]), Point(frame_segments.cols / 2, frame_segments.rows / 2));

			putText(frame_segments, to_string(this->segments_[i].ID), Point(priamry.x - 10, priamry.y - 5), FONT_HERSHEY_COMPLEX, 0.9, colors[i % 3], 1);
		}
	}
	else if (this->c_state_ == 7)
	{
		for (int i = 0; i < this->lines_.size(); i++) // Draw lines
		{
			line(frame_segments, Point(this->lines_[i][0], this->lines_[i][1]), Point(this->lines_[i][2], this->lines_[i][3]), colors[i % 3], 3, 8);
			Point dir = find_further_point(Point(this->lines_[i][0], this->lines_[i][1]), Point(this->lines_[i][2], this->lines_[i][3]),
				Point(frame_segments.cols / 2, frame_segments.rows / 2));


			circle(frame_segments, find_most_centered_point(Point(this->lines_[i][0], this->lines_[i][1]), Point(this->lines_[i][2], this->lines_[i][3]),
				Point(frame_segments.cols / 2, frame_segments.rows / 2)), 1, Scalar(142, 192, 71), 7, 8, 0);
			putText(frame_segments, to_string(i), dir, FONT_HERSHEY_COMPLEX, 0.9, colors[i % 3], 1);
			std::cout << i << ": " << atan2(dir.x - frame_segments.cols / 2, dir.y - frame_segments.rows / 2) << endl;

		}
	}

	return frame_segments;
}

// Locate the 20 lines on the dartboard and sort them according to their arctan2 angle for future segment creation.
cv::Mat DartBoard::locate_singles(int p1, int p2, int p3, int e_p1, int e_p2)
{
	// pipeline: hsv -> detect black & white
	Mat frame_blur, frame_HSV, frame_threshold, frame_edges;
	Mat kernel = Mat(5, 7, CV_8UC1);
	Mat kernel2 = Mat(3, 3, CV_8UC1);
	
	medianBlur(this->frame_, frame_blur, 3);
	cvtColor(frame_blur, frame_HSV, COLOR_BGR2GRAY);
	//inRange(frame_HSV, low_bound, high_bound, frame_threshold);
	morphologyEx(frame_HSV, frame_HSV, MORPH_CLOSE, kernel);
	Canny(frame_HSV, frame_edges, e_p1, e_p2, 3); // 10, 200, 5);
	dilate(frame_edges, frame_edges, kernel2, Point(1, -1), 1);

	// draw black circle middle to remove bullseye
	Point cent = Point(this->get_boundary(BULLEYES_OUTER)->circ[0], this->get_boundary(BULLEYES_OUTER)->circ[1]);
	circle(frame_edges, cent, this->get_boundary(BULLEYES_OUTER)->circ[2], Scalar(0, 0, 0), FILLED, 8, 0);
	HoughLinesP(frame_edges, this->lines_, 1, CV_PI / 180, p1, p2, p3);

	// sort lines
	if (!this->lines_.empty()) {
		for (int e = 0; e < this->lines_.size() - 1; e++)
		{
			for (int j = 0; j < this->lines_.size() - e - 1; j++)
			{
				Point dir_1 = find_further_point(Point(this->lines_[j][0], this->lines_[j][1]), Point(this->lines_[j][2], this->lines_[j][3]), // comparing j & j+1's non-origin angles, bub sort
					Point(frame_edges.cols / 2, frame_edges.rows / 2));
				Point dir_2 = find_further_point(Point(this->lines_[j + 1][0], this->lines_[j + 1][1]), Point(this->lines_[j + 1][2], this->lines_[j + 1][3]),
					Point(frame_edges.cols / 2, frame_edges.rows / 2));
				if (atan2(dir_1.x - frame_edges.cols / 2, dir_1.y - frame_edges.rows / 2) > atan2(dir_2.x - frame_edges.cols / 2, dir_2.y - frame_edges.rows / 2))
					std::swap(this->lines_[j], this->lines_[j + 1]);
			}
		}
	}
	

	int max_distance_away = 30;
	double line_angle_threshold = 0.032;
	
	for (int i = 0; i < this->lines_.size(); i++)
	{

		Point og_dir = find_further_point(Point(this->lines_[i][0], this->lines_[i][1]), Point(this->lines_[i][2], this->lines_[i][3]), cent);
		/* cap distance
		double distance = distance_between(find_most_centered_point(Point(this->lines_[i][0], this->lines_[i][1]), Point(this->lines_[i][2], this->lines_[i][3]), cent), cent);
		if (max_distance_away < distance)
		{
			this->lines_.erase(this->lines_.begin() + i);
			i--;
		}*/
		for (int e = 0; e < this->lines_.size(); e++) // Get rid of overlapping lines
		{
			Point secondary_dir = find_further_point(Point(this->lines_[e][0], this->lines_[e][1]), Point(this->lines_[e][2], this->lines_[e][3]), cent);
			if (og_dir != secondary_dir)
			{
				double angle_difference = abs(atan2(secondary_dir.x - frame_edges.cols / 2, secondary_dir.y - frame_edges.rows / 2) - atan2(og_dir.x - frame_edges.cols / 2, og_dir.y - frame_edges.rows / 2));
				if (angle_difference == 0)
					break;
				if (angle_difference < line_angle_threshold)
				{
					
					if (distance_between(og_dir, cent) < distance_between(secondary_dir, cent)) // take larger one
					{

						this->lines_.erase(this->lines_.begin() + i); // take sec
						i--;
						break;
					}
					else
					{
						this->lines_.erase(this->lines_.begin() + e); // take og
						e--;
					}
				}
			}

		}
	}
	
	// pop last one & move to front
	this->lines_.insert(this->lines_.begin(), this->lines_[this->lines_.size() - 1]);
	this->lines_.pop_back();


	return frame_edges;
}

// Check for what boundaries were hit given a Point.
void DartBoard::check_hit(cv::Point& hit)
{
	// Check for multiplier/special region
	cout << boolalpha;
	bool hit_inside = false;
	if (segment_hit(hit, this->boundaries_[TYPE::DOUBLE_OUTER]->circ, this->boundaries_[TYPE::DOUBLE_INNER]->circ))
	{
		cout << "Hit double: " << hit << "!\n";
		hit_inside = true;
	}
	else if (segment_hit(hit, this->boundaries_[TYPE::TRIPLE_OUTER]->circ, this->boundaries_[TYPE::TRIPLE_INNER]->circ))
	{
		cout << "Hit triple: " << hit << "!\n";
		hit_inside = true;
	}
	else if (segment_hit(hit, this->boundaries_[TYPE::DOUBLE_INNER]->circ, this->boundaries_[TYPE::TRIPLE_OUTER]->circ))
	{
		cout << "Hit outer-single:: " << hit << "!\n";
		hit_inside = true;
	}
	else if (segment_hit(hit, this->boundaries_[TYPE::TRIPLE_INNER]->circ, this->boundaries_[TYPE::BULLEYES_OUTER]->circ))
	{
		cout << "Hit inner-single: " << hit << "!\n";
		hit_inside = true;
	}
	else if (segment_hit(hit, this->boundaries_[TYPE::BULLEYES_OUTER]->circ, this->boundaries_[TYPE::BULLSEYE_INNER]->circ))
		cout << "Hit Green Bullseye: " << hit << "!\n";
	else if (segment_hit(hit, this->boundaries_[TYPE::BULLSEYE_INNER]->circ))
		cout << "Hit Red Bullseye: " << hit << "!\n";
		

	// Check for specific segment if hit inside (non-bullseye)
	bool hit_segment = false;
	Point cent(this->frame_.cols / 2, this->frame_.rows / 2);
	if (hit_inside)
	{
		for (int i = 0; i < this->segments_.size(); i++)
		{
			double slope1 = (static_cast<double>(this->segments_[i].lines[0][1]) - this->segments_[i].lines[0][3]) / (static_cast<double>(this->segments_[i].lines[0][0]) - this->segments_[i].lines[0][2]);
			double slope2 = (static_cast<double>(this->segments_[i].lines[1][1]) - this->segments_[i].lines[1][3]) / (static_cast<double>(this->segments_[i].lines[1][0]) - this->segments_[i].lines[1][2]);
			double y_int1 = this->segments_[i].lines[0][1] - slope1 * this->segments_[i].lines[0][0];
			double y_int2 = this->segments_[i].lines[1][1] - slope2 * this->segments_[i].lines[1][0];
			
			

			if (hit.x > cent.x)
			{
				bool under_top = (hit.y < (slope1* hit.x) + y_int1), over_bot = (hit.y > (slope2 * hit.x) + y_int2);
				if (under_top && over_bot && (find_most_centered_point(Point(this->segments_[i].lines[0][0], this->segments_[i].lines[0][1]), Point(this->segments_[i].lines[0][0], this->segments_[i].lines[0][1]), cent).x > cent.x))
				{
					cout << "Hmm.. hit.. " << this->segments_[i].ID << endl;
					hit_segment = true;
					break;
				}
				
			}
			else
			{
				bool under_top = (hit.y > (slope1 * hit.x) + y_int1), over_bot = (hit.y < (slope2* hit.x) + y_int2);
				if (under_top && over_bot && (find_most_centered_point(Point(this->segments_[i].lines[0][0], this->segments_[i].lines[0][1]), Point(this->segments_[i].lines[0][0], this->segments_[i].lines[0][1]), cent).x < cent.x))
				{
					cout << "Hmm.. hit.. " << this->segments_[i].ID << endl;
					hit_segment = true;
					break;
				}
			}
			// line equations
			//cout << this->segments_[i].ID << ": y = " << slope1 << "x + " << y_int1 << endl;
			//cout << "y = " << slope2 << "x + " << y_int2 << endl;
		}
		if (!hit_segment)
		{
			if (hit.y < cent.y)
				cout << "Hmm.. hit.. " << this->segments_[0].ID << endl;
			if (hit.y > cent.y)
				cout << "Hmm.. hit.. " << this->segments_[10].ID << endl;
		}
		
	}
	
}

// Checks the playing area for a hit on a segment given a Point, and two Vec3f circles.
bool DartBoard::segment_hit(cv::Point& hit, cv::Vec3f outer, cv::Vec3f inner)
{
	bool in_interior = false, in_exterior = false;
	if (sqrt(pow(hit.x - outer[0], 2) + pow(hit.y - outer[1], 2)) < outer[2])
		in_exterior = true;
	if (sqrt(pow(hit.x - inner[0], 2) + pow(hit.y - inner[1], 2)) < inner[2])
		in_interior = true;

	if (in_exterior && !in_interior)
		return true;
	return false;
}

// Checks the playing area for a hit on a segment given a Point and one Vec3f circles, specifically for inner bullseye
bool DartBoard::segment_hit(cv::Point& hit, cv::Vec3f outer)
{
	if (sqrt(pow(hit.x - outer[0], 2) + pow(hit.y - outer[1], 2)) < outer[2])
		return true;
	return false;
}

// Create a segment given two line indexs & assigns the segment the correct ID
void DartBoard::create_segment(int x, int g)
{
	int ccw_ids[20] = { 20, 5, 12, 9, 14, 11, 8, 16, 7, 19, 3, 17, 2, 15, 10, 6, 13, 4, 18, 1 };
	Segment seg;
	seg.ID = ccw_ids[this->segments_.size()];
	seg.lines[0] = this->lines_[x];
	if (g == this->lines_.size()) // Wraps back from 1 to 20
		seg.lines[1] = this->lines_[0];
	else
		seg.lines[1] = this->lines_[g];

	this->segments_.push_back(seg);
}

// Creates segments given sorted lines by merging two lines together, sets the state out of calibration mode
void DartBoard::lock_in_segment_lines()
{
	if (this->lines_.size() == 20)
	{
		for (int i = 0; i < this->lines_.size(); i++)
		{
			this->create_segment(i, i + 1);
		}
	}
	this->set_state(8);
}


// PROTOTYPING: Returns a matrix containing contours of the darts that were thrown
cv::Mat DartBoard::check_darts(int p1, int p2)
{
	// OLD need to perspective transform new input frame...
	/*
	
	//return cropped_board;

	cv::absdiff(this->original_frame_, cropped_board, difference);
	cv::medianBlur(difference, difference, 3);
	cv::dilate(difference, difference, Mat());

	// OLD Get the mask if difference greater than th
	
	int th = 115;  // 0
	Mat mask2(original_frame_.size(), CV_8UC1, cv::Scalar(0, 0, 0));
	for (int j = 0; j < difference.rows; ++j) {
		for (int i = 0; i < difference.cols; ++i) {
			cv::Vec3b pix = difference.at<cv::Vec3b>(j, i);
			int val = (pix[0] + pix[1] + pix[2]);
			if (val > th) {
				mask2.at<unsigned char>(j, i) = 255;
			}
		}
	}
	
	cv::erode(difference, difference, Mat());
	//cv::dilate(difference, difference, Mat());
	//cv::dilate(difference, diffefisrence, Mat());*/

	// Get ROI - replace later
	cv::Mat board = this->take_perspective_transform(p1, p2, bool()), difference;
	cv::Point center(cvRound(this->outer_circle_[0]), cvRound(this->outer_circle_[1]));
	int radius = cvRound(this->outer_circle_[2]);
	cv::Mat roi(board, cv::Rect(center.x - radius, center.y - radius, radius * 2, radius * 2));
	cv::Mat mask(roi.size(), roi.type(), cv::Scalar::all(0));
	circle(mask, cv::Point(radius, radius), radius, cv::Scalar::all(255), -1);
	cv::Mat cropped_board = roi & mask;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	cv::Mat foreground_mask = this->locate_dart_MOG2(p1, p2);

	findContours(foreground_mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	
	int min_area = 20;
	
	this->original_frame_ = cropped_board; // new board
	if (!contours.empty())
	{
		int max = -999;
		//int dart_cnt = 0;
		Mat drawing = cropped_board.clone();
		vector<Point>* dart_contour = &contours[0];

		for (size_t i = 0; i < contours.size(); i++)
		{
			if (contourArea(contours[i]) < min_area)
			{
				contours.erase(contours.begin() + i);
				i--;
				continue;
			}
			if (contourArea(contours[i]) > max)
			{
				max = contourArea(contours[i]);
				dart_contour = &contours[i];
			}
			//drawContours(drawing, contours, int(i), Scalar(0, 0, 255), 2, LINE_8, hierarchy, 0);
			cout << i << ": " << contourArea(contours[i]) << endl;
		}

		if (!contours.empty())
		{
			vector<vector<Point>> dart_container;
			dart_container.push_back(*dart_contour);

			drawContours(drawing, dart_container, int(0), Scalar(255, 0, 0), 2, LINE_8, hierarchy, 0);
			// draw western dot
			int min = 99999;
			Point western;
			for (int i = 0; i < dart_contour->size(); i++)
			{
				if (dart_contour->at(i).x < min)
				{
					min = dart_contour->at(i).x;
					western = dart_contour->at(i);
				}
			}
			// cont here on main pc
			circle(drawing, western, 3, Scalar(0, 255, 0), -1);
			check_hit(western);
			return drawing;
		}
		
	}

	return foreground_mask;
}

// Return the current state of the board
int& DartBoard::get_state() { return this->c_state_; }

// Set the state of the board & toggle new_state_
void DartBoard::set_state(int s)
{
	this->c_state_ = s;
	this->new_state_ = true;
}

// Check for a new state in the board
bool DartBoard::state_change()
{
	if (this->new_state_)
	{
		this->new_state_ = false;
		return true;
	}
	return false;
}

// Return a boundary given the TYPE
BoundaryCircle* DartBoard::get_boundary(TYPE t)
{
	for (int i = 0; i < this->boundaries_.size(); i++)
		if (this->boundaries_[i]->type = t)
			return this->boundaries_[i];
	return nullptr;
}

// Reset the segments
void DartBoard::reset_segments()
{
	this->segments_.clear();
	this->MOG_frame_count_ = 0;
}

cv::Mat DartBoard::get_playing_area(int p1, int p2)
{
	cv::Mat board = this->take_perspective_transform(p1, p2, bool()), difference;
	cv::Point center(cvRound(this->outer_circle_[0]), cvRound(this->outer_circle_[1]));
	int radius = cvRound(this->outer_circle_[2]);
	cv::Mat roi(board, cv::Rect(center.x - radius, center.y - radius, radius * 2, radius * 2));
	cv::Mat mask(roi.size(), roi.type(), cv::Scalar::all(0));
	circle(mask, cv::Point(radius, radius), radius, cv::Scalar::all(255), -1);
	cv::Mat cropped_board = roi & mask;
	return cropped_board;
}

void DartBoard::capture_MOG2(int warpX, int warpY)
{
	if (this->MOG_frame_count_ < this->MOG_frame_target_)
	{
		cv::Mat background_mask(this->get_frame().rows, this->get_frame().cols, CV_8UC1);
		cv::Mat dartboard = this->get_playing_area(warpX, warpY);

		this->MOG2_->apply(dartboard, background_mask);
		imshow("mask", background_mask);
		this->MOG_frame_count_++;
	}
	cv::Mat output(this->get_frame().rows, this->get_frame().cols, CV_8UC1);
	this->MOG2_->getBackgroundImage(output);
	imshow("output", output);
}

cv::Mat DartBoard::locate_dart_MOG2(int warpX, int warpY)
{
	cv::Mat output(this->get_frame().rows, this->get_frame().cols, CV_8UC1), background_mask(this->get_frame().rows, this->get_frame().cols, CV_8UC1),
		dartboard = this->get_playing_area(warpX, warpY), thrframe;

	this->MOG2_->apply(dartboard, background_mask, 0);
	this->MOG2_->getBackgroundImage(output);

	cv::threshold(background_mask, thrframe, 127, 255, cv::THRESH_BINARY);
	Mat kernel = getStructuringElement(MORPH_RECT, Size(1, 1));
	//erode(thrframe, thrframe, kernel, Point(-1, -1), 1);
	kernel = getStructuringElement(MORPH_RECT, Size(7, 5));
	dilate(thrframe, thrframe, kernel, Point(-1, -1), 3);
	kernel = getStructuringElement(MORPH_RECT, Size(5, 3));
	erode(thrframe, thrframe, kernel, Point(-1, -1), 3);

	return thrframe;
}
