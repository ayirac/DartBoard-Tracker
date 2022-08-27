#pragma once
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>

struct Segment
{
	int ID;
	cv::Vec4i lines[2];
};

class DartBoard
{
private:
	std::vector<cv::Vec3f> potential_circles_;
	cv::Vec3f dartboard_circle_;
	cv::Mat frame_, frame_circles_, frame_bullseye_circles_;
	std::vector<Segment> segments_;
	cv::Vec3f outer_bullseye_, inner_bullseye_;
	//std::vector<std::vector<cv::Point>> bullseyes_;
	bool snapshot_taken_, outer_bullseye_calibrated_, inner_bullseye_calibrated_;
	bool doubles_calibrated_, triples_calibrated_;
	bool segment_lines_calibrated_;
	cv::Vec3f outer_triple_, inner_triple_, outer_double_, inner_double_;
	std::vector<cv::Vec4i> lines_;

public:
	DartBoard();
	bool snapshotted();
	void calibrate_board(cv::Mat& input_frame, int dist, int p1, int p2, int min_R, int max_R);
	bool take_snapshot(cv::Mat& input_frame);
	cv::Mat& get_frame();
	cv::Mat& get_frame_circles();
	cv::Mat& get_frame_bullseye_circles();
	bool locate_bullseye(int dist, int p1, int p2, int min_R, int max_R, bool inner);
	void new_snapshot();
	bool bullseye_located(bool inner);
	bool calibrated();
	cv::Mat get_frame_segments();
	bool doubles_located();
	bool locate_triples(int p1, int p2);
	bool locate_doubles(int p1, int p2);
	bool triples_located();
	cv::Mat locate_singles(int p1, int p2, int p3, int e_p1, int e_p2);
	void check_hit(cv::Point& hit);
	bool segment_hit(cv::Point& hit, cv::Vec3f outer, cv::Vec3f inner);
	bool segment_hit(cv::Point& hit, cv::Vec3f outer);
	bool segment_lines_calibrated();
	void create_segment(int x, int g);
	void lock_in_segment_lines();
	cv::Mat check_darts(cv::Mat input_frame);
};
