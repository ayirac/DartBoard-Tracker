#pragma once
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>


typedef enum {BULLSEYE_INNER, BULLEYES_OUTER, TRIPLE_INNER, TRIPLE_OUTER, DOUBLE_INNER, DOUBLE_OUTER } TYPE;

struct Segment
{
	int ID;
	cv::Vec4i lines[2];
};

struct BoundaryCircle
{
	TYPE type;
	cv::Vec3f circ;
};

struct CircleParams
{
	int dist, p1, p2, minR, maxR;
	CircleParams() : dist(1), p1(1), p2(1), minR(1), maxR(1) {};
	CircleParams(int d, int p1, int p2, int min, int max) : dist(d), p1(p1), p2(p2), minR(min), maxR(max) {};
};

struct LinesParams
{
	int p1, p2, p3, e_p1, e_p2;
};

struct ThreshParams
{
	int lowH, lowS, lowV, highH, highS, highV, thresh;
};

class DartBoard
{
private:
	cv::Vec3f dartboard_circle_;
	cv::Mat frame_, cam_frame_, temp_frame_;
	std::vector<Segment> segments_;
	BoundaryCircle* boundaries_[6];
	int c_state_; // Calibration State: Snapshot, Bullseye_O, Bullseye_I, Triples_O, Triples_I, Doubles_O, Doubles_I, Segment LInes (0 - 7)
	std::vector<cv::Vec3f> potential_circles_;
	std::vector<cv::Vec4i> lines_;
	bool new_state_;
	cv::Point2f src_pnts[4], dst_pnts[4];
public:
	DartBoard();
	cv::Mat calibrate_board(int dist, int p1, int p2, int min_R, int max_R);
	cv::Mat locate_four_corners(cv::Mat& input_frame, cv::Scalar lows, cv::Scalar highs, int thresh, int warpX, int warpY);
	void perspective_transform(cv::Mat& input_frame, cv::Scalar lows, cv::Scalar highs, int thresh);
	bool take_snapshot();
	void take_perspective_transform(cv::Mat& input_frame, int warpX, int warpY);
	cv::Mat& get_frame();
	cv::Mat& get_frame_bullseye_circles();
	cv::Mat locate_boundary(int dist, int p1, int p2, int min_R, int max_R, BoundaryCircle& boundary, int offcenter_threshold);
	cv::Mat locate_boundaries(CircleParams* arr);
	cv::Mat get_frame_segments();
	cv::Mat locate_singles(int p1, int p2, int p3, int e_p1, int e_p2);
	void check_hit(cv::Point& hit);
	bool segment_hit(cv::Point& hit, cv::Vec3f outer, cv::Vec3f inner);
	bool segment_hit(cv::Point& hit, cv::Vec3f outer);
	void create_segment(int x, int g);
	void lock_in_segment_lines();
	cv::Mat check_darts(cv::Mat input_frame);
	int& get_state();
	void set_state(int s);
	bool state_change();
	cv::Mat& get_cam_frame() { return cam_frame_; }
};
