#include "DartBoard.h"
#include "Library.h"
#include <iostream>

DartBoard::DartBoard() : snapshot_taken_(false), segment_lines_calibrated_(false), doubles_calibrated_(false), triples_calibrated_(false), outer_bullseye_calibrated_(false), inner_bullseye_calibrated_(false)
{
}

bool DartBoard::snapshotted() { return this->snapshot_taken_; }

void DartBoard::calibrate_board(cv::Mat& input_frame, int dist, int p1, int p2, int min_R, int max_R)
{
	cv::Mat frame_gray;
	this->frame_circles_ = input_frame.clone();
	cv::cvtColor(input_frame, frame_gray, cv::COLOR_BGR2GRAY);
	cv::GaussianBlur(frame_gray, frame_gray, cv::Size(9, 9), 2, 2);
	cv::HoughCircles(frame_gray, this->potential_circles_, cv::HOUGH_GRADIENT, 2, dist, p1, p2, min_R, max_R);
	cv::Point dartboard_center = cv::Point(this->frame_.cols / 2, this->frame_.rows / 2);
	double smallest_dist = 99999;
	cv::Vec3f inner_board;
	for (size_t i = 0; i < this->potential_circles_.size(); i++)
	{
		cv::Point center(cvRound(this->potential_circles_[i][0]), cvRound(this->potential_circles_[i][1]));
		//int radius = cvRound(this->potential_circles_[i][2]);
		double distance_to_center = distance_between(dartboard_center, center);
		if (distance_to_center < smallest_dist)
		{
			smallest_dist = distance_to_center;
			inner_board = this->potential_circles_[i];
		}
		
	}

	circle(this->frame_circles_, Point(cvRound(inner_board[0]), cvRound(inner_board[1])), 3, cv::Scalar(0, 255, 0), -1, 8, 0);
	circle(this->frame_circles_, Point(cvRound(inner_board[0]), cvRound(inner_board[1])), inner_board[2], cv::Scalar(0, 0, 255), 3, 8, 0);
}

bool DartBoard::take_snapshot(cv::Mat& input_frame)
{
	if (this->potential_circles_.size() == 1)
	{
		this->dartboard_circle_ = this->potential_circles_[0];
		cv::Point center(cvRound(dartboard_circle_[0]), cvRound(dartboard_circle_[1]));
		int radius = cvRound(dartboard_circle_[2]);
		cv::Mat roi(input_frame, cv::Rect(center.x - radius, center.y - radius, radius * 2, radius * 2));
		cv::Mat mask(roi.size(), roi.type(), cv::Scalar::all(0));
		circle(mask, cv::Point(radius, radius), radius, cv::Scalar::all(255), -1);
		this->frame_ = roi & mask;
		this->snapshot_taken_ = true;
	}
	else
	{
		std::cout << "Error: Too many circles!\n";
	}
	return true;
	
}

cv::Mat& DartBoard::get_frame() { return this->frame_; }

cv::Mat& DartBoard::get_frame_circles() { return this->frame_circles_; }

cv::Mat& DartBoard::get_frame_bullseye_circles() { return this->frame_bullseye_circles_; }

bool DartBoard::locate_bullseye(int dist, int p1, int p2, int min_R, int max_R, bool inner)
{
	const int ERROR_OFF_CENTER = 10;
	this->frame_bullseye_circles_ = this->frame_.clone();
	cv::Mat frame_dartboard_gray;
	cv::Vec3f bullseye_circle;
	std::vector<cv::Vec3f> potential_bullseyes;
	cv::cvtColor(this->frame_, frame_dartboard_gray, cv::COLOR_BGR2GRAY);
	cv::Point dartboard_center = cv::Point(this->frame_.cols / 2, this->frame_.rows / 2);
	double smallest_dist = 99999;
	cv::GaussianBlur(frame_dartboard_gray, frame_dartboard_gray, cv::Size(5, 5), 2, 2);
	cv::HoughCircles(frame_dartboard_gray, potential_bullseyes, cv::HOUGH_GRADIENT, 1, dist, p1, p2, min_R, max_R);
	for (size_t i = 0; i < potential_bullseyes.size(); i++)
	{
		cv::Point center(cvRound(potential_bullseyes[i][0]), cvRound(potential_bullseyes[i][1]));
		int radius = cvRound(potential_bullseyes[i][2]);
		double distance_to_center = distance_between(dartboard_center, center);
		if (distance_to_center < smallest_dist)
		{
			smallest_dist = distance_to_center;
			bullseye_circle = potential_bullseyes[i];
		}
	}

	circle(this->frame_bullseye_circles_, cv::Point(bullseye_circle[0], bullseye_circle[1]), 3, cv::Scalar(0, 255, 0), -1, 8, 0);
	circle(this->frame_bullseye_circles_, cv::Point(bullseye_circle[0], bullseye_circle[1]), bullseye_circle[2], cv::Scalar(0, 0, 255), 1, 8, 0);
	string type;
	if (inner)
	{
		type = "inner";
		this->inner_bullseye_ = bullseye_circle;
	}
	else
	{
		type = "outer";
		this->outer_bullseye_ = bullseye_circle;
	}

	if (smallest_dist < ERROR_OFF_CENTER)
	{
		
		if (inner)
		{
			this->inner_bullseye_calibrated_ = true;
		}
		else
		{
			this->outer_bullseye_calibrated_ = true;
		}
		std::cout << type << "-bullseye looks good " << smallest_dist << std::endl;
		return true;
	} else
	{
		std::cout << "bullseye looks bad " << smallest_dist << std::endl;
		return false;
	}
	
}

void DartBoard::new_snapshot() { this->snapshot_taken_ = false; this->inner_bullseye_calibrated_ = false; this->outer_bullseye_calibrated_ = false; this->doubles_calibrated_ = false, segment_lines_calibrated_ = false; }

bool DartBoard::bullseye_located(bool inner)
{
	if (inner)
		return this->inner_bullseye_calibrated_;
	return this->outer_bullseye_calibrated_;
}

bool DartBoard::calibrated()
{
	if (this->outer_bullseye_calibrated_ && this->inner_bullseye_calibrated_ && this->doubles_calibrated_ && this->triples_calibrated_ && this->segment_lines_calibrated_)
		return true;
}

cv::Mat DartBoard::get_frame_segments()
{
	Mat frame_segments = this->frame_.clone();
	cv::circle(frame_segments, Point(this->outer_bullseye_[0], this->outer_bullseye_[1]), this->outer_bullseye_[2], cv::Scalar(0, 0, 255), 2, 8, 0);
	cv::circle(frame_segments, Point(this->inner_bullseye_[0], this->inner_bullseye_[1]), this->inner_bullseye_[2], cv::Scalar(255, 0, 0), 2, 8, 0);
	cv::circle(frame_segments, Point(this->inner_triple_[0], this->inner_triple_[1]), this->inner_triple_[2], cv::Scalar(255, 0, 0), 2, 8, 0);
	cv::circle(frame_segments, Point(this->outer_triple_[0], this->outer_triple_[1]), this->outer_triple_[2], cv::Scalar(0, 255, 0), 2, 8, 0);
	cv::circle(frame_segments, Point(this->inner_double_[0], this->inner_double_[1]), this->inner_double_[2], cv::Scalar(0, 255, 0), 2, 8, 0);
	cv::circle(frame_segments, Point(this->outer_double_[0], this->outer_double_[1]), this->outer_double_[2], cv::Scalar(0, 255, 0), 2, 8, 0);


	cv::Scalar colors[3] = { Scalar(255, 0, 0), Scalar(0, 255, 0), Scalar(0, 0, 255) };
	if (this->calibrated())
	{
		
		// segments
		for (int i = 0; i < this->segments_.size(); i++)
		{
			line(frame_segments, Point(this->segments_[i].lines[0][0], this->segments_[i].lines[0][1]), Point(this->segments_[i].lines[0][2], this->segments_[i].lines[0][3]), colors[i % 3], 3, 8);
			line(frame_segments, Point(this->segments_[i].lines[1][0], this->segments_[i].lines[1][1]), Point(this->segments_[i].lines[1][2], this->segments_[i].lines[1][3]), colors[i % 3], 3, 8);
			Point priamry = find_further_point(Point(this->segments_[i].lines[0][0], this->segments_[i].lines[0][1]), Point(this->segments_[i].lines[0][2], this->segments_[i].lines[0][3]), Point(frame_segments.cols / 2, frame_segments.rows / 2));
			
			putText(frame_segments, to_string(this->segments_[i].ID), Point(priamry.x - 10, priamry.y - 5), FONT_HERSHEY_COMPLEX, 0.9, colors[i % 3], 1);
		}

	}
	else
	{
		// lines
		for (int i = 0; i < this->lines_.size(); i++)
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

bool DartBoard::doubles_located()
{
	return this->doubles_calibrated_;
}

bool DartBoard::locate_doubles(int p1, int p2)
{
	const int ERROR_OFF_CENTER = 10;
	cv::Mat gray, edge, frame_contours = Mat::zeros(Size(this->frame_.cols, this->frame_.rows), CV_8UC1);
	cvtColor(this->frame_, gray, COLOR_RGB2GRAY);
	Canny(gray, edge, p1, p2);
	vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

	cv::Scalar colors[3];
	colors[0] = cv::Scalar(255, 0, 0);
	colors[1] = cv::Scalar(0, 255, 0);
	colors[2] = cv::Scalar(0, 0, 255);
	cvtColor(frame_contours, frame_contours, COLOR_GRAY2BGR);
	findContours(edge, contours, hierarchy, RETR_LIST, CHAIN_APPROX_NONE);
	// delete small/errorous 
	for (size_t i = 0; i < contours.size(); i++) {
		if (contours[i].size() < 50)
		{
			contours.erase(contours.begin() + i);
		}
		//cout << i << ": " << contours[i].size() << endl;
	}

	for (size_t i = 0; i < contours.size(); i++) {
		cv::drawContours(frame_contours, contours, i, colors[1]);
	}

	int numb_circles = 0, runs = 0;
	const int MAX_RUNS = 10;

	while (numb_circles != 2 && runs < MAX_RUNS)
	{
		runs++;
		std::vector<cv::Vec3f> circles;
		Mat frame_gray;
		cv::cvtColor(this->frame_, frame_gray, cv::COLOR_BGR2GRAY);
		cv::GaussianBlur(frame_gray, frame_gray, cv::Size(9, 9), 2, 2);// int track_dist5 = 200, p15 = 101, p25 = 80, minR5 = 153, maxR5 = 163;
		if (numb_circles == 0)
			cv::HoughCircles(frame_gray, this->potential_circles_, cv::HOUGH_GRADIENT, 1.7, 200, 101, 80, 153, 163);
		else 
			cv::HoughCircles(frame_gray, this->potential_circles_, cv::HOUGH_GRADIENT, 1.7, 200, 103, 89, 160, 192);

		cv::Point dartboard_center = cv::Point(this->frame_.cols / 2, this->frame_.rows / 2);
		double smallest_dist = 99999;
		cv::Vec3f outer_circ;
		for (size_t i = 0; i < this->potential_circles_.size(); i++)
		{
			cv::Point center(cvRound(this->potential_circles_[i][0]), cvRound(this->potential_circles_[i][1]));
			int radius = cvRound(this->potential_circles_[i][2]);
			double distance_to_center = distance_between(dartboard_center, center);
			if (distance_to_center < smallest_dist)
			{
				smallest_dist = distance_to_center;
				outer_circ = this->potential_circles_[i];
			}
			//circle(frame_contours, cv::Point(this->potential_circles_[i][0], this->potential_circles_[i][1]), 3, cv::Scalar(0, 255, 0), -1, 8, 0);
			//circle(frame_contours, cv::Point(this->potential_circles_[i][0], this->potential_circles_[i][1]), this->potential_circles_[i][2], cv::Scalar(0, 0, 255), 1, 8, 0);
		}

		circle(frame_contours, cv::Point(outer_circ[0], outer_circ[1]), 3, cv::Scalar(0, 255, 0), -1, 8, 0);
		circle(frame_contours, cv::Point(outer_circ[0], outer_circ[1]), outer_circ[2], cv::Scalar(0, 0, 255), 1, 8, 0);

		 
		if (smallest_dist < ERROR_OFF_CENTER)
		{
			if (numb_circles == 0)
				this->inner_double_ = outer_circ;
			else
				this->outer_double_ = outer_circ;

			numb_circles++;
			std::cout << "double looks good " << smallest_dist << std::endl;
		}
	}
	this->doubles_calibrated_ = true;
	return true;
}


bool DartBoard::locate_triples(int p1, int p2)
{
	const int ERROR_OFF_CENTER = 10;
	cv::Mat gray, edge, frame_contours = Mat::zeros(Size(this->frame_.cols, this->frame_.rows), CV_8UC1);
	cvtColor(this->frame_, gray, COLOR_RGB2GRAY);
	Canny(gray, edge, p1, p2);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	cv::Scalar colors[3];
	colors[0] = cv::Scalar(255, 0, 0);
	colors[1] = cv::Scalar(0, 255, 0);
	colors[2] = cv::Scalar(0, 0, 255);
	cvtColor(frame_contours, frame_contours, COLOR_GRAY2BGR);
	findContours(edge, contours, hierarchy, RETR_LIST, CHAIN_APPROX_NONE);
	// delete small/errorous 
	for (size_t i = 0; i < contours.size(); i++) {
		if (contours[i].size() < 50)
		{
			contours.erase(contours.begin() + i);
		}

	}

	for (size_t i = 0; i < contours.size(); i++) {
		cv::drawContours(frame_contours, contours, i, colors[1]);
	}

	int numb_circles = 0;

	while (numb_circles != 2)
	{
		std::vector<cv::Vec3f> circles;
		Mat frame_gray;
		cv::cvtColor(this->frame_, frame_gray, cv::COLOR_BGR2GRAY);
		cv::GaussianBlur(frame_gray, frame_gray, cv::Size(9, 9), 2, 2);// int track_dist5 = 200, p15 = 101, p25 = 80, minR5 = 153, maxR5 = 163;
		if (numb_circles == 0)
			cv::HoughCircles(frame_gray, this->potential_circles_, cv::HOUGH_GRADIENT, 1.5, 170, 52, 70, 99, 139);
		else
			cv::HoughCircles(frame_gray, this->potential_circles_, cv::HOUGH_GRADIENT, 1.5, 118, 43, 77, 71, 106);

		cv::Point dartboard_center = cv::Point(this->frame_.cols / 2, this->frame_.rows / 2);
		double smallest_dist = 99999;
		cv::Vec3f outer_circ;
		for (size_t i = 0; i < this->potential_circles_.size(); i++)
		{
			cv::Point center(cvRound(this->potential_circles_[i][0]), cvRound(this->potential_circles_[i][1]));
			int radius = cvRound(this->potential_circles_[i][2]);
			double distance_to_center = distance_between(dartboard_center, center);
			if (distance_to_center < smallest_dist)
			{
				smallest_dist = distance_to_center;
				outer_circ = this->potential_circles_[i];
			}
			//circle(frame_contours, cv::Point(this->potential_circles_[i][0], this->potential_circles_[i][1]), 3, cv::Scalar(0, 255, 0), -1, 8, 0);
			//circle(frame_contours, cv::Point(this->potential_circles_[i][0], this->potential_circles_[i][1]), this->potential_circles_[i][2], cv::Scalar(0, 0, 255), 1, 8, 0);
		}

		circle(frame_contours, cv::Point(outer_circ[0], outer_circ[1]), 3, cv::Scalar(0, 255, 0), -1, 8, 0);
		circle(frame_contours, cv::Point(outer_circ[0], outer_circ[1]), outer_circ[2], cv::Scalar(0, 0, 255), 1, 8, 0);


		if (smallest_dist < ERROR_OFF_CENTER)
		{
			if (numb_circles == 0)  
				this->outer_triple_ = outer_circ;
			else
				this->inner_triple_ = outer_circ;

			numb_circles++;
			std::cout << "triple looks good " << smallest_dist << std::endl;
		}
	}
	this->triples_calibrated_ = true;
	//return frame_contours;
	return true;
}

bool DartBoard::triples_located() { return this->triples_calibrated_; }

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
	Point cent = Point(frame_edges.cols / 2, frame_edges.rows / 2);
	// draw black circle middle to remove bullseye
	circle(frame_edges, cent, 20, Scalar(0, 0, 0), FILLED, 8, 0);
	HoughLinesP(frame_edges, this->lines_, 1, CV_PI / 180, p1, p2, p3);

	// sort lines
	for (int e = 0; e < this->lines_.size() - 1; e++)
	{
		for (int j = 0; j < this->lines_.size() - e - 1; j++)
		{
			Point dir_1 = find_further_point(Point(this->lines_[j][0], this->lines_[j][1]), Point(this->lines_[j][2], this->lines_[j][3]), // comparing j & j+1's non-origin angles, bub sort
				Point(frame_edges.cols / 2, frame_edges.rows / 2));
			Point dir_2 = find_further_point(Point(this->lines_[j+1][0], this->lines_[j+1][1]), Point(this->lines_[j+1][2], this->lines_[j+1][3]),
				Point(frame_edges.cols / 2, frame_edges.rows / 2));
			if (atan2(dir_1.x - frame_edges.cols / 2, dir_1.y - frame_edges.rows / 2) > atan2(dir_2.x - frame_edges.cols / 2, dir_2.y - frame_edges.rows / 2))
				std::swap(this->lines_[j], this->lines_[j+1]); 
		}
	}
	

	// remove unneeded lines
	
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
					// take larger one

					if (distance_between(og_dir, cent) < distance_between(secondary_dir, cent))
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

void DartBoard::check_hit(cv::Point& hit)
{
	// Check for multiplier/special region
	cout << boolalpha;
	bool hit_inside = false;
	if (segment_hit(hit, this->outer_double_, this->inner_double_))
	{
		cout << "Hit double: " << hit << "!\n";
		hit_inside = true;
	}
	else if (segment_hit(hit, this->outer_triple_, this->inner_triple_))
	{
		cout << "Hit triple: " << hit << "!\n";
		hit_inside = true;
	}
	else if (segment_hit(hit, this->inner_double_, this->outer_triple_))
	{
		cout << "Hit outer-single:: " << hit << "!\n";
		hit_inside = true;
	}
	else if (segment_hit(hit, this->inner_triple_, this->outer_bullseye_))
	{
		cout << "Hit inner-single: " << hit << "!\n";
		hit_inside = true;
	}
	else if (segment_hit(hit, this->outer_bullseye_, this->inner_bullseye_))
		cout << "Hit Green Bullseye: " << hit << "!\n";
	else if (segment_hit(hit, this->inner_bullseye_))
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

bool DartBoard::segment_hit(cv::Point& hit, cv::Vec3f outer)
{
	if (sqrt(pow(hit.x - outer[0], 2) + pow(hit.y - outer[1], 2)) < outer[2])
		return true;
	return false;
}

bool DartBoard::segment_lines_calibrated()
{
	return this->segment_lines_calibrated_;
}

void DartBoard::create_segment(int x, int g)
{
	int ccw_ids[20] = { 20, 5, 12, 9, 14, 11, 8, 16, 7, 19, 3, 17, 2, 15, 10, 6, 13, 4, 18, 1 };
	Segment seg;
	seg.ID = ccw_ids[this->segments_.size()];
	seg.lines[0] = this->lines_[x];
	if (g == this->lines_.size())
		seg.lines[1] = this->lines_[0];
	else
		seg.lines[1] = this->lines_[g];

	this->segments_.push_back(seg);
}

void DartBoard::lock_in_segment_lines()
{
	if (this->lines_.size() == 20)
	{
		this->segment_lines_calibrated_ = true;

		for (int i = 0; i < this->lines_.size(); i++)
		{
			this->create_segment(i, i + 1);
		}
	}
}

cv::Mat DartBoard::check_darts(cv::Mat input_frame)
{
	cv::Point center(cvRound(dartboard_circle_[0]), cvRound(dartboard_circle_[1]));
	int radius = cvRound(dartboard_circle_[2]);
	cv::Mat roi(input_frame, cv::Rect(center.x - radius, center.y - radius, radius * 2, radius * 2));
	cv::Mat mask(roi.size(), roi.type(), cv::Scalar::all(0));
	circle(mask, cv::Point(radius, radius), radius, cv::Scalar::all(255), -1);
	return roi & mask;

}


